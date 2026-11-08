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

import os
import queue
import shlex
import signal
import subprocess
import sys
import threading
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


def interface_scale(dpi_scale: float, screen_height: int,
                    override: str | None = None) -> float:
    """Returns how much to enlarge the window compared with a 1080p screen.
    The system's DPI is not enough: it often stays at 96 on Linux, whatever the
    resolution. So the height of the screen also counts, in steps of 25 %.
    RTYPE_GUI_SCALE, a number from 0.5 to 4, overrides it all."""
    if override:
        try:
            value = float(override)
        except ValueError:
            value = 0.0
        if 0.5 <= value <= 4.0:
            return value
    by_height = round(screen_height / 1080 * 4) / 4
    return min(max(dpi_scale, by_height, 1.0), 4.0)


def make_dpi_aware() -> None:
    """On Windows, asks for the real pixels: otherwise the system stretches
    the window and it looks blurry. Must be called before the window opens."""
    if sys.platform != "win32":
        return
    try:
        import ctypes
        ctypes.windll.shcore.SetProcessDpiAwareness(1)
    except (AttributeError, OSError):
        pass


def apply_scale(root) -> float:
    """Enlarges the fonts of a window to fit the screen; returns the scale.
    Fonts in points follow Tk's scaling, and fonts in pixels (a negative size)
    must be changed one by one."""
    from tkinter import font
    points_to_pixels = float(root.tk.call("tk", "scaling"))
    scale = interface_scale(points_to_pixels * 72 / 96, root.winfo_screenheight(),
                            os.environ.get("RTYPE_GUI_SCALE"))
    root.tk.call("tk", "scaling", scale * 96 / 72)
    for name in font.names(root):
        named = font.nametofont(name, root=root)
        size = int(named.cget("size"))
        if size < 0:
            named.configure(size=round(size * scale))
    return scale


