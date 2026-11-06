"""Graphical launcher of R-Type: a window over the options of rtype.py.

Started with `./rtype.py --gui` (`python rtype.py --gui` on Windows). The
window only builds a command line of rtype.py and runs it: every setting of
the window is an option of the command line, shown in the window, so what can
be done here can also be done in a terminal, and the other way around.

It needs tkinter, part of Python's standard library but packaged apart on some
Linux distributions (python3-tk); rtype.py says how to install it when it is
missing.
"""

from __future__ import annotations

import shlex
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent

# What the window starts, as the value of GameSettings.mode.
MODES = {
    "game": "Server and clients",
    "server": "Server only",
    "clients": "Clients only",
}
GRAPHICS_CHOICES = ("raylib", "sfml", "ncurses")


@dataclass
class GameSettings:
    """The settings of the window, one per game option of rtype.py."""

    mode: str = "game"
    native: bool = False
    clients: int = 1
    host: str = "127.0.0.1"
    port: int = 4242
    room: int = 0  # 0: the lobby, where the players choose a room
    graphics: str = "raylib"


def check(settings: GameSettings) -> str | None:
    """Returns what is wrong with the settings, or None if they can start.
    rtype.py checks them again; this only gives the message before starting."""
    if settings.mode not in MODES:
        return f"unknown mode {settings.mode!r}"
    if settings.graphics not in GRAPHICS_CHOICES:
        return f"unknown graphics library {settings.graphics!r}"
    if not 1 <= settings.port <= 65535:
        return "the port must be between 1 and 65535"
    if settings.mode != "server":
        if settings.clients < 1:
            return "start at least one client"
        if settings.room < 0:
            return "the room is a positive number, or 0 for the lobby"
        if not settings.host.strip():
            return "give the address of the server"
        if settings.graphics == "ncurses" and settings.clients > 1:
            return "the ncurses client uses the terminal: start one client"
    return None


def arguments(settings: GameSettings) -> list[str]:
    """Returns the options of rtype.py that start what the settings describe.
    Options left at their default are not written, and the options of the side
    that does not start are left out, as rtype.py refuses them."""
    args: list[str] = []
    if settings.mode == "server":
        args.append("--server-only")
    elif settings.mode == "clients":
        args.append("--clients-only")
    if settings.native and settings.mode != "clients":
        args.append("--native")
    if settings.mode != "server":
        if settings.clients != 1:
            args += ["--clients", str(settings.clients)]
        if settings.host != "127.0.0.1":
            args += ["--host", settings.host]
        if settings.room > 0:
            args += ["--room", str(settings.room)]
        if settings.graphics != "raylib":
            args += ["--graphics", settings.graphics]
    if settings.port != 4242:
        args += ["--port", str(settings.port)]
    return args


def command(settings: GameSettings) -> list[str]:
    """Returns the full command that the window runs: this Python, rtype.py
    and the options."""
    return [sys.executable, str(ROOT / "rtype.py"), *arguments(settings)]


def shown_command(settings: GameSettings) -> str:
    """Returns the command as a user types it in a terminal, to show it."""
    launcher = "python rtype.py" if sys.platform == "win32" else "./rtype.py"
    return " ".join([launcher, *(shlex.quote(arg) for arg in arguments(settings))])
