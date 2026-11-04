#!/usr/bin/env python3
"""Network benchmark: measures what interpolation and prediction change.

Usage: `./tools/netbench.py [run | report] [options]` (Linux only: it uses
`tc netem` on the loopback interface, with sudo).

`run` plays every client mode under every network scenario of
docs/NETWORK_TESTING.md: for each pair it starts a fresh server with
`--invincible` and one client with `--bench`, which replays a fixed input
sequence and writes its measures to a CSV file in build/netbench/. `report`
reads these files and writes the results (tables and charts) between the
`<!-- results:begin -->` and `<!-- results:end -->` markers of
docs/NETWORK_COMPARISON.md. Without a command, both run.

The charts need matplotlib: it is installed into the `.venv/` of rtype.py if
missing. Build the server and the client first (`./rtype.py --native`).
"""

from __future__ import annotations

import argparse
import csv
import os
import signal
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = ROOT / os.environ.get("BUILD_DIR", "build")
VENV_DIR = ROOT / os.environ.get("TOOLS_DIR", ".venv")
VENV_PYTHON = VENV_DIR / "bin" / "python"
RESULTS_DIR = BUILD_DIR / "netbench"
DOC = ROOT / "docs" / "NETWORK_COMPARISON.md"
IMAGES_DIR = ROOT / "docs" / "images" / "network-comparison"
BEGIN_MARKER = "<!-- results:begin -->"
END_MARKER = "<!-- results:end -->"
PORT = 4242

# Name, netem settings (each packet is delayed once on lo: delay 50ms = RTT
# 100 ms).
SCENARIOS = {
    "normal": ("Normal network", "delay 0ms"),
    "latency": ("RTT 100 ms", "delay 50ms"),
    "high-latency": ("RTT 300 ms", "delay 150ms"),
    "jitter": ("RTT 100 ms ± 40 ms", "delay 50ms 20ms"),
    "loss-5": ("RTT 100 ms, 5 % loss", "delay 50ms loss 5%"),
    "loss-20": ("RTT 100 ms, 20 % loss", "delay 50ms loss 20%"),
    "bursts": ("RTT 100 ms, loss in bursts", "delay 50ms loss gemodel 1% 30%"),
}

# Name, client options.
MODES = {
    "snap": ("Part 1: snap to snapshots", ["--no-interpolation", "--no-prediction"]),
    "interpolation": ("Interpolation", ["--no-prediction"]),
    "prediction": ("Interpolation + prediction", []),
}

STILL = 0.01


class BenchError(Exception):
    """An error to report to the user without a traceback."""


def log(message: str) -> None:
    print(f"[netbench] {message}", flush=True)


# ---------- run ----------
def tc(*args: str, check: bool = True) -> None:
    """Runs `sudo tc` with these arguments."""
    result = subprocess.run(["sudo", "tc", *args], check=False,
                            stderr=None if check else subprocess.DEVNULL)
    if check and result.returncode != 0:
        raise BenchError(f"sudo tc {' '.join(args)} failed")


def netem_setup() -> None:
    """Sends the game's UDP traffic (port 4242, both ways) on lo through a
    netem queue, like docs/NETWORK_TESTING.md."""
    tc("qdisc", "del", "dev", "lo", "root", check=False)
    tc("qdisc", "add", "dev", "lo", "root", "handle", "1:", "prio")
    for direction in ("dport", "sport"):
        tc("filter", "add", "dev", "lo", "parent", "1:", "protocol", "ip", "u32",
           "match", "ip", "protocol", "17", "0xff",
           "match", "ip", direction, str(PORT), "0xffff", "flowid", "1:3")
    tc("qdisc", "add", "dev", "lo", "parent", "1:3", "handle", "30:", "netem", "delay", "0ms")


def netem_set(settings: str) -> None:
    tc("qdisc", "change", "dev", "lo", "parent", "1:3", "handle", "30:", "netem",
       *settings.split())


def netem_teardown() -> None:
    tc("qdisc", "del", "dev", "lo", "root", check=False)


def binary(name: str) -> Path:
    for path in (BUILD_DIR / name, BUILD_DIR / "Release" / name):
        if path.is_file():
            return path
    raise BenchError(f"{name} not found in {BUILD_DIR}: build it first (./rtype.py --native)")


