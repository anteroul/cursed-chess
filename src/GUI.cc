#include "GUI.hh"

#include <algorithm>
#include "Assets.hh"
#include "Renderer.hh"
#include "ScreenSize.hh"
#include "SquareManager.hh"
#include "Texture.hh"

bool GUI::isMouseOver(const SDL_Point &mousePos, const SDL_Rect &rect) noexcept
{
	return SDL_PointInRect(&mousePos, &rect);
}

Square &GUI::squareAt(const SDL_Point &mousePos)
{
	// the board takes 8 of 10 columns of the window
	const int x = mousePos.x * 10 / Screen::getWidth();
	const int y = mousePos.y * BoardSize / Screen::getHeight();

	return Sqr::at(std::clamp(x, 0, BoardSize - 1), std::clamp(y, 0, BoardSize - 1));
}

PromotionTable::PromotionTable() : tooltips{Texture::load(Assets::path("Other/tooltips.png"))} {}

void PromotionTable::render() const
{
	const int w = Screen::getWidth();
	const int h = Screen::getHeight();
	const SDL_Rect canvas{w / 8, h / 5, w / 4 * 3, h / 2};

	Renderer::setColor(128, 128, 128);
	Renderer::fillRect(canvas);
	Renderer::drawTexture(tooltips.get(), canvas);
}
