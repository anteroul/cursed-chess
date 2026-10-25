#include "Game.hh"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include "Assets.hh"
#include "Board.hh"
#include "FontLoader.hh"
#include "GameManager.hh"
#include "Global.hh"
#include "LegalMoves.hh"
#include "Move.hh"
#include "Pieces.hh"
#include "Renderer.hh"
#include "ScreenSize.hh"
#include "Settings.hh"
#include "SquareManager.hh"

namespace
{
	[[nodiscard]] User currentSide() noexcept
	{
		return Global.playerTurn ? User::Player : User::Engine;
	}

	constexpr SDL_Color StatusColor{200, 200, 200, SDL_ALPHA_OPAQUE};

	[[nodiscard]] bool hasLegalMove(User side)
	{
		return std::ranges::any_of(Pieces::all(), [side](const Piece &p) {
			return p.user == side && !p.isEmpty() && !LegalMove::legal(p).empty();
		});
	}
}

Game::Game(const std::optional<NetworkOptions> &networkOptions)
	: network{networkOptions ? std::make_unique<NetworkSession>(*networkOptions) : nullptr},
	  consoleFont{FontLoader::load(Assets::path("Fonts/mytype.ttf"), 14)}
{
	resetGame();

	// whoever plays black sees the board from their own side
	Board::setFlipped(network && network->localSide() == User::Engine);

	if (network)
	{
		for (const std::string &address : network->localAddresses())
			std::cout << "The opponent can join with: nx-chess --join " << address
					  << (networkOptions->password.empty() ? "" : " --password <password>") << '\n';
	}
}

void Game::updateGame()
{
	eventHandler();
	update();
	render();
}

void Game::eventHandler()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		// resize window
		window.handleEvent(e);

		switch (e.type)
		{
			case SDL_QUIT:
				applicationShouldClose = true;
				break;

			case SDL_MOUSEMOTION:
				mousePos = {e.motion.x, e.motion.y};
				break;

			case SDL_MOUSEBUTTONDOWN:
				mousePos = {e.button.x, e.button.y};
				if (!canMoveLocally())
					break;

				if (!Global.inPromotion)
					onMouseClick();
				else if (const auto piece = promotionTable.pieceAt(mousePos))
					choosePromotion(*piece);
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
					applicationShouldClose = true;
				// if game is over press 'R' to reset and play again :)
				else if (e.key.keysym.sym == SDLK_r)
					requestReset();
				else if (Global.inPromotion && canMoveLocally())
					onPromotionKey(e.key.keysym.sym);
				break;

			default:
				break;
		}
	}
}

void Game::onMouseClick()
{
	Square *clicked = GUI::squareAt(mousePos);
	if (!clicked)
		return;

	selectedSquare = clicked;

	// show possible moves
	const Piece &piece = selectedSquare->piece;
	if (!piece.isEmpty() && piece.user == currentSide())
	{
		originalSquare = selectedSquare;
		legalMoves = LegalMove::legal(piece);
		isPieceSelected = true;
	}
}

void Game::onPromotionKey(SDL_Keycode key)
{
	PieceType promoted{};
	switch (key)
	{
		case SDLK_1:
			promoted = PieceType::Queen;
			break;
		case SDLK_2:
			promoted = PieceType::Rook;
			break;
		case SDLK_3:
			promoted = PieceType::Knight;
			break;
		case SDLK_4:
			promoted = PieceType::Bishop;
			break;
		default:
			return;
	}

	choosePromotion(promoted);
}

void Game::choosePromotion(PieceType promoted)
{
	// the promoting pawn stands on the selected (target) square
	assert(selectedSquare);
	promote(*selectedSquare, promoted);

	if (network && pendingPromotion)
	{
		pendingPromotion->promotion = promoted;
		network->sendMove(*pendingPromotion);
		pendingPromotion.reset();
	}
}

