#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "Piece.hh"

struct NetworkOptions
{
	enum class Role : std::uint8_t { Host, Join };

	Role role = Role::Host;
	// server to join; the host always connects to its own server on localhost
	std::string address = "127.0.0.1";
	// shared secret for authentication and TLS; empty means an open, unencrypted game
	std::string password;
};

struct NetworkMove
{
	int fromX = 0;
	int fromY = 0;
	int toX = 0;
	int toY = 0;
	// piece a pawn promotes to, or None
	PieceType promotion = PieceType::None;
};

struct NetworkEvent
{
	enum class Type : std::uint8_t { Move, Reset };

	Type type = Type::Move;
	NetworkMove move{};
};

// a two-player game over Nexilis: the host runs the server and plays the bottom side (User::Player),
// the joining client plays the top side (User::Engine)
class NetworkSession
{
public:
	// connects to the server, starting it first when hosting; throws std::runtime_error on failure
	explicit NetworkSession(const NetworkOptions &options);
	~NetworkSession();

	NetworkSession(const NetworkSession &) = delete;
	NetworkSession &operator=(const NetworkSession &) = delete;

	// advance the room handshake and watch the opponent; call once per frame
	void update();

	// both players are in the room
	[[nodiscard]] bool ready() const noexcept;

	[[nodiscard]] User localSide() const noexcept;

	// human-readable status lines added since the last call
	[[nodiscard]] std::vector<std::string> takeStatus();

	// this computer's IPv4 addresses for the opponent to join; empty when none was found
	[[nodiscard]] const std::vector<std::string> &localAddresses() const noexcept;

	void sendMove(const NetworkMove &move);
	void sendReset();

	// well-formed events sent by the opponent since the last call; moves still need validating
	[[nodiscard]] std::vector<NetworkEvent> receive();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