class LauncherWindow:
    """The window: a form of GameSettings, and the command it gives."""

    def __init__(self, tk) -> None:
        """Builds the window. tk is the tkinter module, imported by main() so
        that the rest of this file works without it."""
        self.tk = tk
        from tkinter import ttk
        self.ttk = ttk
        make_dpi_aware()
        self.root = tk.Tk()
        self.scale = apply_scale(self.root)
        self.root.title("R-Type launcher")
        self.root.minsize(self.px(560), self.px(420))
        defaults = GameSettings()
        self.mode = tk.StringVar(value=defaults.mode)
        self.native = tk.BooleanVar(value=defaults.native)
        self.clients = tk.StringVar(value=str(defaults.clients))
        self.host = tk.StringVar(value=defaults.host)
        self.port = tk.StringVar(value=str(defaults.port))
        self.room = tk.StringVar(value=str(defaults.room))
        self.graphics = tk.StringVar(value=defaults.graphics)
        self.command = tk.StringVar()
        self.status = tk.StringVar(value="Ready.")
        self.client_fields: list = []
        self.server_fields: list = []
        self.process: subprocess.Popen | None = None
        self.lines: queue.Queue[str | None] = queue.Queue()
        self._build_form()
        self._build_output()
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        for variable in (self.mode, self.native, self.clients, self.host,
                         self.port, self.room, self.graphics):
            variable.trace_add("write", lambda *_: self.refresh())
        self.refresh()

    def px(self, size):
        """Scales a size in pixels, or a tuple of sizes, to the screen."""
        if isinstance(size, tuple):
            return tuple(round(value * self.scale) for value in size)
        return round(size * self.scale)

    def _build_form(self) -> None:
        """Lays out the settings, the command and the status line."""
        ttk = self.ttk
        form = ttk.Frame(self.root, padding=self.px(12))
        form.grid(row=0, column=0, sticky="nsew")
        self.root.columnconfigure(0, weight=1)
        form.columnconfigure(1, weight=1)

        ttk.Label(form, text="Start").grid(row=0, column=0, sticky="w")
        modes = ttk.Frame(form)
        modes.grid(row=0, column=1, sticky="w")
        for value, label in MODES.items():
            ttk.Radiobutton(modes, text=label, value=value,
                            variable=self.mode).pack(side="left", padx=self.px((0, 12)))

        native = ttk.Checkbutton(form, variable=self.native,
                                 text="Run the server here instead of in Docker")
        native.grid(row=1, column=1, sticky="w", pady=self.px(4))
        self.server_fields.append(native)

        rows = (
            ("Clients", ttk.Spinbox(form, from_=1, to=4, width=6,
                                    textvariable=self.clients), True),
            ("Server address", ttk.Entry(form, textvariable=self.host), True),
            ("Port (UDP)", ttk.Entry(form, width=8, textvariable=self.port),
             False),
            ("Room (0: lobby)", ttk.Spinbox(form, from_=0, to=16, width=6,
                                            textvariable=self.room), True),
            ("Graphics", ttk.Combobox(form, values=GRAPHICS_CHOICES,
                                      state="readonly", width=10,
                                      textvariable=self.graphics), True),
        )
        for row, (label, widget, client_only) in enumerate(rows, start=2):
            ttk.Label(form, text=label).grid(row=row, column=0, sticky="w",
                                             padx=self.px((0, 12)), pady=self.px(2))
            widget.grid(row=row, column=1, sticky="w" if label != "Server address"
                        else "ew", pady=self.px(2))
            if client_only:
                self.client_fields.append(widget)

        ttk.Label(form, text="Command").grid(row=7, column=0, sticky="w",
                                             pady=self.px((10, 0)))
        ttk.Entry(form, textvariable=self.command, state="readonly").grid(
            row=7, column=1, sticky="ew", pady=self.px((10, 0)))
        ttk.Label(form, textvariable=self.status, foreground="#a33").grid(
            row=8, column=0, columnspan=2, sticky="w", pady=self.px((6, 0)))
        self.form = form

    def _build_output(self) -> None:
        """Lays out the Start and Stop buttons and the output of the game."""
        ttk = self.ttk
        buttons = ttk.Frame(self.root, padding=self.px((12, 0)))
        buttons.grid(row=1, column=0, sticky="ew")
        self.start_button = ttk.Button(buttons, text="Start", command=self.start)
        self.start_button.pack(side="left")
        self.stop_button = ttk.Button(buttons, text="Stop", command=self.stop)
        self.stop_button.pack(side="left", padx=self.px(8))
        self.stop_button.state(["disabled"])

        output = ttk.Frame(self.root, padding=self.px(12))
        output.grid(row=2, column=0, sticky="nsew")
        self.root.rowconfigure(2, weight=1)
        output.columnconfigure(0, weight=1)
        output.rowconfigure(0, weight=1)
        self.output = self.tk.Text(output, height=12, wrap="word",
                                   state="disabled")
        self.output.grid(row=0, column=0, sticky="nsew")
        scroll = ttk.Scrollbar(output, command=self.output.yview)
        scroll.grid(row=0, column=1, sticky="ns")
        self.output.configure(yscrollcommand=scroll.set)

        # The administration console of the server reads this input.
        console = ttk.Frame(self.root, padding=self.px((12, 0, 12, 12)))
        console.grid(row=3, column=0, sticky="ew")
        console.columnconfigure(1, weight=1)
        ttk.Label(console, text="Server console").grid(row=0, column=0,
                                                        padx=self.px((0, 8)))
        self.console_line = self.tk.StringVar()
        entry = ttk.Entry(console, textvariable=self.console_line)
        entry.grid(row=0, column=1, sticky="ew")
        entry.bind("<Return>", lambda _: self.send())
        ttk.Button(console, text="Send", command=self.send).grid(
            row=0, column=2, padx=self.px((8, 0)))

    def send(self) -> None:
        """Sends the console line to the running command: with a
        server, its administration commands (help, status, kick...)."""
        line = self.console_line.get().strip()
        if not line or self.process is None or self.process.stdin is None:
            return
        try:
            self.process.stdin.write(line + "\n")
            self.process.stdin.flush()
        except OSError:
            self.status.set("The command does not read input any more.")
            return
        self.write(f"> {line}\n")
        self.console_line.set("")

    def write(self, text: str) -> None:
        """Appends text to the output and scrolls to it."""
        self.output.configure(state="normal")
        self.output.insert("end", text)
        self.output.see("end")
        self.output.configure(state="disabled")

    def start(self) -> None:
        """Runs the command of the settings, unless one is already running or
        the settings are wrong."""
        settings = self.settings()
        problem = "Clients, port and room are whole numbers." if settings is None \
            else check(settings)
        if problem or self.process is not None or settings is None:
            self.status.set(problem or "Already running: stop it first.")
            return
        self.write(f"$ {shown_command(settings)}\n")
        # A process group of its own: Stop reaches rtype.py, which then stops
        # the server and the clients it started.
        extra = ({"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP}
                 if sys.platform == "win32" else {"start_new_session": True})
        # Unbuffered, so that its lines show up as soon as they are written.
        env = {**os.environ, "PYTHONUNBUFFERED": "1"}
        self.process = subprocess.Popen(
            command(settings), cwd=ROOT, env=env, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            errors="replace", bufsize=1, **extra)
        threading.Thread(target=self._read, args=(self.process,),
                         daemon=True).start()
        self.start_button.state(["disabled"])
        self.stop_button.state(["!disabled"])
        self.status.set("Running.")
        self.root.after(100, self._poll)

    def _read(self, process: subprocess.Popen) -> None:
        """Thread: passes every line of the process to the window, then None
        once it is done. Tk is only touched from its own thread (_poll)."""
        assert process.stdout is not None
        for line in process.stdout:
            self.lines.put(line)
        self.lines.put(None)

    def _poll(self) -> None:
        """Shows the new lines of the process, until it is done."""
        while True:
            try:
                line = self.lines.get_nowait()
            except queue.Empty:
                self.root.after(100, self._poll)
                return
            if line is None:
                self._finished()
                return
            self.write(line)

    def _finished(self) -> None:
        """Shows how the process ended and lets a new one start."""
        code = self.process.wait() if self.process is not None else 0
        self.process = None
        self.write(f"[finished with status {code}]\n")
        self.start_button.state(["!disabled"])
        self.stop_button.state(["disabled"])
        self.status.set("Ready." if code == 0 else f"Stopped (status {code}).")

    def stop(self) -> None:
        """Asks rtype.py to stop, like Ctrl+C does: it stops the server and
        the clients it started, then exits."""
        if self.process is None or self.process.poll() is not None:
            return
        if sys.platform == "win32":
            self.process.send_signal(signal.CTRL_BREAK_EVENT)
        else:
            self.process.send_signal(signal.SIGINT)
        self.status.set("Stopping...")

    def close(self) -> None:
        """Closes the window, stopping the game first."""
        self.stop()
        if self.process is not None:
            try:
                self.process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.process.kill()
        self.root.destroy()

    def settings(self) -> GameSettings | None:
        """Reads the form, or returns None if a number is not one."""
        try:
            return GameSettings(
                mode=self.mode.get(), native=self.native.get(),
                clients=int(self.clients.get()), host=self.host.get().strip(),
                port=int(self.port.get()), room=int(self.room.get()),
                graphics=self.graphics.get())
        except ValueError:
            return None

    def refresh(self) -> None:
        """Updates the command shown, the problem of the settings if any, and
        greys out the fields that do nothing in the chosen mode."""
        mode = self.mode.get()
        for widget in self.client_fields:
            widget.state(["disabled"] if mode == "server" else ["!disabled"])
        for widget in self.server_fields:
            widget.state(["disabled"] if mode == "clients" else ["!disabled"])
        settings = self.settings()
        if settings is None:
            self.command.set("")
            self.status.set("Clients, port and room are whole numbers.")
            return
        self.command.set(shown_command(settings))
        self.status.set(check(settings) or "Ready.")

    def run(self) -> None:
        """Shows the window until it is closed."""
        self.root.mainloop()


def main() -> int:
    """Opens the window; returns 1 if tkinter is missing."""
    try:
        import tkinter
    except ImportError:
        print("[error] the graphical launcher needs tkinter (Linux: install "
              "python3-tk; macOS with Homebrew: brew install python-tk). "
              "The command line does the same: ./rtype.py --help",
              file=sys.stderr)
        return 1
    LauncherWindow(tkinter).run()
    return 0
