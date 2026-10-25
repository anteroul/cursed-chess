#pragma once

#include <array>
#include <cstddef>
#include "Piece.hh"
#include "Sdl.hh"

// loads and owns the piece textures
class PieceFactory
{
public:
	// throws std::runtime_error if a texture can't be loaded
	PieceFactory();

	// texture for the piece, or nullptr for an empty square
	[[nodiscard]] SDL_Texture *texture(PieceType type, Color color) const noexcept;

	// draw the piece on its square
	void render(const Piece &piece) const;

private:
	static constexpr std::size_t TypeCount = 6;
	static constexpr std::size_t ColorCount = 2;

	[[nodiscard]] static constexpr std::size_t index(PieceType type, Color color) noexcept
	{
		return static_cast<std::size_t>(color) * TypeCount + static_cast<std::size_t>(type);
	}

	std::array<sdl::Texture, TypeCount * ColorCount> textures;
};