def run_one(server: Path, client: Path, mode: str, output: Path, duration: float) -> None:
    """Runs one fresh server and one benchmarked client until the client is done."""
    server_process = subprocess.Popen([str(server), "--invincible", "--no-console"],
                                      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(0.5)
        result = subprocess.run(
            [str(client), "127.0.0.1", str(PORT), *MODES[mode][1],
             "--bench", str(duration), str(output)],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            timeout=duration + 30, check=False)
        if result.returncode != 0 or not output.is_file():
            raise BenchError(f"the client failed ({mode}): exit code {result.returncode}")
    except subprocess.TimeoutExpired as error:
        raise BenchError(f"the client did not finish ({mode})") from error
    finally:
        server_process.send_signal(signal.SIGINT)
        try:
            server_process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server_process.kill()


def run(scenarios: list[str], modes: list[str], duration: float, use_netem: bool) -> None:
    server, client = binary("r-type_server"), binary("r-type_client")
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    total = len(scenarios) * len(modes)
    log(f"{total} runs of {duration:g} s: do not touch the client windows.")
    if use_netem:
        log("tc needs sudo: enter your password if asked.")
        netem_setup()
    try:
        done = 0
        for scenario in scenarios:
            if use_netem:
                netem_set(SCENARIOS[scenario][1])
            for mode in modes:
                done += 1
                log(f"[{done}/{total}] {SCENARIOS[scenario][0]}, {MODES[mode][0]}")
                run_one(server, client, mode, RESULTS_DIR / f"{scenario}-{mode}.csv", duration)
    finally:
        if use_netem:
            netem_teardown()
    log(f"Measures written to {RESULTS_DIR}")


# ---------- report ----------
@dataclass
class Result:
    """The metrics of one run."""
    duration: float = 0.0
    frames: int = 0
    delays: list[float] = field(default_factory=list)
    missed: int = 0
    moves: list[float] = field(default_factory=list)
    received: float = 0.0
    sent: float = 0.0
    corrections: list[float] = field(default_factory=list)
    predicted: bool = False

    @property
    def fps(self) -> float:
        return self.frames / self.duration if self.duration else 0.0

    def delay_ms(self, quantile: float) -> float | None:
        if not self.delays:
            return None
        ordered = sorted(self.delays)
        return 1000 * ordered[min(len(ordered) - 1, int(quantile * len(ordered)))]

    @property
    def still_ratio(self) -> float:
        """Share of the frames where a synced entity did not move on screen."""
        return sum(m < STILL for m in self.moves) / len(self.moves) if self.moves else 0.0

    @property
    def jump(self) -> float:
        """99th percentile of the on-screen move of an entity in one frame."""
        if not self.moves:
            return 0.0
        ordered = sorted(self.moves)
        return ordered[int(0.99 * (len(ordered) - 1))]


def load(path: Path) -> Result:
    result = Result()
    with path.open(newline="") as file:
        for row in csv.DictReader(file):
            kind, t, a = row["kind"], float(row["time"]), float(row["a"])
            result.duration = max(result.duration, t)
            if kind == "frame":
                result.frames += 1
            elif kind == "move":
                result.moves.append(a)
            elif kind == "delay":
                if a < 0:
                    result.missed += 1
                else:
                    result.delays.append(a)
            elif kind == "correction":
                result.corrections.append(a)
            elif kind == "net" and t > 0:
                result.received = a / t
                result.sent = float(row["b"]) / t
    return result


def fmt(value: float | None, digits: int = 0, suffix: str = "") -> str:
    return "n/a" if value is None else f"{value:.{digits}f}{suffix}"


def table(results: dict, scenarios: list[str], title: str, cell) -> list[str]:
    modes = list(MODES)
    lines = [f"**{title}**", "",
             "| Scenario | " + " | ".join(MODES[m][0] for m in modes) + " |",
             "|---|" + "---:|" * len(modes)]
    for scenario in scenarios:
        cells = [cell(results[(scenario, m)]) if (scenario, m) in results else "n/a"
                 for m in modes]
        lines.append(f"| {SCENARIOS[scenario][0]} | " + " | ".join(cells) + " |")
    return lines + [""]


def ensure_matplotlib() -> None:
    """Imports matplotlib, installing it into .venv/ and restarting this script
    with the .venv/ interpreter if needed."""
    try:
        import matplotlib  # noqa: F401  pylint: disable=import-outside-toplevel,unused-import
        return
    except ImportError:
        pass
    if not VENV_PYTHON.is_file():
        raise BenchError("matplotlib is missing and .venv/ does not exist: run ./rtype.py once")
    has_it = subprocess.run([str(VENV_PYTHON), "-c", "import matplotlib"],
                            capture_output=True, check=False).returncode == 0
    if not has_it:
        log("Installing matplotlib in .venv/...")
        subprocess.run([str(VENV_PYTHON), "-m", "pip", "install", "--quiet",
                        "--disable-pip-version-check", "matplotlib"], check=True)
    if Path(sys.prefix).resolve() != VENV_DIR.resolve():
        os.execv(str(VENV_PYTHON), [str(VENV_PYTHON), *sys.argv])


def charts(results: dict, scenarios: list[str]) -> list[str]:
    """Draws the bar charts of the main metrics; returns their Markdown."""
    import matplotlib  # pylint: disable=import-outside-toplevel
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt  # pylint: disable=import-outside-toplevel

    IMAGES_DIR.mkdir(parents=True, exist_ok=True)
    metrics = (
        ("delay", "Median input-to-display delay (ms)", "%.0f",
         lambda r: r.delay_ms(0.5) or 0.0),
        ("still", "Frames where an entity is frozen (%)", "%.1f", lambda r: 100 * r.still_ratio),
        ("jump", "99th percentile of the move per frame (units)", "%.1f", lambda r: r.jump),
    )
    lines = []
    width = 0.8 / len(MODES)
    for name, label, value_format, value in metrics:
        figure, axes = plt.subplots(figsize=(9, 4))
        for index, mode in enumerate(MODES):
            values = [value(results[(s, mode)]) if (s, mode) in results else 0.0
                      for s in scenarios]
            positions = [i + (index - (len(MODES) - 1) / 2) * width for i in range(len(scenarios))]
            bars = axes.bar(positions, values, width, label=MODES[mode][0])
            axes.bar_label(bars, fmt=value_format, fontsize=7, padding=2)
        axes.set_xticks(range(len(scenarios)))
        axes.set_xticklabels([SCENARIOS[s][0] for s in scenarios], rotation=20, ha="right")
        axes.set_ylabel(label)
        axes.set_ylim(top=axes.get_ylim()[1] * 1.25)
        axes.legend(loc="upper center", ncol=len(MODES))
        axes.grid(axis="y", alpha=0.3)
        figure.tight_layout()
        figure.savefig(IMAGES_DIR / f"netbench-{name}.png", dpi=120)
        plt.close(figure)
        lines += [f"![{label}](images/network-comparison/netbench-{name}.png)", ""]
    return lines


def report(with_charts: bool) -> None:
    results = {}
    for scenario in SCENARIOS:
        for mode in MODES:
            path = RESULTS_DIR / f"{scenario}-{mode}.csv"
            if path.is_file():
                results[(scenario, mode)] = load(path)
                results[(scenario, mode)].predicted = "--no-prediction" not in MODES[mode][1]
    if not results:
        raise BenchError(f"no measures in {RESULTS_DIR}: run `./tools/netbench.py run` first")
    scenarios = [s for s in SCENARIOS if any((s, m) in results for m in MODES)]

    any_result = next(iter(results.values()))
    lines = [BEGIN_MARKER, "",
             f"Generated by `tools/netbench.py` on {time.strftime('%Y-%m-%d')}: "
             f"{any_result.duration:.0f} s per run, client at about "
             f"{statistics.median(r.fps for r in results.values()):.0f} frames per second.", ""]
    lines += table(results, scenarios, "Input-to-display delay of the local ship, median "
                   "(95th percentile), in ms",
                   lambda r: f"{fmt(r.delay_ms(0.5))} ({fmt(r.delay_ms(0.95))})"
                   + (f", {r.missed} missed" if r.missed else ""))
    lines += table(results, scenarios, "Frames where a synced entity is frozen on screen",
                   lambda r: fmt(100 * r.still_ratio, 1, " %"))
    lines += table(results, scenarios, "99th percentile of the move of an entity in one "
                   "frame, in playfield units", lambda r: fmt(r.jump, 1))
    lines += table(results, scenarios, "Corrections of the predicted ship per second, and "
                   "their median size in playfield units",
                   lambda r: "n/a" if not r.predicted else "0" if not r.corrections else
                   f"{len(r.corrections) / r.duration:.1f} ({statistics.median(r.corrections):.1f})")
    lines += table(results, scenarios, "Traffic of the client, received / sent, in bytes "
                   "per second", lambda r: f"{r.received:.0f} / {r.sent:.0f}")
    if with_charts:
        lines += charts(results, scenarios)
    lines.append(END_MARKER)

    text = DOC.read_text(encoding="utf-8")
    begin, end = text.find(BEGIN_MARKER), text.find(END_MARKER)
    if begin < 0 or end < begin:
        raise BenchError(f"{DOC.name} has no {BEGIN_MARKER} ... {END_MARKER} block")
    DOC.write_text(text[:begin] + "\n".join(lines) + text[end + len(END_MARKER):],
                   encoding="utf-8")
    log(f"Results written to {DOC.relative_to(ROOT)}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Network benchmark of the R-Type client.")
    parser.add_argument("command", nargs="?", choices=("run", "report"),
                        help="run the measures, or write the report (default: both)")
    parser.add_argument("-d", "--duration", type=float, default=30.0,
                        help="seconds measured per run (default: 30)")
    parser.add_argument("-s", "--scenarios", nargs="+", choices=list(SCENARIOS),
                        default=list(SCENARIOS), help="network scenarios (default: all)")
    parser.add_argument("-m", "--modes", nargs="+", choices=list(MODES),
                        default=list(MODES), help="client modes (default: all)")
    parser.add_argument("--no-netem", action="store_true",
                        help="do not touch tc: measure the current network as is")
    parser.add_argument("--no-charts", action="store_true",
                        help="write the tables only, without matplotlib")
    args = parser.parse_args()
    try:
        if args.command in (None, "report") and not args.no_charts:
            ensure_matplotlib()
        if args.command in (None, "run"):
            if args.no_netem:
                args.scenarios = ["normal"]
            elif not sys.platform.startswith("linux"):
                raise BenchError("tc netem only exists on Linux (or use --no-netem)")
            run(args.scenarios, args.modes, args.duration, not args.no_netem)
        if args.command in (None, "report"):
            report(not args.no_charts)
    except BenchError as error:
        print(f"netbench: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130
    return 0


if __name__ == "__main__":
    sys.exit(main())
