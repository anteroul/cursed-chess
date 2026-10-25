#pragma once

#include <span>
#include "Piece.hh"

namespace Pieces
{
	// index layout:
	// 0-7    ENGINE PAWNS
	// 8-15   ENGINE PIECES
	// 16-23  PLAYER PAWNS
	// 24-31  PLAYER PIECES
	inline constexpr int Count = 32;

	inline constexpr int EngineQueenSideRook = 8;
	inline constexpr int EngineKingSideRook = 9;
	inline constexpr int PlayerQueenSideRook = 24;
	inline constexpr int PlayerKingSideRook = 25;

	// put all pieces in their starting positions
	void init();

	[[nodiscard]] Piece &get(int n);

	[[nodiscard]] std::span<Piece, Count> all() noexcept;

	// the piece in the piece list matching a copy (e.g. one stored in a square), or nullptr
	[[nodiscard]] Piece *getReal(const Piece &piece) noexcept;

	// overwrite type, color and user of every piece standing at (x, y)
	void set(const Piece &piece, int x, int y) noexcept;
}
