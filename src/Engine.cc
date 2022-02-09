#include "Engine.hh"

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::PlayMove()
{
	getAllSquares();

	// get random num
	int piece = pickPiece();
	int timer = 0;

	// this is bad
	Square s;

	// get moves for random num
	std::vector<Square> v = LegalMove::getLegal(Pieces::get(piece));

	// get piece that has legal moves
	for (;;)
	{
		if (timer > 30)
		{
			currentGameState = VICTORY;
			break;
		}
		if (!v.empty())
			break;
		else
		{
			piece = pickPiece();
			v = LegalMove::getLegal(Pieces::get(piece));
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			timer++;
		}
	}

	for (auto &i: v)
	{
		s = Sqr::getSquare(i.x, i.y);
		if (Sqr::getSquare(i.x, i.y).piece.type != NONE && Sqr::getSquare(i.x, i.y).piece.type != KING && Sqr::getSquare(i.x, i.y).piece.user == PLAYER)
			break;
	}
	Move::execute(Pieces::get(piece), s);
}

int Engine::pickPiece()
{
	// TODO: Evaluate the board. Pick pieces based on player's and AI's legal moves.
	return rand() % 16;
}

// get all squares
void Engine::getAllSquares()
{
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 8; j++)
			squares[i][j] = Sqr::squareHelper(i, j);
}

void Engine::makeFakeMove(Piece source, Square target)
{
	// source goes to target
	squares[target.x][target.y]->piece = source;

	// source is set to zero
	squares[source.x][source.y]->piece = ghost(source.x, source.y);
}



