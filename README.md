# R-Type

A networked multiplayer remake of **R-Type**, the 1987 arcade shoot'em up.
Up to four players team up online, each piloting a spaceship against waves
of enemies.

## The game

- **Cooperative multiplayer**: one server hosts the game, and up to four
  players join it over the network from their own computers.
- **Classic arcade action**: move your ship around the screen, shoot the
  enemies coming from the right, and avoid getting hit.
- **Health and score**: a health bar follows each ship, and every player earns
  points for each enemy shot down.
- **Fair play**: the server is the only judge of what happens in the game, so
  every player sees the same game.

### Controls

| Key | Action |
|-----|--------|
| Arrow keys | Move the ship |
| Space | Shoot |

## Getting started

R-Type runs on **Linux** and **Windows**. A single script builds and launches
everything:

```bash
./rtype.py              # on Windows: python rtype.py
```

See the [installation guide](docs/INSTALLATION.md) for the requirements and
all the options.

## Documentation

| Document | What you will find |
|----------|--------------------|
| [Installation guide](docs/INSTALLATION.md) | Requirements, building, running the server and the clients, troubleshooting. |
| [Architecture](docs/ARCHITECTURE.md) | How the code is organized: game engine, subsystems, game loop, threads. |
| [Network protocol](docs/PROTOCOL.md) | The messages exchanged between the server and the clients. |
| [Contributing](docs/CONTRIBUTORS.md) | How to work on the project: workflow, conventions, tools. |
| Code documentation | Every class and function of the code, generated with Doxygen: <https://DOCS_URL_PLACEHOLDER> (or `./rtype.py --docs` to build it locally). |

## Project status

| Milestone | Date | Content |
|-----------|------|---------|
| Part 1 | October 18, 2026 | Game engine, networking and a playable prototype. |
| Part 2 | November 8, 2026 | Advanced features, built on top of the prototype. |
