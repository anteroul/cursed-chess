#pragma once

#include <SDL.h>
#include <optional>
#include <vector>
#include "Piece.hh"
#include "Sdl.hh"
#include "Square.hh"
#include "Text.hh"

namespace GUI
{
	[[nodiscard]] bool isMouseOver(const SDL_Point &mousePos, const SDL_Rect &rect) noexcept;

	// the board square under the mouse, or nullptr outside the board
	[[nodiscard]] Square *squareAt(const SDL_Point &mousePos);
}

// pawn promotion choice: a button with the piece's logo for each piece, its name below
class PromotionTable
{
public:
	// throws std::runtime_error if a texture or the font can't be loaded
	PromotionTable();

	// draw over the board, highlighting the button under the mouse
	void render(const SDL_Point &mousePos) const;

	// the piece whose button is under the mouse, if any
	[[nodiscard]] std::optional<PieceType> pieceAt(const SDL_Point &mousePos) const;

private:
	explicit PromotionTable(const sdl::Font &font);

	struct Button
	{
		PieceType type;
		sdl::Texture logo;
		Text label;
	};

	struct Layout
	{
		SDL_Rect panel;
		std::vector<SDL_Rect> buttons;
	};

	// computed from the current window size, so the table follows resizes
	[[nodiscard]] Layout layout() const;

	std::vector<Button> buttons;
	Text title;
};
