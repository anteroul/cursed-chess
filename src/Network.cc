#include "Network.hh"

#include <algorithm>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/udp.hpp>
#include <chrono>
#include <nexilis/client/client_api.hh>
#include <nexilis/client/packet.hh>
#include <nexilis/client/protocol/nxboost/tcp_client.hh>
#include <nexilis/protocol_manager.hh>
#include <nexilis/server/protocol/nxboost/tcp_server.hh>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>
#include "SquareManager.hh"

namespace
{
	namespace nx = nexilis;
	using nx::client::Packet;
	using Clock = std::chrono::steady_clock;
	using namespace std::chrono_literals;

	constexpr std::string_view RoomName = "nx-chess";
	// first word of every message, so unrelated room traffic is ignored
	constexpr std::string_view ProtocolTag = "nx-chess/1";

	// the host retries while its own server starts; a guest gives up sooner
	constexpr int HostConnectAttempts = 20;
	constexpr int GuestConnectAttempts = 3;
	constexpr auto AuthenticationTimeout = 2s;
	constexpr auto RetryDelay = 250ms;
	constexpr auto RoomListInterval = 1s;

	nx::server::AuthenticationMode authenticationMode(const NetworkOptions &options)
	{
		return options.password.empty() ? nx::server::AuthenticationMode::skip
										: nx::server::AuthenticationMode::password_protected;
	}

	nx::client::ClientConfig makeClientConfig(const NetworkOptions &options)
	{
		nx::client::ClientConfig config;
		config.setBoostTCPAddress(options.role == NetworkOptions::Role::Host ? "127.0.0.1" : options.address);
		config.setMode(authenticationMode(options));
		// TLS-PSK derives its key from the password, so an open game is unencrypted
		config.setTls(!options.password.empty());
		if (!options.password.empty())
			config.setPassword(options.password);
		return config;
	}

	// IPv4 addresses other computers can most likely reach this one at
	std::vector<std::string> findLocalAddresses()
	{
		namespace ip = boost::asio::ip;
		boost::asio::io_context io;
		boost::system::error_code ec;
		std::vector<std::string> addresses;

		const auto add = [&](const ip::address &address) {
			const std::string text = address.to_string();
			if (address.is_v4() && !address.is_loopback() && !address.is_unspecified()
				&& std::ranges::find(addresses, text) == addresses.end())
				addresses.push_back(text);
		};

		// the interface that routes to the internet; connecting a UDP socket sends no packets
		ip::udp::socket probe(io);
		probe.connect({ip::make_address_v4("8.8.8.8"), 53}, ec);
		if (!ec)
		{
			const auto local = probe.local_endpoint(ec);
			if (!ec)
				add(local.address());
		}

		// plus whatever the host name resolves to, which covers networks without a default route
		ip::udp::resolver resolver(io);
		const auto results = resolver.resolve(ip::udp::v4(), ip::host_name(ec), "", ec);
		if (!ec)
			for (const auto &entry : results)
				add(entry.endpoint().address());

		return addresses;
	}

	template <typename Predicate>
	bool waitFor(Predicate &&predicate, Clock::duration timeout)
	{
		const auto deadline = Clock::now() + timeout;
		while (!predicate() && Clock::now() < deadline)
			std::this_thread::sleep_for(50ms);
		return predicate();
	}

	char promotionLetter(PieceType type) noexcept
	{
		switch (type)
		{
			case PieceType::Queen:
				return 'q';
			case PieceType::Rook:
				return 'r';
			case PieceType::Knight:
				return 'n';
			case PieceType::Bishop:
				return 'b';
			default:
				return '-';
		}
	}

	std::optional<PieceType> promotionFromLetter(char letter) noexcept
	{
		switch (letter)
		{
			case 'q':
				return PieceType::Queen;
			case 'r':
				return PieceType::Rook;
			case 'n':
				return PieceType::Knight;
			case 'b':
				return PieceType::Bishop;
			case '-':
				return PieceType::None;
			default:
				return std::nullopt;
		}
	}

	// "nx-chess/1 move <fromX> <fromY> <toX> <toY> <promotion>" or "nx-chess/1 reset"
	std::string encode(const NetworkMove &move)
	{
		std::ostringstream out;
		out << ProtocolTag << " move " << move.fromX << ' ' << move.fromY << ' ' << move.toX << ' ' << move.toY << ' '
			<< promotionLetter(move.promotion);
		return out.str();
	}

	std::optional<NetworkEvent> decode(const std::string &payload)
	{
		std::istringstream in(payload);
		std::string tag;
		std::string command;
		if (!(in >> tag >> command) || tag != ProtocolTag)
			return std::nullopt;

		if (command == "reset")
			return NetworkEvent{NetworkEvent::Type::Reset};

		NetworkMove move;
		char promotion = 0;
		if (command != "move" || !(in >> move.fromX >> move.fromY >> move.toX >> move.toY >> promotion))
			return std::nullopt;

		if (!Sqr::onBoard(move.fromX, move.fromY) || !Sqr::onBoard(move.toX, move.toY))
			return std::nullopt;

		const auto type = promotionFromLetter(promotion);
		if (!type)
			return std::nullopt;

		move.promotion = *type;
		return NetworkEvent{NetworkEvent::Type::Move, move};
	}
}

struct NetworkSession::Impl
{
	enum class Stage : std::uint8_t { FindingRoom, Joining, WaitingForOpponent, Playing, OpponentLeft };

	explicit Impl(const NetworkOptions &opts);
	~Impl();

	Impl(const Impl &) = delete;
	Impl &operator=(const Impl &) = delete;

	void connect();
	void update();
	void send(const nx::nx_data &message);
	[[nodiscard]] std::size_t playersInRoom() const;
	[[nodiscard]] bool isHost() const noexcept { return options.role == NetworkOptions::Role::Host; }

