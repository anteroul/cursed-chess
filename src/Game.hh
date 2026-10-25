#pragma once

#include <SDL.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "GUI.hh"
#include "Network.hh"
#include "Piece.hh"
#include "PieceFactory.hh"
#include "Sdl.hh"
#include "Square.hh"
#include "Text.hh"
#include "Window.hh"

class Game
{
public:
	// local game on one computer, or a networked game when options are given;
	// throws std::runtime_error if the window, assets or connection can't be created
	explicit Game(const std::optional<NetworkOptions> &networkOptions = std::nullopt);
	void updateGame();
	[[nodiscard]] bool shouldClose() const noexcept { return applicationShouldClose; }

private:
	void eventHandler();
	void onMouseClick();
	void onPromotionKey(SDL_Keycode key);
	void update();
	void render();
	void playTurn();
	void executeMove(Square &from, const Square &target);
	void promote(Square &square, PieceType type);
	void requestReset();
	void resetGame();
	void updateConsole();
	void updateConsoleText(const std::string &text);
	void updateConsoleText(const std::string &text, SDL_Color color);

	void handleNetwork();
	void applyRemoteMove(const NetworkMove &move);
	// may the person at this computer move now
	[[nodiscard]] bool canMoveLocally() const noexcept;
	// the side whose point of view game results are reported from
	[[nodiscard]] User viewer() const noexcept;

	// connected before the window opens, so a failed connection doesn't flash a window
	std::unique_ptr<NetworkSession> network;
	// our promotion move, sent once the new piece is chosen
	std::optional<NetworkMove> pendingPromotion;

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
