#include "LegalMoves.hh"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <utility>
#include "Global.hh"
#include "Pieces.hh"
#include "SquareManager.hh"

namespace
{
	using Moves = std::vector<Square>;
	using Direction = std::pair<int, int>;

	constexpr std::array<Direction, 8> knightSteps{
		{{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}}};

	constexpr std::array<Direction, 8> allDirections{
		{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};

	constexpr std::array<Direction, 4> diagonals{{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
	constexpr std::array<Direction, 4> orthogonals{{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

	// KNIGHT, KING: a single step to an empty or enemy square
	void addStep(Moves &moves, const Piece &p, Direction d)
	{
		const Square *s = Sqr::find(p.x + d.first, p.y + d.second);
		if (s && (s->piece.isEmpty() || s->piece.color != p.color))
			moves.push_back(*s);
	}

	// ROOK, BISHOP, QUEEN: slide until blocked, including a capture of the blocking enemy
	void addSlide(Moves &moves, const Piece &p, Direction d)
	{
		for (int i = 1; i < BoardSize; i++)
		{
			const Square *s = Sqr::find(p.x + i * d.first, p.y + i * d.second);
			if (!s)
				break;

			if (s->piece.isEmpty())
			{
				moves.push_back(*s);
				continue;
			}

			if (s->piece.color != p.color)
				moves.push_back(*s);
			break;
		}
	}

	// the path to the rook must be empty
	void addCastling(Moves &moves, const Piece &king, bool kingSide)
	{
		const int dir = kingSide ? 1 : -1;
		const int rookDistance = kingSide ? 3 : 4;

		for (int i = 1; i <= rookDistance; i++)
		{
			const Square *s = Sqr::find(king.x + i * dir, king.y);
			if (!s)
				break;

			if (s->piece.isEmpty())
				continue;

			if (s->piece.type == PieceType::Rook && i == rookDistance)
				moves.push_back(Sqr::at(king.x + 2 * dir, king.y));
			break;
		}
	}

	void addPawnMoves(Moves &moves, const Piece &p)
	{
		const bool player = p.user == User::Player;
		const int forward = player ? -1 : 1;
		const int startRank = player ? 6 : 1;

		// double step from the starting rank
		if (p.y == startRank && Sqr::at(p.x, p.y + forward).piece.isEmpty()
			&& Sqr::at(p.x, p.y + 2 * forward).piece.isEmpty())
			moves.push_back(Sqr::at(p.x, p.y + 2 * forward));

		const auto addCapture = [&](int dx) {
			const Square *s = Sqr::find(p.x + dx, p.y + forward);
			if (s && !s->piece.isEmpty() && s->piece.color != p.color)
				moves.push_back(*s);
		};

		addCapture(forward);

		if (const Square *s = Sqr::find(p.x, p.y + forward); s && s->piece.isEmpty())
			moves.push_back(*s);

		addCapture(-forward);

		if (const Square *ep = Global.enPassant; ep && ep->y == p.y + forward && std::abs(ep->x - p.x) == 1)
			moves.push_back(*ep);
	}

	// play the move on the board temporarily and see if any opponent piece can then take the king
	bool leavesKingInCheck(const Piece &piece, const Square &target)
	{
		Square &from = Sqr::at(piece.x, piece.y);
		Square &to = Sqr::at(target.x, target.y);
		const Piece captured = to.piece;

		to.piece = piece;
		from.piece = ghost(piece.x, piece.y);

		const User opponent = opponentOf(piece.user);
		const bool inCheck = std::ranges::any_of(Pieces::all(), [&](const Piece &attacker) {
			// skip pieces of the other side and the piece captured by this move
			if (attacker.user != opponent || Sqr::at(attacker.x, attacker.y).piece.user != opponent)
				return false;

			return std::ranges::any_of(LegalMove::pseudoLegal(attacker),
									   [](const Square &s) { return s.piece.type == PieceType::King; });
		});

		to.piece = captured;
		from.piece = piece;
		return inCheck;
	}
}

std::vector<Square> LegalMove::pseudoLegal(const Piece &piece)
{
	Moves moves;

	switch (piece.type)
	{
		case PieceType::Pawn:
			addPawnMoves(moves, piece);
			break;

		case PieceType::Knight:
			for (const Direction d : knightSteps)
				addStep(moves, piece, d);
			break;

		case PieceType::Queen:
			for (const Direction d : allDirections)
				addSlide(moves, piece, d);
			break;

		case PieceType::King:
		{
			for (const Direction d : allDirections)
				addStep(moves, piece, d);

			const SideState &side = Global.side(piece.user);
			if (side.canCastleKingSide)
				addCastling(moves, piece, true);
			if (side.canCastleQueenSide)
				addCastling(moves, piece, false);
			break;
		}

		case PieceType::Bishop:
			for (const Direction d : diagonals)
				addSlide(moves, piece, d);
			break;

		case PieceType::Rook:
			for (const Direction d : orthogonals)
				addSlide(moves, piece, d);
			break;

		case PieceType::None:
			break;
	}

	return moves;
}

std::vector<Square> LegalMove::legal(const Piece &piece)
{
	Moves moves = pseudoLegal(piece);
	std::erase_if(moves, [&](const Square &target) { return leavesKingInCheck(piece, target); });
	return moves;
}
