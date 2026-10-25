#include "Move.hh"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include "Global.hh"
#include "Pieces.hh"
#include "SquareManager.hh"

namespace
{
	// make the square empty
	void emptySquare(int x, int y)
	{
		Sqr::at(x, y).piece = ghost(x, y);
	}

	// make the piece empty
	void emptyPiece(int x, int y)
	{
		Pieces::set(ghost(x, y), x, y);
	}

	constexpr std::string_view pieceName(PieceType type) noexcept
	{
		switch (type)
		{
			case PieceType::Pawn:
				return "Pawn";
			case PieceType::Rook:
				return "Rook";
			case PieceType::Knight:
				return "Knight";
			case PieceType::Bishop:
				return "Bishop";
			case PieceType::Queen:
				return "Queen";
			case PieceType::King:
				return "King";
			case PieceType::None:
				break;
		}
		return "";
	}

	// e.g. (4, 6) -> "E2"
	std::string squareName(int x, int y)
	{
		return {static_cast<char>('A' + x), static_cast<char>('8' - y)};
	}

	// move king and rook to their castled squares; returns the castling notation
	std::string_view castle(Piece &king, Piece &rook, bool queenSide)
	{
		emptySquare(king.x, king.y);
		emptySquare(rook.x, rook.y);

		if (queenSide)
		{
			king.x -= 2;
			rook.x += 3;
		} else
		{
			king.x += 2;
			rook.x -= 2;
		}

		Sqr::at(rook.x, rook.y).piece = rook;
		Sqr::at(king.x, king.y).piece = king;

		return queenSide ? "0-0-0" : "0-0";
	}
}

std::string Move::execute(Piece &source, Square target)
{
	std::string prefix{pieceName(source.type)};
	const bool player = source.user == User::Player;
	SideState &side = Global.side(source.user);

	// castling
	if (source.type == PieceType::King)
	{
		side.kingMoved = true;

		if (target.x == source.x - 2)
			prefix =
				castle(source, Pieces::get(player ? Pieces::PlayerQueenSideRook : Pieces::EngineQueenSideRook), true);
		else if (target.x == source.x + 2)
			prefix =
				castle(source, Pieces::get(player ? Pieces::PlayerKingSideRook : Pieces::EngineKingSideRook), false);
	}

	// if rook is moved, has to do with castling
	if (source.type == PieceType::Rook)
	{
		if (source.x == 0)
			side.queenSideRookMoved = true;
		if (source.x == 7)
			side.kingSideRookMoved = true;
	}

	if (source.type == PieceType::Pawn)
	{
		// en passant capture: the captured pawn is behind the target square
		if (const Square *ep = Global.enPassant; ep && target.x == ep->x && target.y == ep->y)
		{
			const int capturedY = target.y + (player ? 1 : -1);
			emptyPiece(target.x, capturedY);
			emptySquare(target.x, capturedY);
		}

		// a double step makes the skipped square available for en passant
		Global.enPassant =
			std::abs(target.y - source.y) == 2 ? Sqr::find(source.x, (source.y + target.y) / 2) : nullptr;
	} else
		Global.enPassant = nullptr;

	// regular move, capturing whatever is on the target square
	if (!target.piece.isEmpty())
		emptyPiece(target.x, target.y);

	emptySquare(source.x, source.y);
	source.x = target.x;
	source.y = target.y;
	Sqr::at(source.x, source.y).piece = source;

	std::string name = prefix + " to " + squareName(target.x, target.y);
	std::cout << name << '\n';
	return name;
}
