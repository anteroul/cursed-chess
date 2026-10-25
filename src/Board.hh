#pragma once

#include <SDL.h>

namespace Board
{
	// show the board from the top player's side, for the one playing black
	void setFlipped(bool flipped) noexcept;
	[[nodiscard]] bool isFlipped() noexcept;

	// the part of the window the board covers
	[[nodiscard]] SDL_Rect bounds() noexcept;

	// lay out the squares for the current window size and draw them
	void render();
}
