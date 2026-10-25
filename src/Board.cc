#include "Board.hh"

#include "Renderer.hh"
#include "ScreenSize.hh"
#include "SquareManager.hh"

void Board::render()
{
	// width and height of single tile; leave space on the right for the console
	const float tileWidth = static_cast<float>(Screen::getWidth() / BoardSize) * 0.8f;
	const int tileHeight = Screen::getHeight() / BoardSize;

	for (int x = 0; x < BoardSize; x++)
	{
		for (int y = 0; y < BoardSize; y++)
		{
			Square &square = Sqr::at(x, y);
			square.rect = {static_cast<int>(static_cast<float>(x) * tileWidth), y * tileHeight,
						   static_cast<int>(tileWidth), tileHeight};

			// different colors to every other square
			if ((x + y) % 2 != 0)
				Renderer::setColor(64, 48, 0);
			else
				Renderer::setColor(128, 64, 0);

			Renderer::fillRect(square.rect);
		}
	}
}