void Game::promote(Square &square, PieceType type)
{
	if (Piece *pawn = Pieces::getReal(square.piece))
		pawn->type = type;
	square.piece.type = type;

	Global.inPromotion = false;
	Global.playerTurn = !Global.playerTurn;
}

void Game::requestReset()
{
	if (network)
	{
		if (!network->ready())
			return;
		network->sendReset();
	}
	resetGame();
}

// update turns
void Game::update()
{
	if (network)
		handleNetwork();

	if (Global.state == GameState::On)
		playTurn();
}

void Game::handleNetwork()
{
	network->update();

	// show connection status both in the terminal and in the game console
	for (const std::string &line : network->takeStatus())
	{
		std::cout << line << '\n';
		updateConsoleText(line, StatusColor);
	}

	for (const NetworkEvent &event : network->receive())
	{
		if (event.type == NetworkEvent::Type::Reset)
		{
			resetGame();
			updateConsoleText("Opponent restarted the game.", StatusColor);
		} else
			applyRemoteMove(event.move);
	}
}

void Game::applyRemoteMove(const NetworkMove &move)
{
	// never trust the peer: only accept a legal move of one of its pieces on its turn
	const User opponent = opponentOf(network->localSide());
	Square &from = Sqr::at(move.fromX, move.fromY);

	const bool opponentsTurn = Global.state == GameState::On && !Global.inPromotion && currentSide() == opponent;
	if (!opponentsTurn || from.piece.isEmpty() || from.piece.user != opponent)
	{
		std::cerr << "Ignoring an out-of-turn move from the opponent\n";
		return;
	}

	GameManager::update();
	const auto moves = LegalMove::legal(from.piece);
	const auto target =
		std::ranges::find_if(moves, [&](const Square &s) { return s.x == move.toX && s.y == move.toY; });
	if (target == moves.end())
	{
		std::cerr << "Ignoring an illegal move from the opponent\n";
		return;
	}

	// the board is about to change, so drop the local selection
	selectedSquare = nullptr;
	originalSquare = nullptr;
	isPieceSelected = false;

	executeMove(from, *target);

	if (Global.inPromotion)
	{
		const bool validPromotion =
			move.promotion != PieceType::None && move.promotion != PieceType::Pawn && move.promotion != PieceType::King;
		promote(Sqr::at(move.toX, move.toY), validPromotion ? move.promotion : PieceType::Queen);
	}
}

bool Game::canMoveLocally() const noexcept
{
	return !network || (network->ready() && currentSide() == network->localSide());
}

User Game::viewer() const noexcept
{
	return network ? network->localSide() : User::Player;
}

void Game::render()
{
	// make gray background
	Renderer::setColor(64, 64, 64);
	Renderer::clear();

	// the console is the right fifth of the window
	for (Text &line : console)
	{
		line.position.x = Screen::getWidth() - Screen::getWidth() / 5;
		line.render();
	}

	Board::render();

	if (selectedSquare)
	{
		// color selected square
		Renderer::setColor(0, 0, 255);
		Renderer::fillRect(selectedSquare->rect);

		if (isPieceSelected)
		{
			// color legal moves
			Renderer::setColor(255, 0, 0);
			for (const Square &move : legalMoves)
				Renderer::fillRect(move.rect);
		}
	}

	// color the square where the mouse is
	for (int x = 0; x < BoardSize; x++)
	{
		for (int y = 0; y < BoardSize; y++)
		{
			const SDL_Rect &rect = Sqr::at(x, y).rect;
			if (GUI::isMouseOver(mousePos, rect))
			{
				Renderer::setColor(0, 255, 0);
				Renderer::fillRect(rect);
			}
		}
	}

	for (const Piece &piece : Pieces::all())
		pieces.render(piece);

	if (Global.inPromotion && canMoveLocally())
		promotionTable.render(mousePos);

	Renderer::present();
}

