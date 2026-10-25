#include "PieceFactory.hh"

#include <string>
#include "Assets.hh"
#include "Renderer.hh"
#include "SquareManager.hh"
#include "Texture.hh"

namespace
{
	// file name letter for each PieceType, e.g. "wh.png" is the white knight
	constexpr std::array<char, 6> typeLetters{'p', 'r', 'h', 'b', 'q', 'k'};
}

PieceFactory::PieceFactory()
{
	static_assert(typeLetters.size() == TypeCount);

	for (const Color color : {Color::Black, Color::White})
	{
		const char colorLetter = color == Color::Black ? 'b' : 'w';

		for (std::size_t type = 0; type < TypeCount; type++)
		{
			const std::string file{colorLetter, typeLetters[type]};
			textures[index(static_cast<PieceType>(type), color)] = Texture::load(Assets::path(file + ".png"));
		}
	}
}

SDL_Texture *PieceFactory::texture(PieceType type, Color color) const noexcept
{
	if (type == PieceType::None || color == Color::Undefined)
		return nullptr;

	return textures[index(type, color)].get();
}

void PieceFactory::render(const Piece &piece) const
{
	if (SDL_Texture *t = texture(piece.type, piece.color))
		Renderer::drawTexture(t, Sqr::at(piece.x, piece.y).rect);
}
