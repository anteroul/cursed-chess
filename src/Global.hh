#pragma once

#include <cstdint>
#include "Piece.hh"
#include "Square.hh"

enum class GameState : std::uint8_t { On, Victory, Defeat, Draw, End };

// castling and check bookkeeping for one side
struct SideState
{
	bool kingMoved = false;
	bool queenSideRookMoved = false;
	bool kingSideRookMoved = false;

	bool canCastleKingSide = true;
	bool canCastleQueenSide = true;

	bool inCheck = false;
};

struct GlobalState
{
	bool playerTurn = true;
	GameState state = GameState::On;
	Square *enPassant = nullptr;
	bool inPromotion = false;

	SideState player;
	SideState engine;

	[[nodiscard]] SideState &side(User user) noexcept { return user == User::Player ? player : engine; }
	[[nodiscard]] const SideState &side(User user) const noexcept { return user == User::Player ? player : engine; }
};

// state shared by the whole game; reset it by assigning a fresh GlobalState
inline GlobalState Global;
