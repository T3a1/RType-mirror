# Installation

This guide explains how to build and run R-Type on Linux, macOS and Windows,
either with the launcher script `rtype.py` (recommended) or by hand with Conan
and CMake.

## Contents

- [1. Requirements](#1-requirements)
- [2. Quick start](#2-quick-start)
- [3. The launcher: rtype.py](#3-the-launcher-rtypepy)
- [4. Building by hand](#4-building-by-hand)
- [5. Running the programs](#5-running-the-programs)
- [6. Developer tools](#6-developer-tools)
- [7. Troubleshooting](#7-troubleshooting)

## 1. Requirements

| Tool | Needed for | Notes |
|------|------------|-------|
| Git | Getting the sources | |
| Python 3.8 or newer | `rtype.py` | `python3` on Linux and macOS, `python` on Windows. Standard library only. |
| C++20 compiler | Building | See the table below. |
| CMake 3.15+ and Conan 2 | Building | Installed automatically by `rtype.py` if missing. |
| Docker | Running the server in a container (default mode) | Optional: use `--native` to run it without Docker. |
| clang-tidy | `--tidy`, `--fix` | Optional. |
| Doxygen, Graphviz | Generating the documentation website | Optional. |
| tkinter | The graphical launcher, `--gui` | Optional. Part of Python, but packaged apart on Linux (`sudo apt install python3-tk`, `sudo dnf install python3-tkinter`, `sudo pacman -S tk`) and with Homebrew's Python (`brew install python-tk`). Python from python.org already has it on Windows and macOS. |
| ffmpeg | `tools/generate_sounds.py`, to generate the sounds again | Optional: the generated files are in the repository. |

What to install by hand on each OS (everything else is installed by
`rtype.py`):

| OS | Compiler | Docker | clang-tidy | Doxygen, Graphviz |
|----|----------|--------|------------|-------------------|
| Debian, Ubuntu | `sudo apt install build-essential` | Docker Engine | `sudo apt install clang-tidy` | `sudo apt install doxygen graphviz` |
| Fedora | `sudo dnf install gcc-c++` | Docker Engine | `sudo dnf install clang-tools-extra` | `sudo dnf install doxygen graphviz` |
| Arch | `sudo pacman -S base-devel` | Docker Engine | `sudo pacman -S clang` | `sudo pacman -S doxygen graphviz` |
| macOS (Intel or Apple Silicon) | `xcode-select --install` | Docker Desktop | `brew install llvm` | `brew install doxygen graphviz` |
| Windows 10/11 | Installed by `rtype.py` (MSVC, through winget) | Docker Desktop (Linux containers) | `winget install LLVM.LLVM` | `winget install DimitriVanHeesch.Doxygen Graphviz.Graphviz` |

When something is missing, `rtype.py` stops with a message that says what to
install.

Supported platforms:

- **Linux**: required by the project, and tested in the CI on Ubuntu 24.04.
- **macOS**: supported by `rtype.py` (tested by hand on Apple Silicon, not in
  the CI).
- **Windows 10/11**: supported by `rtype.py`. Crossplay with Linux is still
  under discussion.

All third-party libraries (Raylib, SFML or ncurses, Asio, nlohmann-json, GoogleTest) come from the Conan
package manager. Nothing is copied into the repository, and no library has to
be installed system-wide.

## 2. Quick start

```bash
git clone https://github.com/T3a1/RType-mirror.git rtype
cd rtype
./rtype.py              # on Windows: python rtype.py
```

On Windows, replace `./rtype.py` with `python rtype.py` in every command of
this guide.

The first launch takes a few minutes: it installs the missing build tools,
downloads the libraries, builds the client, builds the server's Docker image,
then starts the server and one client.

To play without Docker, or with friends on the same machine:

```bash
./rtype.py --native          # server built and run on your machine
./rtype.py --native -c 2     # same, with two clients
./rtype.py --native --room 3 # same, directly in room 3, without the lobby
./rtype.py --graphics sfml   # client built with SFML instead of Raylib
./rtype.py --native -g ncurses   # client drawn in this terminal (not on Windows)
```

To play on several machines, start the server alone on one of them, then only
the clients on the others:

```bash
./rtype.py --server-only --native         # on the host: no client, runs until quit or Ctrl+C
./rtype.py --clients-only -H 192.168.1.20 # on each player's machine: the host's address
./rtype.py --clients-only -H 192.168.1.20 -c 2   # two players on the same machine
```

The server listens on every network address of the host. Its firewall must let
UDP port 4242 in (or the port given with `-p`, the same on both sides).

## 3. The launcher: rtype.py

`rtype.py` works from any directory and on every OS: it only uses Python's
standard library and never goes through a shell. `./rtype.py --help` lists
the options. Before building, it sets up everything that is missing:

- `cmake`, `conan` and `gcovr` are installed with pip into a local virtual
  environment, `.venv/` (no admin rights, ignored by Git).
- A default Conan profile is created from your compiler if you have none.
- On Linux, Conan installs the X11 and OpenGL development packages that the
  graphics library needs with your package manager (apt, dnf, pacman...). It
  asks for your `sudo` password only if one of them is missing. macOS and
  Windows need no system package.
- On Windows, if no MSVC compiler is found, it installs Visual Studio 2022
  Build Tools with the C++ workload through `winget` (several GB, asks for
  admin rights). On Linux and macOS, it stops and tells you how to install a
  compiler (see the table above).

### Options

| Option | Effect |
|--------|--------|
| *(none)* | Starts the server in Docker and one client. |
| `-G`, `--gui` | Opens the [graphical launcher](#graphical-launcher). |
| `-S`, `--server-only` | Starts only the server, in Docker or with `--native`, until Ctrl+C (or `quit` in its console). It prints the command the players run. Refuses the client options (`-c`, `-r`, `-H`). |
| `-O`, `--clients-only` | Starts only the clients, connected to a server already running at `-H` and `-p`. Refuses the server options (`-n`, `--banlist`, `--whitelist`). |
| `-n`, `--native` | Runs the server natively instead of in Docker. |
| `-c N`, `--clients N` | Starts `N` clients (default: 1). |
| `-r N`, `--room N` | The clients enter room `N` directly, created if needed (default: they start in the main menu, and Play leads to the lobby, where they choose a room). One server runs up to 16 independent games. |
| `-p PORT`, `--port PORT` | UDP port used by the clients (default: 4242). |
| `-H HOST`, `--host HOST` | Address the clients connect to (default: 127.0.0.1). |
| `-g LIB`, `--graphics LIB` | Graphics library of the client: `raylib` (default), `sfml` or `ncurses` (see [Terminal client](#terminal-client-ncurses)). Changing it reconfigures and rebuilds `build/`. |
| `-T`, `--test` | Builds and runs the unit tests, then exits. |
| `-C`, `--coverage` | Runs the unit tests with coverage and writes an HTML report. |
| `-t`, `--tidy` | Runs clang-tidy on the project. |
| `-f`, `--fix` | Applies the clang-tidy fixes, then runs clang-format. |
| `-F`, `--format` | Runs clang-format on the project (same version as the CI). |
| `--check-format [FILE...]` | Checks the formatting like the CI, without changing anything. |
| `-d`, `--docs` | Generates the documentation website with Doxygen: the pages of `docs/` and `docs/fr/`, and the code reference. |

> In Docker mode, `--port` changes the port published on your machine; inside the
> container, the server stays on 4242.

The client (and the server with `--native`) is rebuilt from the current
sources at every launch, which takes a second when nothing changed. The build
is configured again when `build/` is new, when `conanfile.py` changed, or when
`--graphics` names another library. Docker mode builds the server inside the
image, never into `build/`.

### Graphical launcher

`./rtype.py --gui` opens a window with the game options of the command line:
what to start (server and clients, server only, or clients only), Docker or
native server, number of clients, server address and port, room and graphics
library.

- The window shows the **command** that matches its settings, and greys out
  the options that the chosen mode does not use. Everything it does can also
  be typed in a terminal.
- **Start** runs that command and shows its output in the window. **Stop**
  stops it like Ctrl+C: the server and the clients it started are stopped too.
  Closing the window also stops it.
- **Server console**: a line typed there goes to the administration console of
  the server, in Docker or native (`help`, `status`, `kick`, `ban`...).
- **On a very high resolution screen**, the window enlarges itself from the
  resolution and the DPI of the screen. If the result does not suit you, set
  `RTYPE_GUI_SCALE`, for example `RTYPE_GUI_SCALE=2 ./rtype.py --gui`.
- To **create a game**, start the server and clients with a room number, or
  room 0 to choose one in the lobby. To **join a game** on another machine,
  choose "Clients only" with its address. Several rooms are several games run
  in parallel by the same server.

Without tkinter, `--gui` says how to install it (see
[Requirements](#1-requirements)); the command line does the same things.

### Environment variables

| Variable | Default | Meaning |
|----------|---------|---------|
| `BUILD_DIR` | `build` | Build directory. |
| `COVERAGE_DIR` | `build-coverage` | Build directory of `--coverage`. |
| `SERVER_BIN`, `CLIENT_BIN` | *(found in `BUILD_DIR`)* | Paths to existing binaries. |
| `IMAGE`, `CONTAINER` | `rtype-server` | Docker image and container names. |
| `RTYPE_GUI_SCALE` | *(from the screen)* | How much the graphical launcher enlarges its window, from `0.5` to `4`. |
| `GRAPHICS` | `raylib` | Default of `--graphics`. |
| `CONAN_SYSTEM_PACKAGES` | `install` | Linux only: `check` only checks the X11/OpenGL packages, `report` only lists them. |
| `TOOLS_DIR` | `.venv` | Virtual environment for the build tools. |
| `NO_COLOR` | *(unset)* | Disables colored output. |

## 4. Building by hand

From the repository root:

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Release
cmake -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executables `r-type_server` and `r-type_client` are written to `build/`
(`build/Release/` with Visual Studio).

The client uses Raylib by default. To build it with SFML (or ncurses), choose
the library for Conan, which installs it and passes the choice on to CMake. On a build
directory that was already configured, also pass it to CMake:

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Release -o "&:graphics=sfml"
cmake -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DRTYPE_GRAPHICS_BACKEND=sfml
```

CMake options:

| Option | Default | Effect |
|--------|---------|--------|
| `BUILD_CLIENT` | `ON` | Builds the client. `OFF` builds the server only (it still needs Conan for Asio). |
| `RTYPE_GRAPHICS_BACKEND` | `raylib` | Graphics library of the client: `raylib`, `sfml` or `ncurses`. It must match the Conan option `graphics`. |
| `RTYPE_NETWORK_BACKEND` | `asio` | Network library behind `engine::net::IUdpSocket`. Only `asio` exists. |
| `BUILD_TESTS` | `OFF` | Builds the unit tests (`r-type_tests`). |
| `ENABLE_COVERAGE` | `OFF` | Instruments the code for test coverage (GCC/Clang). |

## 5. Running the programs

### Server

```bash
./build/r-type_server
```

The server listens on UDP port 4242, or on the port given as its first argument
(`./build/r-type_server 5000`), and stops cleanly on Ctrl+C.

To run it in Docker without the launcher:

```bash
docker build -t rtype-server .
docker run -dit --rm --init --name rtype-server -p 4242:4242/udp rtype-server
docker logs -f rtype-server     # follow its output
docker attach rtype-server      # type administration commands (leave: Ctrl+P, Ctrl+Q)
docker stop rtype-server        # stop and remove it
```

#### Administration console

Type these commands in the terminal of the server, while it runs. With
`rtype.py`, in Docker as natively, that is the terminal it was started from (or
the **Server console** of the graphical launcher). Next to an ncurses client, or
for a server started by hand in Docker, the container is detached: use
`docker attach rtype-server`.

| Command | Effect |
|---------|--------|
| `help` | Lists the commands. |
| `status` | Uptime, number of rooms and clients, access rules. |
| `rooms` | Lists the rooms, their status and players. |
| `players [room]` | Lists the clients: id, address, room, last packet. |
| `kick <id>` | Disconnects a client. |
| `ban <id or ip>` | Bans an address and disconnects its clients. |
| `unban <ip>`, `banlist` | Removes a ban, lists the bans. |
| `stop-room <room>` | Closes a room; its players go back to the lobby. |
| `whitelist on`, `off` | Only accepts the listed addresses, or everyone. |
| `whitelist add <ip>`, `remove <ip>`, `list` | Edits or shows the listed addresses. |
| `quit` | Stops the server. |

Bans and white lists use IPv4 addresses, and can be kept in text files, one
address per line (`#` starts a comment). They are read at startup and rewritten
at each change:

```bash
./build/r-type_server --banlist bans.txt --whitelist allowed.txt
```

There are no player accounts, so these lists can only name addresses: a ban hits
everyone behind the same address, and a player who changes address gets around
it.

`--whitelist` also turns the white list on. `--no-console` does not read the
standard input, for a server started by a script. The launcher has the same
`--banlist` and `--whitelist` options, with `--native` only.

The image contains a single static binary and runs as an unprivileged user.

### Client

```bash
./build/r-type_client [host] [port]
```

`host` defaults to `127.0.0.1` and `port` to `4242`. To join a server on
another machine, pass its IP address and make sure UDP port 4242 is open on
that machine's firewall.

The Raylib and SFML clients play sound effects and music. The ncurses client has no
sound.

The Raylib and SFML clients also read an Xbox or a PlayStation gamepad, plugged in
before or during the game. To see what the SFML client reads from a gamepad, set
`RTYPE_PAD_DEBUG`: it then logs the gamepad found, its vendor, and every button and
axis that changes.

```bash
RTYPE_PAD_DEBUG=1 ./rtype.py --native --graphics sfml
```

### Terminal client (ncurses)

Built with `--graphics ncurses`, the client draws the game in the terminal it
was started from. It shows that the client does not depend on its graphics
library, but the game is degraded:

- Not available on Windows; tested on Linux. A large terminal (about 160x45)
  gives the best result.
- Ships, missiles and enemies are colored blocks of characters, and there is
  no background image. Colors are rounded to the terminal's palette (256
  colors, or 8).
- A terminal only sends key presses and their repeats, never key releases. A
  key counts as held for about half a second after it is pressed, then as long
  as it repeats. Only one key works at a time: you cannot move and fire
  together.
- The chat of the room is shown, but you cannot write in it: `q` quits the game
  in this client, so the keys cannot be used for typing.
- Quit with `q` or Ctrl+C. Escape leaves the room, then goes back to the main
  menu from the room list.
- The logs go to `r-type_client.log` in the current directory instead of the
  terminal. With `--native`, `rtype.py` writes the server's output to
  `build/server.log`, and it starts one client only (start others from other
  terminals with `./build/r-type_client`).
- The terminal descriptions come from the system (`/usr/share/terminfo`...),
  unless `TERMINFO` or `TERMINFO_DIRS` is set.

## 6. Developer tools

| Command | Result |
|---------|--------|
| `./rtype.py --test` | Unit tests (GoogleTest). Same as `cmake -B build -DBUILD_TESTS=ON`, build, then `ctest --test-dir build`. |
| `./rtype.py --coverage` | Coverage report in `build-coverage/coverage/index.html`. |
| `./rtype.py --tidy` | clang-tidy, with the same rules as the CI. |
| `./rtype.py --fix` | clang-tidy fixes, then clang-format. |
| `./rtype.py --format` | clang-format only. |
| `./rtype.py --docs` | Documentation website in `build/docs/html/index.html`, the French version under `fr/`. A broken link between pages, or a page without its French version, is an error. |
| `./tools/generate_sounds.py` | Generates the sound effects and musics again, in `client/assets/`. |

Every clang-format version formats a little differently, so the launcher, the
pre-commit hook and the CI all use clang-format 18.1.3, installed from PyPI
into `.venv/`. Your system clang-format is never used.

The first run of `rtype.py` enables the project's Git hooks
(`git config core.hooksPath .githooks`). The pre-commit hook refuses a commit
whose C++ files are badly formatted: run `./rtype.py --format`, `git add` the
files again and commit. Use `git commit --no-verify` to skip it once.

On Windows, the hook runs in Git's own shell and uses the first of `python3`,
`python` and `py` that really runs (it skips the Microsoft Store placeholder).

`--tidy` and `--fix` need clang-tidy (see the table in
[section 1](#1-requirements)). Homebrew's `llvm` and the Windows LLVM installer
do not always add it to the `PATH`: `rtype.py` also looks for it in
`/opt/homebrew/opt/llvm/bin`, `/usr/local/opt/llvm/bin` and
`C:\Program Files\LLVM\bin`. clang-tidy reads `build/compile_commands.json`,
which the Visual Studio generator does not write: on Windows, configure
`build/` with Ninja to use it. `--coverage` needs GCC or Clang (gcov), so it
does not work with MSVC.

`--docs` needs Doxygen, version 1.18 to draw the Mermaid diagrams of the pages
(the CI uses this version). Install Graphviz too to get the class diagrams;
without it the documentation is generated without them.

## 7. Troubleshooting

**CMake cannot find raylib.** The build directory was configured without the
Conan toolchain. Delete `build/`, or let `rtype.py` reconfigure it (it does so
automatically).

**CMake cannot find SFML, ncurses or raylib after switching library.** Conan installed
the other library: `RTYPE_GRAPHICS_BACKEND` and the Conan option `graphics`
must name the same one. Run `conan install` again with `-o "&:graphics=..."`,
or use `./rtype.py --graphics ...`, which keeps them in sync.

**`conan install` failed.** The full output is in `build/conan-install.log`.
The usual cause on Linux is a missing X11/OpenGL package that could not be
installed: run `CONAN_SYSTEM_PACKAGES=report ./rtype.py` to list them.

**`Docker is installed but not running`.** Start Docker Desktop (macOS,
Windows) or the Docker service (`sudo systemctl start docker` on Linux), or
use `--native`.

**`your user cannot use Docker`.** On Linux, your user is not in the `docker`
group. Add it (`sudo usermod -aG docker $USER`, then log in again) or use
`--native`.

**`no C++ compiler`.** Install one as shown in
[section 1](#1-requirements), then run `rtype.py` again. On macOS, `rtype.py`
checks for the Xcode Command Line Tools.

**The client does not connect.** Check that the server is running
(`docker logs rtype-server` in Docker mode), that the host and port match,
and that no firewall blocks UDP port 4242.

**No sound.** Without an audio device, the client prints `No audio device: the game
runs without sound` and plays normally. If a file cannot be read, it prints `Cannot
load the sound` or `Cannot load the music` with its name: build the client again to
copy `client/assets/` next to it. The ncurses client never plays sound, and
`--bench` mutes the game.

**A gamepad button does nothing with the SFML client.** SFML numbers the buttons as
the system does, and only the layout of a DualSense on Linux has been checked. Run
the client with `RTYPE_PAD_DEBUG=1`, press the button, and fix its number in the
`PadLayout` of `client/src/graphics/sfml/SfmlGraphics.cpp`. With an Xbox gamepad on
Windows, SFML may give both triggers on one axis, so that pressed together they
cancel out: this has not been checked.

**Windows: MSVC still not found.** Open the Visual Studio Installer and add the
"Desktop development with C++" workload to Build Tools 2022.
