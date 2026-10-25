#include "Game.hh"

#include <algorithm>
#include <cassert>
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

	[[nodiscard]] bool hasLegalMove(User side)
	{
		return std::ranges::any_of(Pieces::all(), [side](const Piece &p) {
			return p.user == side && !p.isEmpty() && !LegalMove::legal(p).empty();
		});
	}
}

Game::Game() : consoleFont{FontLoader::load(Assets::path("Fonts/mytype.ttf"), 14)}
{
	resetGame();
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
				if (!Global.inPromotion)
					onMouseClick();
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
					applicationShouldClose = true;
				// if game is over press 'R' to reset and play again :)
				else if (e.key.keysym.sym == SDLK_r)
					resetGame();
				else if (Global.inPromotion)
					onPromotionKey(e.key.keysym.sym);
				break;

			default:
				break;
		}
	}
}

void Game::onMouseClick()
{
	selectedSquare = &GUI::squareAt(mousePos);

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

	// the promoting pawn stands on the selected (target) square
	assert(selectedSquare);
	if (Piece *pawn = Pieces::getReal(selectedSquare->piece))
		pawn->type = promoted;
	selectedSquare->piece.type = promoted;

	Global.inPromotion = false;
	Global.playerTurn = !Global.playerTurn;
}

// update turns
void Game::update()
{
	if (Global.state == GameState::On)
		playTurn();
}

void Game::render()
{
	// make gray background
	Renderer::setColor(64, 64, 64);
	Renderer::clear();

	for (Text &line : console)
		line.render();

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

	if (Global.inPromotion)
		promotionTable.render();

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
	if (!isPieceSelected || selectedSquare == originalSquare)
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
	executeMove(target);
}

void Game::executeMove(const Square &target)
{
	const User side = currentSide();
	const auto all = Pieces::all();
	const auto piece = std::ranges::find_if(
		all, [&](const Piece &p) { return p.user == side && !p.isEmpty() && &Sqr::at(p.x, p.y) == originalSquare; });

	if (piece == all.end())
		return;

	// pawn promotion
	if (piece->type == PieceType::Pawn && (target.y == 0 || target.y == BoardSize - 1))
		Global.inPromotion = true;

	lastMove = Move::execute(*piece, target);
	legalMoves.clear();
	updateConsole();

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
			updateConsoleText("Checkmate! You won!");
			Global.state = GameState::End;
			break;
		case GameState::Defeat:
			updateConsoleText("Checkmate! You lost.");
			Global.state = GameState::End;
			break;
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
	constexpr int lineHeight = 18;

	console.emplace_back(consoleFont.get(), text, Global.playerTurn ? playerColor : engineColor,
						 SDL_Point{Screen::getWidth() - 150, Screen::getHeight() - 25});

	// scroll older lines up
	for (Text &line : console)
		line.position.y -= lineHeight;
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

	// clear console
	console.clear();
}
