#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include "Game.hh"
#include "Network.hh"

namespace
{
	constexpr Uint32 TargetFps = 60;
	constexpr Uint32 FrameDelayMs = 1000 / TargetFps;

#ifndef NX_CHESS_VERSION
#define NX_CHESS_VERSION "unknown"
#endif
	constexpr std::string_view Version = NX_CHESS_VERSION;

	constexpr std::string_view Usage = R"(usage: nx-chess [--host | --join ADDRESS] [--password PASSWORD]

Without options both players share this computer.

  --host               host a network game and play white
  --join ADDRESS       join the game hosted at ADDRESS and play black
  --password PASSWORD  require PASSWORD from both players and encrypt the
                       connection (TLS-PSK); NX_CHESS_PASSWORD also works
  -h, --help           show this help
  --version            show the version

The host listens on TCP ports 54200 and 54202.
)";

	struct CommandLine
	{
		std::optional<NetworkOptions> network;
		bool help = false;
		bool version = false;
	};

	// throws std::invalid_argument on bad usage
	CommandLine parseCommandLine(std::span<char *const> args)
	{
		CommandLine result;
		NetworkOptions options;
		std::optional<NetworkOptions::Role> role;

		for (std::size_t i = 1; i < args.size(); i++)
		{
			const std::string_view arg = args[i];
			const auto value = [&]() -> std::string {
				if (i + 1 >= args.size())
					throw std::invalid_argument(std::string(arg) + " needs a value");
				return args[++i];
			};

			if (arg == "-h" || arg == "--help")
				result.help = true;
			else if (arg == "--version")
				result.version = true;
			else if (arg == "--host")
				role = NetworkOptions::Role::Host;
			else if (arg == "--join")
			{
				role = NetworkOptions::Role::Join;
				options.address = value();
			} else if (arg == "--password")
				options.password = value();
			else
				throw std::invalid_argument("unknown option " + std::string(arg));
		}

		// prefer the environment, a password on the command line is visible to other users
		if (const char *password = std::getenv("NX_CHESS_PASSWORD"); password && options.password.empty())
			options.password = password;

		if (role)
		{
			options.role = *role;
			result.network = options;
		}
		return result;
	}

	// initializes SDL and its extension libraries for the lifetime of the object
	class SdlContext
	{
	public:
		SdlContext()
		{
			if (SDL_Init(SDL_INIT_VIDEO) != 0)
				throw std::runtime_error(std::string("SDL could not initialize: ") + SDL_GetError());

			// initialize PNG loading
			if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
				throw std::runtime_error(std::string("SDL_image could not initialize: ") + IMG_GetError());

			if (TTF_Init() != 0)
				throw std::runtime_error(std::string("SDL_ttf could not initialize: ") + TTF_GetError());
		}

		~SdlContext()
		{
			TTF_Quit();
			IMG_Quit();
			SDL_Quit();
		}

		SdlContext(const SdlContext &) = delete;
		SdlContext &operator=(const SdlContext &) = delete;
	};
}

int main(int argc, char *argv[])
{
	CommandLine commandLine;
	try
	{
		commandLine = parseCommandLine(std::span(argv, static_cast<std::size_t>(argc)));
	}
	catch (const std::invalid_argument &e)
	{
		std::cerr << "nx-chess: " << e.what() << "\n\n" << Usage;
		return 2;
	}

	if (commandLine.version)
	{
		std::cout << "nx-chess " << Version << '\n';
		return EXIT_SUCCESS;
	}

	if (commandLine.help)
	{
		std::cout << Usage;
		return EXIT_SUCCESS;
	}

	if (commandLine.network && commandLine.network->password.empty())
		std::cerr << "Warning: no password set, the game is unauthenticated and unencrypted.\n";

	try
	{
		const SdlContext sdl;
		Game game(commandLine.network);

		// Game loop:
		while (!game.shouldClose())
		{
			const Uint32 frameStart = SDL_GetTicks();

			game.updateGame();

			const Uint32 frameTime = SDL_GetTicks() - frameStart;
			if (frameTime < FrameDelayMs)
				SDL_Delay(FrameDelayMs - frameTime);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
