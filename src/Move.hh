#pragma once

#include <string>
#include "Piece.hh"
#include "Square.hh"

namespace Move
{
	// play `source` to `target`, handling captures, castling and en passant;
	// returns a human-readable description of the move.
	// `target` is a copy on purpose: the move rewrites the board squares while it runs
	std::string execute(Piece &source, Square target);
}
