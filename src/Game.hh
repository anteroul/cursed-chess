#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include "GUI.hh"
#include "Piece.hh"
#include "PieceFactory.hh"
#include "Sdl.hh"
#include "Square.hh"
#include "Text.hh"
#include "Window.hh"

class Game
{
public:
	// throws std::runtime_error if the window or assets can't be created
	Game();
	void updateGame();
	[[nodiscard]] bool shouldClose() const noexcept { return applicationShouldClose; }

private:
	void eventHandler();
	void onMouseClick();
	void onPromotionKey(SDL_Keycode key);
	void update();
	void render();
	void playTurn();
	void executeMove(const Square &target);
	void resetGame();
	void updateConsole();
	void updateConsoleText(const std::string &text);

	// the window owns the renderer, so it is declared first to outlive every texture below
	Window window;
	PieceFactory pieces;
	PromotionTable promotionTable;
	sdl::Font consoleFont;

	std::vector<Text> console;
	std::vector<Square> legalMoves;
	std::string lastMove;

	SDL_Point mousePos{};
	Square *selectedSquare = nullptr;
	Square *originalSquare = nullptr;
	bool isPieceSelected = false;
	bool applicationShouldClose = false;
};
