#include "ScreenSize.hh"

namespace
{
	// the screen sizes
	int width = 0;
	int height = 0;
}

void Screen::setWidth(int w) noexcept
{
	width = w;
}
void Screen::setHeight(int h) noexcept
{
	height = h;
}

int Screen::getWidth() noexcept
{
	return width;
}
int Screen::getHeight() noexcept
{
	return height;
}
