# R-Type

A networked multiplayer remake of **R-Type**, the 1987 arcade shoot'em up.
Up to four players team up online, each piloting a spaceship against waves
of enemies.

## The game

- **Cooperative multiplayer**: one server hosts the game, and up to four
  players join it over the network from their own computers.
- **Classic arcade action**: move your ship around the screen, shoot the
  enemies coming from the right, and avoid getting hit.
- **One hit, one life**: a single enemy shot or collision destroys your ship,
  and every player earns points for each enemy shot down. If you die, you watch
  the others until the whole team is out and the final scores are shown.
- **Fair play**: the server is the only judge of what happens in the game, so
  every player sees the same game.
- **Your game, your rules**: before starting, choose the difficulty (easy,
  normal, hard), the level to start at (or the endless mode) and the boss.

### Controls

Every action has a key for each hand, so the game can be played with one
hand only.

| Left hand | Right hand | Action |
|-----------|------------|--------|
| W A S D | Arrow keys | Move the ship |
| Space | Right Ctrl | Shoot: one shot per press. Held for 1 second, then released: a charged shot that goes through enemies |
| F | Right Shift | Auto-fire on or off: the ship shoots on its own, 4 times per second |
| Escape | Escape | Leave the room. In the room list: back to the main menu |

In the menus, the arrow keys move the selection, and Space or Enter chooses.

**Choose your own keys**: write a `controls.cfg` file in the folder you start
the game from (or give its path with `--controls FILE`). Each line gives the
keys of one action and replaces its default keys:

```text
# action = key key...
fire = Enter Space
autofire = Tab
```

- Actions: `up`, `down`, `left`, `right`, `fire`, `autofire`.
- Keys: `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `LeftShift`,
  `RightShift`, `LeftCtrl`, `RightCtrl`, `LeftAlt`, `RightAlt`, the letters
  `A` to `Z` and the digits `0` to `9`. With the default (Raylib) client a
  letter is the key at that place on a US keyboard: W A S D are the keys
  Z Q S D of a French AZERTY keyboard.
- A line the game cannot read is reported when it starts, and that action
  keeps its default keys.

**Change the volume**: open Settings in the main menu. The game saves your
choice in a `settings.cfg` file in the folder you start it from, which you can
also edit by hand. A volume goes from 0 to 1:

```text
# name = value
music_volume = 0.4
sound_volume = 1
```

A line the game cannot read is reported when it starts, and that setting keeps
its value. Saving replaces the whole file, so your comments are not kept.

## Getting started

R-Type runs on **Linux** and **Windows**. A single script builds and launches
everything:

```bash
./rtype.py              # on Windows: python rtype.py
```

It needs Python 3, a C++20 compiler and Docker (or `--native` to do without
Docker); it installs the rest, such as CMake and Conan, by itself. The game uses
[Raylib](https://www.raylib.com/) and [Asio](https://think-async.com/Asio/), which
Conan downloads. See the [installation guide](docs/INSTALLATION.md) for the
requirements and all the options.

## Documentation

The full documentation is online, with an outline and a search bar:
**<https://t3a1.github.io/RType-mirror/>** (or `./rtype.py --docs` to build it
locally).

| Document | What you will find |
|----------|--------------------|
| [Installation guide](docs/INSTALLATION.md) | Requirements, building, running the server and the clients, troubleshooting. |
| [Architecture](docs/ARCHITECTURE.md) | How the code is organized: game engine, subsystems, game loop, threads. |
| [Network protocol](docs/PROTOCOL.md) | The messages exchanged between the server and the clients. |
| [Technical study](docs/TECHNICAL_STUDY.md) | Why these technologies: comparisons, storage, security, accessibility, before and after. |
| [How-to guides](docs/HOWTO.md) | Adding a component, a system, an enemy or a packet; building and debugging. |
| [Levels](docs/LEVELS.md) | Writing a level: the JSON file of its enemy waves, and how the server plays the levels. |
| [Weapons](docs/WEAPONS.md) | The charged shot, the power-ups and the weapon settings file. |
| [Network testing](docs/NETWORK_TESTING.md) | Playing with added latency, packet loss, duplication and low bandwidth on one machine. |
| [Network performance comparison](docs/NETWORK_COMPARISON.md) | Measured effect of interpolation and prediction on a degraded network, compared with the Part 1 prototype. |
| [Contributing](docs/CONTRIBUTING.md) | How to work on the project: workflow, conventions, tools. |
| [Contributors](docs/CONTRIBUTORS.md) | Who made the project, and how to reach us. |

## Project status

| Milestone | Date | Content |
|-----------|------|---------|
| Part 1 | October 18, 2026 | Game engine, networking and a playable prototype. |
| Part 2 | November 8, 2026 | Advanced features, built on top of the prototype. |

## Authors

Léandre Fouret, Romane Cauvez, Thomas Peze and David Caron: see the
[contributors](docs/CONTRIBUTORS.md).
