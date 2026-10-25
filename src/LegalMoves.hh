#pragma once

#include <vector>
#include "Piece.hh"
#include "Square.hh"

namespace LegalMove
{
	// moves allowed by the piece's movement rules, ignoring whether they leave its own king in check
	[[nodiscard]] std::vector<Square> pseudoLegal(const Piece &piece);

	// pseudo-legal moves that don't leave the piece's own king in check
	[[nodiscard]] std::vector<Square> legal(const Piece &piece);
}
