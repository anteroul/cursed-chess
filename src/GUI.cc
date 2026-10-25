#include "GUI.hh"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include "Assets.hh"
#include "Board.hh"
#include "FontLoader.hh"
#include "Renderer.hh"
#include "SquareManager.hh"
#include "Texture.hh"

namespace
{
	struct Choice
	{
		PieceType type;
		std::string_view name;
		std::string_view logo;
	};

	// in the order of the 1-4 keyboard shortcuts
	constexpr std::array<Choice, 4> choices{{
		{PieceType::Queen, "Queen", "Buttons/queen_btn.png"},
		{PieceType::Rook, "Rook", "Buttons/rook_btn.png"},
		{PieceType::Knight, "Knight", "Buttons/knight_btn.png"},
		{PieceType::Bishop, "Bishop", "Buttons/bishop_btn.png"},
	}};

	constexpr SDL_Color LabelColor{230, 230, 230, SDL_ALPHA_OPAQUE};
	constexpr int LabelFontSize = 16;
}

bool GUI::isMouseOver(const SDL_Point &mousePos, const SDL_Rect &rect) noexcept
{
	return SDL_PointInRect(&mousePos, &rect);
}

Square *GUI::squareAt(const SDL_Point &mousePos)
{
	// hit-test the laid out squares, which also covers a flipped board
	for (int x = 0; x < BoardSize; x++)
		for (int y = 0; y < BoardSize; y++)
			if (Square &square = Sqr::at(x, y); isMouseOver(mousePos, square.rect))
				return &square;

	return nullptr;
}

// the labels are rendered once, so the font is only needed while constructing
PromotionTable::PromotionTable() : PromotionTable(FontLoader::load(Assets::path("Fonts/mytype.ttf"), LabelFontSize)) {}

PromotionTable::PromotionTable(const sdl::Font &font) : title(font.get(), "Promote pawn to:", LabelColor, {})
{
	buttons.reserve(choices.size());
	for (const Choice &choice : choices)
		buttons.push_back({choice.type, Texture::load(Assets::path(choice.logo)),
						   Text(font.get(), std::string(choice.name), LabelColor, {})});
}

PromotionTable::Layout PromotionTable::layout() const
{
	const SDL_Rect board = Board::bounds();
	const int count = static_cast<int>(buttons.size());
	const int gap = std::max(board.w / 40, 4);
	const int labelHeight = buttons.front().label.position.h;
	const int titleHeight = title.position.h;

	// square buttons as large as fit across the board, but not taller than a third of it
	const int size = std::min((board.w * 9 / 10 - gap * (count + 1)) / count, board.h / 3);

	Layout result;
	const int panelWidth = count * size + (count + 1) * gap;
	const int panelHeight = gap + titleHeight + gap + size + gap / 2 + labelHeight + gap;
	result.panel = {board.x + (board.w - panelWidth) / 2, board.y + (board.h - panelHeight) / 2, panelWidth,
					panelHeight};

	for (int i = 0; i < count; i++)
		result.buttons.push_back(
			{result.panel.x + gap + i * (size + gap), result.panel.y + gap + titleHeight + gap, size, size});

	return result;
}

void PromotionTable::render(const SDL_Point &mousePos) const
{
	const Layout l = layout();

	// dim the board so the choice stands out
	Renderer::setColor(0, 0, 0, 140);
	Renderer::fillRect(Board::bounds());

	Renderer::setColor(40, 40, 40, 235);
	Renderer::fillRect(l.panel);
	Renderer::setColor(200, 200, 200);
	Renderer::drawRect(l.panel);

	title.renderAt(l.panel.x + (l.panel.w - title.position.w) / 2,
				   l.panel.y + (l.buttons.front().y - l.panel.y - title.position.h) / 2);

	for (std::size_t i = 0; i < buttons.size(); i++)
	{
		const Button &button = buttons[i];
		const SDL_Rect &rect = l.buttons[i];
		const bool hovered = GUI::isMouseOver(mousePos, rect);

		// highlight: warmer background and a thick gold border under the mouse
		if (hovered)
			Renderer::setColor(120, 95, 30);
		else
			Renderer::setColor(75, 75, 75);
		Renderer::fillRect(rect);

		if (hovered)
		{
			Renderer::setColor(255, 215, 0);
			Renderer::drawRect(rect, 3);
		} else
		{
			Renderer::setColor(140, 140, 140);
			Renderer::drawRect(rect);
		}

		const int inset = rect.w / 8;
		Renderer::drawTexture(button.logo.get(),
							  {rect.x + inset, rect.y + inset, rect.w - 2 * inset, rect.h - 2 * inset});

		const SDL_Rect &label = button.label.position;
		button.label.renderAt(rect.x + (rect.w - label.w) / 2,
							  rect.y + rect.h + (l.panel.y + l.panel.h - rect.y - rect.h - label.h) / 2);
	}
}

std::optional<PieceType> PromotionTable::pieceAt(const SDL_Point &mousePos) const
{
	const Layout l = layout();
	for (std::size_t i = 0; i < buttons.size(); i++)
		if (GUI::isMouseOver(mousePos, l.buttons[i]))
			return buttons[i].type;

	return std::nullopt;
}
