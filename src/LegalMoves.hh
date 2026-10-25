#pragma once

#include <vector>
#include "Piece.hh"
#include "Square.hh"

namespace LegalMove
{
	// `piece` is a copy on purpose: move generation temporarily rewrites the board squares,
	// so a reference to a square's piece would be clobbered

	// moves allowed by the piece's movement rules, ignoring whether they leave its own king in check
	[[nodiscard]] std::vector<Square> pseudoLegal(Piece piece);

	// pseudo-legal moves that don't leave the piece's own king in check
	[[nodiscard]] std::vector<Square> legal(Piece piece);
}
