#include "Pieces.hh"

#include <array>
#include <cassert>
#include "Settings.hh"
#include "SquareManager.hh"

namespace
{
	std::array<Piece, Pieces::Count> pieces{};

	struct Placement
	{
		PieceType type;
		int x;
	};

	// order matters: it defines the indices of the non-pawn pieces
	constexpr std::array<Placement, BoardSize> backRank{{
		{PieceType::Rook, 0},
		{PieceType::Rook, 7},
		{PieceType::Knight, 1},
		{PieceType::Knight, 6},
		{PieceType::Bishop, 2},
		{PieceType::Bishop, 5},
		{PieceType::Queen, 3},
		{PieceType::King, 4},
	}};
}

Piece &Pieces::get(int n)
{
	assert(n >= 0 && n < Count);
	return pieces[n];
}

std::span<Piece, Pieces::Count> Pieces::all() noexcept
{
	return pieces;
}

void Pieces::set(const Piece &piece, int x, int y) noexcept
{
	for (Piece &p : pieces)
	{
		if (p.x == x && p.y == y)
		{
			p.color = piece.color;
			p.type = piece.type;
			p.user = piece.user;
		}
	}
}

Piece *Pieces::getReal(const Piece &piece) noexcept
{
	for (Piece &p : pieces)
		if (p.x == piece.x && p.y == piece.y && p.user == piece.user && p.type == piece.type)
			return &p;

	return nullptr;
}

void Pieces::init()
{
	constexpr int pawnCount = BoardSize;
	constexpr int playerOffset = Count / 2;

	for (int i = 0; i < pawnCount; i++)
	{
		pieces[i] = {PieceType::Pawn, Settings::engineColor, i, 1, User::Engine};
		pieces[playerOffset + i] = {PieceType::Pawn, Settings::playerColor, i, 6, User::Player};

		const auto [type, x] = backRank[i];
		pieces[pawnCount + i] = {type, Settings::engineColor, x, 0, User::Engine};
		pieces[playerOffset + pawnCount + i] = {type, Settings::playerColor, x, 7, User::Player};
	}

	// clear the board, then place every piece on its square
	for (int x = 0; x < BoardSize; x++)
		for (int y = 0; y < BoardSize; y++)
			Sqr::at(x, y).piece = ghost(x, y);

	for (const Piece &p : pieces)
		Sqr::at(p.x, p.y).piece = p;
}