	NetworkOptions options;
	nx::ProtocolManager protocols;
	// declared before the client so it is destroyed after it
	std::unique_ptr<nx::server::nxboost::TCPServer> server;
	// the client keeps a pointer to the API, so it must not move
	nx::client::ClientAPI api;
	std::unique_ptr<nx::client::nxboost::TCPClient> client;

	Stage stage = Stage::FindingRoom;
	std::uint64_t roomId = 0;
	std::size_t readMessages = 0;
	Clock::time_point lastRoomRequest{};
	std::vector<std::string> status;
	std::vector<std::string> addresses;
};

NetworkSession::Impl::Impl(const NetworkOptions &opts) : options(opts), api(makeClientConfig(opts))
{
	if (isHost())
	{
		nx::server::ServerConfig settings;
		settings.setMode(authenticationMode(options));
		settings.setTls(!options.password.empty());
		if (!options.password.empty())
			settings.setPassphrase(options.password);

		server = std::make_unique<nx::server::nxboost::TCPServer>(
			protocols.createProtocol<nx::server::nxboost::TCPServer>(settings));
		server->start();
	}

	connect();

	send(Packet::Set::General::username(api, isHost() ? "host" : "guest"));
	if (isHost())
	{
		send(Packet::Room::Management::create(api, std::string(RoomName), nx::RoomData::Context::_2D));

		addresses = findLocalAddresses();
		if (addresses.empty())
			status.emplace_back("Hosting, but no network address was found.");
		for (const std::string &address : addresses)
			status.push_back("Hosting on " + address);
		status.emplace_back("Waiting for an opponent...");
	} else
		status.push_back("Connected to " + options.address + ", looking for the game...");
}

NetworkSession::Impl::~Impl()
{
	if (client)
		client->stop();
	if (server)
		server->stop();
}

void NetworkSession::Impl::connect()
{
	// the server may still be starting up, so retry for a few seconds
	const int attempts = isHost() ? HostConnectAttempts : GuestConnectAttempts;
	for (int attempt = 0; attempt < attempts; attempt++)
	{
		client = std::make_unique<nx::client::nxboost::TCPClient>(
			protocols.createProtocol<nx::client::nxboost::TCPClient>(api));
		client->start();

		if (waitFor([this] { return api.isBoostTCPReady(); }, AuthenticationTimeout))
			return;

		client->stop();
		client.reset();
		std::this_thread::sleep_for(RetryDelay);
	}

	throw std::runtime_error("Could not connect to " + api.getBoostTCPServerAddress()
							 + " (is the host running, and is the password right?)");
}

void NetworkSession::Impl::send(const nx::nx_data &message)
{
	client->sendMessage(message);
}

std::size_t NetworkSession::Impl::playersInRoom() const
{
	for (auto &room : api.getActiveRooms())
		if (room.getId() == roomId)
			return room.getClients().size();
	return 0;
}

void NetworkSession::Impl::update()
{
	switch (stage)
	{
		case Stage::FindingRoom:
		{
			const auto rooms = api.getActiveRooms();
			const auto room = std::ranges::find_if(rooms, [](const auto &r) { return r.getName() == RoomName; });
			if (room != rooms.end())
			{
				roomId = room->getId();
				send(Packet::Room::Management::join(api, roomId));
				stage = Stage::Joining;
			} else if (!isHost() && Clock::now() - lastRoomRequest > RoomListInterval)
			{
				send(Packet::Get::Info::rooms(api));
				lastRoomRequest = Clock::now();
			}
			break;
		}

		case Stage::Joining:
			if (api.clientInRoom())
				stage = Stage::WaitingForOpponent;
			break;

		case Stage::WaitingForOpponent:
			if (playersInRoom() >= 2)
			{
				stage = Stage::Playing;
				status.emplace_back(isHost() ? "Opponent joined, you play white." : "Joined the game, you play black.");
			}
			break;

		case Stage::Playing:
			if (playersInRoom() < 2)
			{
				stage = Stage::OpponentLeft;
				status.emplace_back("Opponent left the game.");
			}
			break;

		case Stage::OpponentLeft:
			break;
	}
}

NetworkSession::NetworkSession(const NetworkOptions &options) : impl(std::make_unique<Impl>(options)) {}

NetworkSession::~NetworkSession() = default;

void NetworkSession::update()
{
	impl->update();
}

bool NetworkSession::ready() const noexcept
{
	return impl->stage == Impl::Stage::Playing;
}

User NetworkSession::localSide() const noexcept
{
	return impl->isHost() ? User::Player : User::Engine;
}

std::vector<std::string> NetworkSession::takeStatus()
{
	return std::exchange(impl->status, {});
}

const std::vector<std::string> &NetworkSession::localAddresses() const noexcept
{
	return impl->addresses;
}

void NetworkSession::sendMove(const NetworkMove &move)
{
	impl->send(Packet::Room::Communicate::broadcast(impl->api, encode(move)));
}

void NetworkSession::sendReset()
{
	impl->send(Packet::Room::Communicate::broadcast(impl->api, std::string(ProtocolTag) + " reset"));
}

std::vector<NetworkEvent> NetworkSession::receive()
{
	std::vector<NetworkEvent> events;
	if (impl->roomId == 0)
		return events;

	// broadcasts reach the sender too, so skip our own messages
	const auto messages = impl->api.getChatSnapshot(impl->roomId);
	const std::uint64_t self = impl->api.getClientId();

	for (; impl->readMessages < messages.size(); impl->readMessages++)
	{
		const auto &message = messages[impl->readMessages];
		if (message.sender_id == self)
			continue;

		if (auto event = decode(message.message))
			events.push_back(*event);
	}

	return events;
}
