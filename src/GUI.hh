#pragma once

#include <SDL.h>
#include "Sdl.hh"
#include "Square.hh"

namespace GUI
{
	[[nodiscard]] bool isMouseOver(const SDL_Point &mousePos, const SDL_Rect &rect) noexcept;

	// the board square under the mouse; clicks right of the board select the last file
	[[nodiscard]] Square &squareAt(const SDL_Point &mousePos);
}

// displays during pawn promotion
class PromotionTable
{
public:
	// throws std::runtime_error if the texture can't be loaded
	PromotionTable();
	void render() const;

private:
	sdl::Texture tooltips;
	// TODO: Promotion GUI buttons (Assets/Buttons)
};
