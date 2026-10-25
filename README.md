# nx-chess — Ukko Chess Engine v1.0

#### Building:
Requires a C++20 compiler, CMake 3.25+, SDL2, SDL2_image, SDL2_ttf and OpenSSL.
Multiplayer uses [Nexilis](https://github.com/nexilistudios/nexilis), included as a git submodule.
```sh
git clone --recurse-submodules <repository>   # or: git submodule update --init --recursive
cmake -S . -B build
cmake --build build
./build/nx-chess
```
On Nix, `nix develop` opens a shell with all of the above.

#### Playing:
Without options both players share one computer. For a network game, one player hosts and plays white,
the other joins and plays black:
```sh
./build/nx-chess --host --password secret
./build/nx-chess --join 192.168.1.10 --password secret
```
The password can also be given with `NX_CHESS_PASSWORD`. With a password, players are authenticated and the
connection is encrypted with TLS-PSK; without one, the game is open and unencrypted, so only use that on a
trusted network. The host listens on TCP ports 54200 and 54202. Run `./build/nx-chess --help` for all options.