void Game::playTurn()
{
	GameManager::update();

	const User side = currentSide();
	if (!hasLegalMove(side))
	{
		if (!Global.side(side).inCheck)
			Global.state = GameState::Draw;
		else
			Global.state = side == User::Player ? GameState::Defeat : GameState::Victory;

		updateConsole();
		return;
	}

	// wait until a new square is selected
	if (!canMoveLocally() || !isPieceSelected || selectedSquare == originalSquare)
		return;

	const auto move = std::ranges::find_if(
		legalMoves, [this](const Square &s) { return s.x == selectedSquare->x && s.y == selectedSquare->y; });

	if (move == legalMoves.end())
	{
		isPieceSelected = false;
		return;
	}

	// copy: executeMove clears legalMoves
	const Square target = *move;
	executeMove(*originalSquare, target);
}

void Game::executeMove(Square &from, const Square &target)
{
	const User side = currentSide();
	const auto all = Pieces::all();
	const auto piece = std::ranges::find_if(
		all, [&](const Piece &p) { return p.user == side && !p.isEmpty() && &Sqr::at(p.x, p.y) == &from; });

	if (piece == all.end())
		return;

	const NetworkMove networkMove{from.x, from.y, target.x, target.y};

	// pawn promotion
	if (piece->type == PieceType::Pawn && (target.y == 0 || target.y == BoardSize - 1))
		Global.inPromotion = true;

	lastMove = Move::execute(*piece, target);
	legalMoves.clear();
	updateConsole();

	// tell the opponent about our own moves; a promotion is sent once the new piece is chosen
	if (network && side == network->localSide())
	{
		if (Global.inPromotion)
			pendingPromotion = networkMove;
		else
			network->sendMove(networkMove);
	}

	if (!Global.inPromotion)
	{
		isPieceSelected = false;
		Global.playerTurn = !Global.playerTurn;
	}
}

// update console output
void Game::updateConsole()
{
	switch (Global.state)
	{
		case GameState::On:
			updateConsoleText(lastMove);
			break;
		case GameState::Victory:
		case GameState::Defeat:
		{
			const User mated = Global.state == GameState::Defeat ? User::Player : User::Engine;
			updateConsoleText(mated == viewer() ? "Checkmate! You lost." : "Checkmate! You won!");
			Global.state = GameState::End;
			break;
		}
		case GameState::Draw:
			updateConsoleText("Draw!");
			Global.state = GameState::End;
			break;
		case GameState::End:
			break;
	}

	if (Global.state == GameState::End)
		updateConsoleText("Hit 'R' to play again.");
}

void Game::updateConsoleText(const std::string &text)
{
	constexpr SDL_Color playerColor{255, 255, 0, SDL_ALPHA_OPAQUE};
	constexpr SDL_Color engineColor{0, 255, 255, SDL_ALPHA_OPAQUE};

	updateConsoleText(text, Global.playerTurn ? playerColor : engineColor);
}

void Game::updateConsoleText(const std::string &text, SDL_Color color)
{
	// the console is the right fifth of the window, see Text::render
	constexpr int margin = 4;
	constexpr int bottom = 25;
	const int wrapWidth = Screen::getWidth() / 5 - margin;

	Text line(consoleFont.get(), text, color, SDL_Point{Screen::getWidth() - 150, 0}, wrapWidth);
	const int height = line.position.h;
	line.position.y = Screen::getHeight() - bottom - height;

	// scroll older lines up to make room
	for (Text &older : console)
		older.position.y -= height;

	console.push_back(std::move(line));
}

// reset game variables
void Game::resetGame()
{
	// pieces to their correct places
	Pieces::init();

	// gameplay settings to normal; white starts the game
	Global = GlobalState{};
	Global.playerTurn = Settings::playerColor == Color::White;

	// unselect square
	originalSquare = nullptr;
	selectedSquare = nullptr;
	isPieceSelected = false;
	legalMoves.clear();
	pendingPromotion.reset();

	// clear console
	console.clear();
}
