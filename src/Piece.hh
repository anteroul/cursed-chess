#pragma once

#include <cstdint>

enum class PieceType : std::uint8_t { Pawn, Rook, Knight, Bishop, Queen, King, None };
enum class Color : std::uint8_t { Black, White, Undefined };
enum class User : std::uint8_t { Player, Engine, Ghost };

struct Piece
{
	PieceType type = PieceType::None;
	Color color = Color::Undefined;
	int x = 0;
	int y = 0;
	User user = User::Ghost;

	[[nodiscard]] constexpr bool isEmpty() const noexcept { return type == PieceType::None; }
};

// placeholder for an empty square at (x, y)
[[nodiscard]] constexpr Piece ghost(int x, int y) noexcept
{
	return {PieceType::None, Color::Undefined, x, y, User::Ghost};
}

[[nodiscard]] constexpr User opponentOf(User user) noexcept
{
	switch (user)
	{
		case User::Player:
			return User::Engine;
		case User::Engine:
			return User::Player;
		default:
			return User::Ghost;
	}
}
