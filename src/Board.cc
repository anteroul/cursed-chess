#include "Board.hh"

#include "Renderer.hh"
#include "ScreenSize.hh"
#include "SquareManager.hh"

namespace
{
	bool flipped = false;

	// width and height of a single tile; leave space on the right for the console
	float tileWidth() noexcept
	{
		return static_cast<float>(Screen::getWidth() / BoardSize) * 0.8f;
	}

	int tileHeight() noexcept
	{
		return Screen::getHeight() / BoardSize;
	}
}

void Board::setFlipped(bool flip) noexcept
{
	flipped = flip;
}

bool Board::isFlipped() noexcept
{
	return flipped;
}

SDL_Rect Board::bounds() noexcept
{
	return {0, 0, static_cast<int>(static_cast<float>(BoardSize) * tileWidth()), BoardSize * tileHeight()};
}

void Board::render()
{
	const float width = tileWidth();
	const int height = tileHeight();

	for (int x = 0; x < BoardSize; x++)
	{
		for (int y = 0; y < BoardSize; y++)
		{
			// where the square appears on screen
			const int column = flipped ? BoardSize - 1 - x : x;
			const int row = flipped ? BoardSize - 1 - y : y;

			Square &square = Sqr::at(x, y);
			square.rect = {static_cast<int>(static_cast<float>(column) * width), row * height, static_cast<int>(width),
						   height};

			// different colors to every other square
			if ((x + y) % 2 != 0)
				Renderer::setColor(64, 48, 0);
			else
				Renderer::setColor(128, 64, 0);

			Renderer::fillRect(square.rect);
		}
	}
}
