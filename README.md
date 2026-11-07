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
| F | Right Shift | Auto-fire, on by default: the ship shoots on its own, 10 times per second |
| Escape | Escape | Leave the room. In the room list: back to the main menu |
| T | T | Write to your room, in the waiting room or during the game: Enter sends, Escape cancels. The ship stops while you type |

In the menus, the arrow keys move the selection, and Space or Enter chooses.

**Play with a gamepad**: plug in an Xbox or a PlayStation gamepad, before or
during the game. It works at the same time as the keyboard:

| Xbox | PlayStation | Action |
|------|-------------|--------|
| D-pad or left stick | D-pad or left stick | Move the ship. In the menus: move the selection |
| A or RT | Cross or R2 | Shoot, and charged shot. In the menus: A (Cross) chooses |
| RB | R1 | Auto-fire on or off |
| B | Circle | In the menus: go back. Closes the chat. It never leaves a game |
| Back | Create | Leave the room |

The chat needs a keyboard to write. The stick moves the ship at full speed once it
is pushed past half of its range, in eight directions like the keyboard.

**Choose your own keys**: open Settings in the main menu, then the Keyboard tab.
Each action has two keys: choose one, then press the new key, or Escape to
cancel. The game tells you if another action uses the same key, and Reset keys
brings back the default ones. The Gamepad tab does the same for the buttons of
the gamepad, and Escape or B (Circle) cancels there. The arrow keys, Space,
Enter and Escape, and the D-pad, A (Cross) and B (Circle), always work in the
menus.

Your choice is saved in a `controls.cfg` file in the folder you start the game
from (or in the file given with `--controls FILE`), which you can also edit by
hand. Each line gives the keys of one action and replaces its default keys. A
line starting with `pad_` gives its gamepad buttons instead:

```text
# action = key key...
fire = Enter Space
autofire = Tab
pad_fire = PadA PadRT
```

- Actions: `up`, `down`, `left`, `right`, `fire`, `autofire`, `chat`, and the
  same names after `pad_` for the gamepad.
- Keys: `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `LeftShift`,
  `RightShift`, `LeftCtrl`, `RightCtrl`, `LeftAlt`, `RightAlt`, the letters
  `A` to `Z` and the digits `0` to `9`. With the default (Raylib) client a
  letter is the key at that place on a US keyboard: W A S D are the keys
  Z Q S D of a French AZERTY keyboard.
- Gamepad buttons, only on `pad_` lines: `PadUp`, `PadDown`, `PadLeft`,
  `PadRight` (D-pad), `PadStickUp`, `PadStickDown`, `PadStickLeft`,
  `PadStickRight` (left stick), `PadA`, `PadB` (Cross, Circle), `PadLB`,
  `PadRB`, `PadLT`, `PadRT` (L1, R1, L2, R2), `PadStart`, `PadSelect`
  (Options, Create).
- A line the game cannot read is reported when it starts, and that action
  keeps its default keys.

**Change the volume or the window**: open Settings in the main menu. The Audio tab
holds the volumes. The Video tab holds the fullscreen mode, the size of the window,
V-Sync and the frame limit, which V-Sync replaces while it is on. The game saves your
choice in a `settings.cfg` file in the folder you start it from, which you can
also edit by hand. A volume goes from 0 to 1:

```text
# name = value
music_volume = 0.4
sound_volume = 1
fullscreen = off
window_size = 1280x720
vsync = on
frame_limit = 0
auto_fire = on
player_labels = on
background = on
reduce_flashing = off
```

Window sizes: `1280x720`, `1600x900`, `1920x1080`, `2560x1440`. Frame limits: `15`,
`30`, `60`, `120`, or `0` for no limit. A line the game cannot read is reported when it
starts, and that setting keeps its value. Saving replaces the whole file, so your comments are not kept.

**Accessibility options**: open Accessibility in the main menu.

- **Auto-fire**: auto-fire is on when the game starts, by default. The auto-fire
  key still turns it on or off during a session, without changing this option.
- **Player labels**: shows "P1" and "(you)" above the ships. Without them, only
  the color tells the ships apart during a game.
- **Background**: draws the scrolling stars and planets, by default. Off, the
  game is drawn on a still black screen, for players bothered by the motion or
  who need more contrast.
- **Reduce flashing**: off by default. On, destroyed ships and enemies vanish
  without an explosion (their sound still plays) and the glow of a charging
  shot stops flickering. The game has no screen shake.

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

To play on several machines, one of them runs only the server and the others
only the clients:

```bash
./rtype.py --server-only --native     # on the host
./rtype.py --clients-only -H <host>   # on each player's machine
```

The same options are also in a window: `./rtype.py --gui`.

## Documentation

The full documentation is online, with an outline and a search bar:
**<https://t3a1.github.io/RType-mirror/>** (or `./rtype.py --docs` to build it
locally). A button on each page switches to its French version
([version française](https://t3a1.github.io/RType-mirror/fr/)).

| Document | What you will find |
|----------|--------------------|
| [Installation guide](docs/INSTALLATION.md) | Requirements, building, running the server and the clients, troubleshooting. |
| [Architecture](docs/ARCHITECTURE.md) | How the code is organized: game engine, subsystems, game loop, threads. |
| [Network protocol](docs/PROTOCOL.md) | The messages exchanged between the server and the clients. |
| [Technical study](docs/TECHNICAL_STUDY.md) | Why these technologies: comparisons, storage, security, accessibility, before and after. |
| [How-to guides](docs/HOWTO.md) | Adding a component, a system, an enemy, a packet, a sound, a menu page or a setting; building and debugging. |
| [Levels](docs/LEVELS.md) | Writing a level: the JSON file of its enemy waves, and how the server plays the levels. |
| [Weapons](docs/WEAPONS.md) | The charged shot, the power-ups and the weapon settings file. |
| [Enemies](docs/ENEMIES.md) | The enemy file: the stats of every enemy, and new enemies and bosses without writing code. |
| [Network testing](docs/NETWORK_TESTING.md) | Playing with added latency, packet loss, duplication and low bandwidth on one machine. |
| [Network performance comparison](docs/NETWORK_COMPARISON.md) | Measured effect of interpolation and prediction on a degraded network, compared with the Part 1 prototype. |
| [Contributing](docs/CONTRIBUTING.md) | How to work on the project: workflow, conventions, tools. |
| [Contributors](docs/CONTRIBUTORS.md) | Who made the project, and how to reach us. |

## Project status

| Milestone | Date | Content |
|-----------|------|---------|
| Part 1 | October 18, 2026 | Game engine, networking and a playable prototype. |
| Part 2 | November 8, 2026 | Advanced features, built on top of the prototype. |

## License

R-Type is free software, under the [GNU General Public License v3.0](LICENSE).

The license covers our code and our documentation, not the third-party assets of
`client/assets/`: the R-Type sprite sheets remain the property of their rights
holders, and DejaVu Sans has its own license, next to the font.

## Authors

Léandre Fouret, Romane Cauvez, Thomas Peze and David Caron: see the
[contributors](docs/CONTRIBUTORS.md).
