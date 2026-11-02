#!/usr/bin/env python3
"""R-Type launcher: build, run, test and lint the project with one command.

Usage: `./rtype.py [options]` on Linux and macOS, `python rtype.py [options]`
on Windows. Run it with `--help` for the options.

Without options, it builds the client, starts the server (in Docker, or
natively with `--native`), starts the clients and waits for them. The other
modes (`--test`, `--tidy`, `--format`, `--docs`...) run one developer tool and
exit.

Before building, it installs what is missing without admin rights: cmake,
conan and gcovr go into a local virtual environment (TOOLS_DIR, `.venv/`), and
Conan installs the libraries. Only the standard library of Python 3.8+ is used,
and no command goes through a shell.

Environment variables override the settings below. Relative paths are relative
to the repository root, whatever the current directory.
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import signal
import subprocess
import sys
import urllib.request
from pathlib import Path

# ---------- Settings (environment variables override them) ----------
ROOT = Path(__file__).resolve().parent

BUILD_DIR = Path(os.environ.get("BUILD_DIR", "build"))
COVERAGE_DIR = Path(os.environ.get("COVERAGE_DIR", "build-coverage"))
SERVER_BIN = os.environ.get("SERVER_BIN")
CLIENT_BIN = os.environ.get("CLIENT_BIN")
IMAGE = os.environ.get("IMAGE", "rtype-server")
CONTAINER = os.environ.get("CONTAINER", "rtype-server")
# What Conan does with missing system packages (X11/OpenGL headers for the
# graphics library): install (with sudo), check (fail if missing) or report
# (only list them).
CONAN_SYSTEM_PACKAGES = os.environ.get("CONAN_SYSTEM_PACKAGES", "install")
# Graphics library of the client (also --graphics).
GRAPHICS_CHOICES = ("raylib", "sfml", "ncurses")
GRAPHICS = os.environ.get("GRAPHICS", "raylib")
# Virtual environment where missing build tools (cmake, conan...) are installed.
TOOLS_DIR = Path(os.environ.get("TOOLS_DIR", ".venv"))
# Every clang-format version formats a little differently: everyone (and the
# CI) uses this one, installed from PyPI into TOOLS_DIR.
CLANG_FORMAT_VERSION = "18.1.3"
# Folders whose C++ sources are formatted and checked.
FORMAT_FOLDERS = ("engine", "common", "server", "client", "tests")
# Versioned Git hooks (pre-commit: formatting check), enabled by this script.
HOOKS_DIR = ".githooks"

BUILD_TYPE = "Release"
JOBS = str(os.cpu_count() or 1)

# ---------- Operating system ----------
# Everything that differs between OSes is decided here or in the functions
# that test these flags; the rest of the script is the same everywhere.
SYSTEM = platform.system()  # "Linux", "Darwin" (macOS) or "Windows"
IS_WINDOWS = SYSTEM == "Windows"
IS_MACOS = SYSTEM == "Darwin"
EXE = ".exe" if IS_WINDOWS else ""
# How to start this script, for the messages that suggest a command.
LAUNCHER = "python rtype.py" if IS_WINDOWS else "./rtype.py"
TOOLS_BIN = (ROOT / TOOLS_DIR) / ("Scripts" if IS_WINDOWS else "bin")
PIP_PYZ_URL = "https://bootstrap.pypa.io/pip/pip.pyz"


# ---------- Output and errors ----------
class LauncherError(Exception):
    """A fatal error: main() prints its message and exits with status 1."""


if IS_WINDOWS:
    os.system("")  # enables ANSI colors in the legacy Windows console
COLORS = sys.stdout.isatty() and "NO_COLOR" not in os.environ


def log(message: str) -> None:
    """Prints a progress message on stdout."""
    prefix = "\033[1;34m[run]\033[0m" if COLORS else "[run]"
    print(f"{prefix} {message}", flush=True)


def print_error(message: str) -> None:
    """Prints an error message on stderr."""
    prefix = "\033[1;31m[error]\033[0m" if COLORS else "[error]"
    print(f"{prefix} {message}", file=sys.stderr, flush=True)


# ---------- Running commands ----------
def call(*args: str | Path, **kwargs) -> subprocess.CompletedProcess:
    """Runs a command (no shell) and returns its result, whatever its exit
    status. Keyword arguments go to subprocess.run. A program that cannot be
    started raises LauncherError instead of a traceback."""
    try:
        return subprocess.run([str(arg) for arg in args], **kwargs)
    except OSError as error:
        raise LauncherError(f"could not run {args[0]}: {error}") from error


def run(*args: str | Path, error: str, quiet: bool = False) -> None:
    """Runs a command and raises LauncherError(error) if it fails. A quiet
    command prints nothing, except the end of its output when it fails."""
    if not quiet:
        if call(*args).returncode != 0:
            raise LauncherError(error)
        return
    result = call(*args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                  text=True, errors="replace")
    if result.returncode != 0:
        print("\n".join(result.stdout.splitlines()[-30:]), file=sys.stderr)
        raise LauncherError(error)


def succeeds(*args: str | Path) -> bool:
    """True if the command runs and exits with status 0. Prints nothing."""
    try:
        return subprocess.run([str(arg) for arg in args], capture_output=True).returncode == 0
    except OSError:
        return False


def output_of(*args: str | Path) -> str:
    """The standard output of a command, stripped ("" if it cannot run)."""
    try:
        return subprocess.run([str(arg) for arg in args], capture_output=True,
                              text=True, errors="replace").stdout.strip()
    except OSError:
        return ""


def install_hint(windows: str, macos: str, linux: str) -> str:
    """The way to install something on this OS."""
    return windows if IS_WINDOWS else macos if IS_MACOS else linux


def require(tool: str, hint: str = "") -> str:
    """Returns the path of `tool`, or raises LauncherError if it is not in the
    PATH."""
    path = shutil.which(tool)
    if path is None:
        raise LauncherError(f"{tool} is not installed" + (f": {hint}" if hint else ""))
    return path


# ---------- Build tools (.venv/, compiler) ----------
def use_tools_dir() -> None:
    """Makes the tools installed in TOOLS_DIR visible to this script and to the
    commands it runs. System tools come first."""
    path = os.environ.get("PATH", "")
    if TOOLS_BIN.is_dir() and str(TOOLS_BIN) not in path.split(os.pathsep):
        os.environ["PATH"] = path + os.pathsep + str(TOOLS_BIN)


def has_pip(python: Path) -> bool:
    """True if this Python interpreter has pip."""
    return succeeds(python, "-m", "pip", "--version")


def ensure_tools_env() -> Path:
    """Creates TOOLS_DIR if needed and makes sure it has pip (also repairs an
    environment left without pip). Returns its Python interpreter."""
    python = TOOLS_BIN / f"python{EXE}"
    if not python.is_file():
        log(f"Creating the tools environment in {TOOLS_DIR}...")
        # Without pip: Debian/Ubuntu strip ensurepip unless python3-venv is
        # installed, and venv fails halfway when it is missing.
        run(sys.executable, "-m", "venv", "--without-pip", ROOT / TOOLS_DIR,
            error="Could not create a Python virtual environment")
        use_tools_dir()
    if has_pip(python):
        return python
    log(f"Installing pip in {TOOLS_DIR}...")
    if not succeeds(python, "-m", "ensurepip", "--default-pip"):
        # No ensurepip: use the official standalone pip to install pip.
        pip_pyz = ROOT / TOOLS_DIR / "pip.pyz"
        try:
            urllib.request.urlretrieve(PIP_PYZ_URL, pip_pyz)
        except OSError as error:
            raise LauncherError(f"Could not download {PIP_PYZ_URL}: {error}") from error
        try:
            run(python, pip_pyz, "install", "--quiet", "--disable-pip-version-check", "pip",
                error="Could not install pip in the tools environment")
        finally:
            pip_pyz.unlink(missing_ok=True)
    if not has_pip(python):
        raise LauncherError(f"pip still not found in {TOOLS_DIR}")
    return python


def pip_install(requirement: str) -> None:
    """Installs a PyPI package into TOOLS_DIR."""
    python = ensure_tools_env()
    log(f"Installing {requirement} in {TOOLS_DIR}...")
    run(python, "-m", "pip", "install", "--quiet", "--disable-pip-version-check", requirement,
        error=f"pip install {requirement} failed")


def ensure_tool(tool: str) -> None:
    """Installs a missing Python-packaged tool (cmake, conan, gcovr) from PyPI
    into TOOLS_DIR: no pipx, no admin rights, same on every OS."""
    if shutil.which(tool):
        return
    log(f"{tool} not found.")
    pip_install(tool)
    if shutil.which(tool) is None:
        raise LauncherError(f"{tool} still not found after installing it in {TOOLS_DIR}")


def has_msvc() -> bool:
    """Windows: true if vswhere finds a Visual Studio with the C++ tools."""
    vswhere = (Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
               / "Microsoft Visual Studio" / "Installer" / "vswhere.exe")
    return vswhere.is_file() and bool(output_of(
        vswhere, "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"))


def ensure_compiler() -> None:
    """Makes sure a C++ compiler is installed. On Windows, installs MSVC
    (Visual Studio Build Tools) with winget if it is missing. On Linux and
    macOS, explains how to install one: it needs admin rights."""
    if not IS_WINDOWS:
        if IS_MACOS and not succeeds("xcode-select", "-p"):
            raise LauncherError("no C++ compiler: install the Xcode Command Line Tools "
                                "with `xcode-select --install`, then run this again")
        if not any(shutil.which(cxx) for cxx in (os.environ.get("CXX", ""), "c++",
                                                  "g++", "clang++") if cxx):
            raise LauncherError(
                "no C++ compiler found: install GCC or Clang with your package manager "
                "(e.g. `sudo apt install build-essential`, `sudo dnf install gcc-c++` "
                "or `sudo pacman -S base-devel`)")
        return
    if shutil.which("cl") or has_msvc():
        return
    require("winget", 'install Visual Studio Build Tools with the "Desktop development '
                      'with C++" workload')
    log("No C++ compiler found, installing Visual Studio Build Tools "
        "(several GB, asks for admin rights)...")
    # Its exit code is not reliable (e.g. already installed without the C++
    # workload): check the result instead.
    call("winget", "install", "--id", "Microsoft.VisualStudio.2022.BuildTools",
         "--exact", "--accept-package-agreements", "--accept-source-agreements",
         "--override", "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools "
         "--includeRecommended")
    if not has_msvc():
        raise LauncherError('MSVC still not found: add the "Desktop development with C++" '
                            "workload in the Visual Studio Installer")


# ---------- Configure and build (Conan + CMake) ----------
def conan_install(build_dir: Path) -> None:
    """Installs the Conan packages and the CMake toolchain into build_dir.
    Conan's output goes to build_dir/conan-install.log."""
    # The compiler comes first: pip builds Conan's dependencies (e.g. PyYAML)
    # from source when PyPI has no prebuilt wheel for this Python version.
    ensure_compiler()
    ensure_tool("conan")
    # A fresh Conan install has no profile: detect one from the local compiler.
    if not succeeds("conan", "profile", "path", "default"):
        log("No default Conan profile, detecting one...")
        run("conan", "profile", "detect", quiet=True, error="conan profile detect failed")

    # Only Linux has system packages to install (xorg/system, opengl/system):
    # macOS and Windows ship OpenGL and their window system. Conan asks for the
    # sudo password only if one of them is missing.
    system_packages = [] if IS_WINDOWS or IS_MACOS else [
        "-c", f"tools.system.package_manager:mode={CONAN_SYSTEM_PACKAGES}",
        "-c", "tools.system.package_manager:sudo=True"]

    log("Installing the Conan packages...")
    build_dir.mkdir(parents=True, exist_ok=True)
    conan_log = build_dir / "conan-install.log"
    with conan_log.open("w") as output:
        status = call(
            "conan", "install", ".", f"--output-folder={build_dir}", "--build=missing",
            "-s", "compiler.cppstd=20", "-s", f"build_type={BUILD_TYPE}",
            "-o", f"&:graphics={GRAPHICS}", *system_packages,
            stdout=output, stderr=subprocess.STDOUT).returncode
    if status != 0:
        lines = conan_log.read_text(errors="replace").splitlines()
        print("\n".join(lines[-30:]), file=sys.stderr)
        raise LauncherError(f"conan install failed (full output: {conan_log})")


def configure(*cmake_args: str, build_dir: Path = BUILD_DIR) -> None:
    """Installs the Conan packages into build_dir, then configures it with the
    Conan toolchain. Extra arguments are passed to CMake."""
    ensure_tool("cmake")
    conan_install(build_dir)
    # CMake only reads the toolchain on a build folder's first configure: a
    # folder first configured without it must be configured from scratch.
    fresh = []
    if (build_dir / "CMakeCache.txt").is_file() and not any(
            "conan_toolchain.cmake" in path.read_text(errors="replace")
            for path in build_dir.glob("CMakeFiles/*/CMakeSystem.cmake")):
        log(f"{build_dir} was configured without Conan, reconfiguring it from scratch...")
        fresh = ["--fresh"]
    run("cmake", *fresh, "-B", build_dir, "-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake",
        f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        # The toolchain only sets it on the first configure: force it here.
        f"-DRTYPE_GRAPHICS_BACKEND={GRAPHICS}", *cmake_args,
        quiet=True, error="CMake configuration failed")


def build(target: str, build_dir: Path = BUILD_DIR) -> None:
    """Builds one CMake target of build_dir."""
    ensure_tool("cmake")
    run("cmake", "--build", build_dir, "--target", target, "--config", BUILD_TYPE,
        "--parallel", JOBS, error=f"{target} build failed")


def find_binary(name: str, override: str | None) -> Path | None:
    """The path of a built program, None if it is not built. Single-config
    generators put binaries in the build folder, multi-config ones (Visual
    Studio) in a per-configuration subfolder."""
    if override:
        candidates = [Path(override)]
    else:
        candidates = [BUILD_DIR / f"{name}{EXE}", BUILD_DIR / BUILD_TYPE / f"{name}{EXE}"]
    return next((path for path in candidates if path.is_file()), None)


def conan_outdated(build_dir: Path = BUILD_DIR) -> bool:
    """True if conanfile.py is newer than the last Conan install: a build
    would then miss the newly required packages."""
    conan_log = build_dir / "conan-install.log"
    return (not conan_log.is_file()
            or (ROOT / "conanfile.py").stat().st_mtime > conan_log.stat().st_mtime)


def configured_graphics(build_dir: Path = BUILD_DIR) -> str | None:
    """The graphics library build_dir was configured with, None if unknown."""
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        return None
    for line in cache.read_text(errors="replace").splitlines():
        if line.startswith("RTYPE_GRAPHICS_BACKEND:"):
            return line.partition("=")[2]
    return None


def ensure_binary(name: str, override: str | None, *cmake_args: str) -> Path:
    """Returns the program, built from the current sources. A binary given by
    the user (SERVER_BIN, CLIENT_BIN) is used as is."""
    binary = find_binary(name, override)
    if override:
        if binary is None:
            raise LauncherError(f"{override} does not exist (set by SERVER_BIN or CLIENT_BIN)")
        return binary
    if binary is None:
        log(f"{name} not found, configuring the build...")
        configure(*cmake_args)
    elif conan_outdated():
        log("conanfile.py changed since the last Conan install, configuring the build...")
        configure(*cmake_args)
    elif configured_graphics() != GRAPHICS:
        log(f"Switching the graphics library to {GRAPHICS}, configuring the build...")
        configure(*cmake_args)
    # Always build: it is almost instant when nothing changed, and a stale
    # binary would silently ignore the latest changes.
    log(f"Building {name}...")
    build(name)
    binary = find_binary(name, override)
    if binary is None:
        raise LauncherError(f"{name} still not found after building it")
    return binary


# ---------- Game: server and clients ----------
def check_docker() -> None:
    """Raises LauncherError with the fix if Docker is missing, stopped, or not
    usable by this user."""
    require("docker", install_hint("install Docker Desktop", "install Docker Desktop",
                                   "install Docker Engine") + ", or use --native")
    info = call("docker", "info", capture_output=True, text=True, errors="replace")
    if info.returncode == 0:
        return
    if "permission denied" in info.stderr.lower():
        raise LauncherError("your user cannot use Docker: add it to the docker group "
                            "(`sudo usermod -aG docker $USER`, then log in again), "
                            "or use --native")
    raise LauncherError("Docker is installed but not running: start " + install_hint(
        "Docker Desktop", "Docker Desktop", "it (`sudo systemctl start docker`)")
        + ", or use --native")


def start_docker_server(port: int) -> None:
    """Builds the server image and starts it in a detached container."""
    check_docker()
    log("Building server image...")
    run("docker", "build", "-t", IMAGE, ".", error="Docker build failed")
    succeeds("docker", "rm", "-f", CONTAINER)
    log(f"Starting server in Docker on port {port}/udp...")
    run("docker", "run", "-d", "--rm", "--init", "--name", CONTAINER,
        "-p", f"{port}:4242/udp", IMAGE, quiet=True,
        error="Could not start the server container")


def start_native_server(port: int, in_terminal: bool) -> subprocess.Popen:
    """Builds and starts the server on this machine. When the client draws in
    this terminal, the server's output goes to BUILD_DIR/server.log."""
    server_bin = ensure_binary("r-type_server", SERVER_BIN)
    log(f"Starting native server on port {port}...")
    if not in_terminal:
        return subprocess.Popen([str(server_bin), str(port)])
    server_log = BUILD_DIR / "server.log"
    log(f"Server output: {server_log}")
    with server_log.open("w") as output:
        return subprocess.Popen([str(server_bin), str(port)],
                                stdout=output, stderr=subprocess.STDOUT)


def stop(process: subprocess.Popen) -> None:
    """Stops a process: politely first, then by force after 5 seconds."""
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()


def play(args: argparse.Namespace) -> int:
    """Default mode: starts the server and the clients, waits until every
    client is closed, then stops everything (also on Ctrl+C or an error)."""
    server: subprocess.Popen | None = None
    clients: list[subprocess.Popen] = []
    container_started = False
    # The ncurses client draws in this terminal: nothing else may write to it.
    in_terminal = GRAPHICS == "ncurses"
    if in_terminal and args.clients > 1:
        raise LauncherError("the ncurses client uses this terminal: start one client "
                            "here (-c 1) and the others from other terminals")
    try:
        client_bin = ensure_binary("r-type_client", CLIENT_BIN, "-DBUILD_CLIENT=ON")
        if args.native:
            server = start_native_server(args.port, in_terminal)
        else:
            start_docker_server(args.port)
            container_started = True

        for i in range(1, args.clients + 1):
            log(f"Starting client {i}/{args.clients} -> {args.host}:{args.port}")
            clients.append(subprocess.Popen([str(client_bin), args.host, str(args.port)]))

        if in_terminal:
            log("Running. Press q in the game to stop (its logs: r-type_client.log).")
        else:
            log("Running. Close all clients or press Ctrl+C to stop.")
        for client in clients:
            client.wait()
        return 0
    finally:
        if clients or server is not None or container_started:
            log("Shutting down...")
        for client in clients:
            stop(client)
        if server is not None:
            stop(server)
        if container_started:
            succeeds("docker", "stop", "-t", "5", CONTAINER)


# ---------- Unit tests (GoogleTest + CTest) ----------
def run_tests() -> int:
    """Builds the unit tests in BUILD_DIR and runs them with CTest."""
    configure("-DBUILD_TESTS=ON")
    log("Building the unit tests...")
    build("r-type_tests")
    log("Running the unit tests...")
    return call("ctest", "--test-dir", BUILD_DIR, "-C", BUILD_TYPE,
                "--output-on-failure").returncode


def run_coverage() -> int:
    """Runs the unit tests with coverage and writes an HTML report. Uses its
    own build directory: instrumented binaries must not mix with the normal
    build."""
    if IS_WINDOWS:
        raise LauncherError("--coverage needs GCC or Clang (gcov): it does not work with MSVC")
    ensure_tool("gcovr")
    log("Building the unit tests with coverage...")
    configure("-DBUILD_TESTS=ON", "-DENABLE_COVERAGE=ON", "-DBUILD_CLIENT=OFF",
              build_dir=COVERAGE_DIR)
    build("r-type_tests", COVERAGE_DIR)
    # Counters add up across runs: start from zero.
    for counters in COVERAGE_DIR.rglob("*.gcda"):
        counters.unlink()
    log("Running the unit tests...")
    run("ctest", "--test-dir", COVERAGE_DIR, "--output-on-failure", error="Some tests failed")
    report = COVERAGE_DIR / "coverage" / "index.html"
    report.parent.mkdir(parents=True, exist_ok=True)
    run("gcovr", "--root", ".", "--object-directory", COVERAGE_DIR,
        "--filter", "engine/", "--filter", "common/",
        "--html-details", report, "--print-summary", error="gcovr failed")
    log(f"HTML report: {report}")
    return 0


# ---------- Static analysis (clang-tidy) ----------
# Where LLVM installs its tools without adding them to the PATH: Homebrew's
# llvm is "keg-only", and the Windows installer may skip the PATH.
LLVM_DIRS = (Path("/opt/homebrew/opt/llvm/bin"), Path("/usr/local/opt/llvm/bin"),
             Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "LLVM" / "bin")


def find_llvm_tool(name: str) -> str | None:
    """The path of an LLVM tool (clang-tidy...): in the PATH, or in LLVM_DIRS."""
    found = shutil.which(name)
    if found is None:
        found = next((str(path) for path in (folder / f"{name}{EXE}" for folder in LLVM_DIRS)
                      if path.is_file()), None)
    return found


def run_tidy(fix: bool = False) -> int:
    """Runs clang-tidy on every file of compile_commands.json (configuring
    BUILD_DIR first if it has none). With fix, applies its fixes, then
    clang-format."""
    clang_tidy = find_llvm_tool("clang-tidy")
    if clang_tidy is None:
        raise LauncherError("clang-tidy is not installed: " + install_hint(
            "winget install LLVM.LLVM", "brew install llvm",
            "install clang-tidy with your package manager"))
    compile_commands = BUILD_DIR / "compile_commands.json"
    if not compile_commands.is_file():
        log(f"Generating {compile_commands}...")
        configure()
    if not compile_commands.is_file():  # Visual Studio generators write none
        raise LauncherError(f"CMake wrote no {compile_commands}: configure {BUILD_DIR} with "
                            "Ninja or Makefiles (`cmake -G Ninja ...`) to run clang-tidy")

    run_clang_tidy = find_llvm_tool("run-clang-tidy")
    if run_clang_tidy:
        log("Running clang-tidy on every file of the project...")
        status = call(run_clang_tidy, "-p", BUILD_DIR, "-quiet",
                      f"-clang-tidy-binary={clang_tidy}", *(["-fix"] if fix else [])).returncode
    else:
        log("run-clang-tidy not found, checking files one by one...")
        files = sorted({entry["file"] for entry in json.loads(compile_commands.read_text())})
        status = 0
        for file in files:
            if call(clang_tidy, "-p", BUILD_DIR, "--quiet",
                    *(["--fix"] if fix else []), file).returncode != 0:
                status = 1

    if fix:
        run_format()
    return status


# ---------- Formatting (clang-format) ----------
def ensure_clang_format() -> Path:
    """Returns the pinned clang-format of TOOLS_DIR, installing it if it is
    missing or has another version. The system one is never used."""
    clang_format = TOOLS_BIN / f"clang-format{EXE}"
    if clang_format.is_file() and f"version {CLANG_FORMAT_VERSION}" in output_of(
            clang_format, "--version"):
        return clang_format
    pip_install(f"clang-format=={CLANG_FORMAT_VERSION}")
    return clang_format


def format_files(paths: list[str] | None = None) -> list[Path]:
    """The project's C++ sources, or the given paths that are among them
    (paths relative to the repository root, as Git prints them)."""
    if paths is not None:
        return sorted(Path(path) for path in paths
                      if Path(path).suffix in (".cpp", ".hpp")
                      and Path(path).parts[0] in FORMAT_FOLDERS and Path(path).is_file())
    return sorted(path for folder in FORMAT_FOLDERS
                  for pattern in ("*.cpp", "*.hpp") for path in Path(folder).rglob(pattern))


def run_format() -> int:
    """Formats every C++ source of the project in place."""
    clang_format = ensure_clang_format()
    log("Running clang-format on every file of the project...")
    run(clang_format, "-i", *format_files(), error="clang-format failed")
    return 0


def check_format(paths: list[str]) -> int:
    """Same check as the CI (all the sources when no path is given): returns
    1 if a file is badly formatted, without changing it."""
    files = format_files(paths or None)
    if not files:
        return 0
    clang_format = ensure_clang_format()
    if call(clang_format, "--dry-run", "--Werror", *files).returncode != 0:
        print_error(f"Badly formatted files: run {LAUNCHER} --format, then git add them again")
        return 1
    return 0


def install_hooks() -> None:
    """Points Git to the versioned hooks, unless the clone already uses its
    own hooks folder."""
    if not Path(".git").exists() or output_of("git", "config", "--local", "core.hooksPath"):
        return
    if succeeds("git", "config", "--local", "core.hooksPath", HOOKS_DIR):
        log(f"Git hooks enabled ({HOOKS_DIR}/): commits are checked by clang-format")


# ---------- Documentation (Doxygen) ----------
def run_docs() -> int:
    """Generates the HTML documentation in BUILD_DIR/docs. Settings come from
    the Doxyfile; the diagrams are turned off when Graphviz is missing instead
    of failing. Warnings go to a log file, not to the terminal."""
    require("doxygen", install_hint("winget install DimitriVanHeesch.Doxygen",
                                    "brew install doxygen",
                                    "install doxygen with your package manager"))
    output = (BUILD_DIR / "docs").resolve()
    output.mkdir(parents=True, exist_ok=True)
    warnings = output / "doxygen-warnings.log"
    config = Path("Doxyfile").read_text(encoding="utf-8")
    config += (f'\nOUTPUT_DIRECTORY = "{output.as_posix()}"\n'
               f'WARN_LOGFILE = "{warnings.as_posix()}"\nQUIET = YES\n')
    if shutil.which("dot") is None:
        log("Graphviz (dot) not found, generating the documentation without diagrams...")
        config += "HAVE_DOT = NO\n"
    log("Generating the documentation...")
    if call("doxygen", "-", input=config, text=True).returncode != 0:
        raise LauncherError(f"doxygen failed (warnings and errors: {warnings})")
    count = warnings.read_text(errors="replace").count("warning:") if warnings.is_file() else 0
    if count:
        log(f"{count} Doxygen warnings, see {warnings}")
    log(f"HTML documentation: {output / 'html' / 'index.html'}")
    return 0


# ---------- Command line ----------
def positive(value: str) -> int:
    """argparse type: an integer >= 1."""
    if not value.isdigit() or int(value) < 1:
        raise argparse.ArgumentTypeError("must be a positive number")
    return int(value)


def port(value: str) -> int:
    """argparse type: a port number."""
    if not value.isdigit() or not 1 <= int(value) <= 65535:
        raise argparse.ArgumentTypeError("must be between 1 and 65535")
    return int(value)


EPILOG = """\
examples:
  ./rtype.py                    server in Docker + 1 client
  ./rtype.py --native -c 2      server built and run here + 2 clients
  ./rtype.py -g sfml            client built with SFML
  ./rtype.py --test             build and run the unit tests
  ./rtype.py --check-format     check the formatting like the CI
  (on Windows: python rtype.py ...)

environment variables:
  BUILD_DIR (build), COVERAGE_DIR (build-coverage), SERVER_BIN, CLIENT_BIN,
  IMAGE and CONTAINER (rtype-server), GRAPHICS (raylib),
  CONAN_SYSTEM_PACKAGES (install | check | report), TOOLS_DIR (.venv), NO_COLOR

See docs/INSTALLATION.md for the requirements of each OS."""


def parse_args() -> argparse.Namespace:
    """Parses the command line. At most one developer tool can be chosen."""
    parser = argparse.ArgumentParser(
        prog="rtype.py", epilog=EPILOG, formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Builds and starts the R-Type server (in Docker by default, or\n"
                    "natively) and its clients, or runs one developer tool.")
    game = parser.add_argument_group("game (default mode)")
    game.add_argument("-n", "--native", action="store_true",
                      help="run the server natively instead of in Docker")
    game.add_argument("-c", "--clients", type=positive, default=1, metavar="N",
                      help="number of clients to launch (default: 1)")
    game.add_argument("-p", "--port", type=port, default=4242,
                      help="UDP port of the server (default: 4242)")
    game.add_argument("-H", "--host", default="127.0.0.1",
                      help="address the clients connect to (default: 127.0.0.1)")
    game.add_argument("-g", "--graphics", choices=GRAPHICS_CHOICES, default=GRAPHICS,
                      help=f"graphics library of the client (default: {GRAPHICS})")
    tools = parser.add_argument_group("developer tools (run one, then exit)")
    mode = tools.add_mutually_exclusive_group()
    mode.add_argument("-T", "--test", dest="action", action="store_const", const="test",
                      help="build and run the unit tests (GoogleTest)")
    mode.add_argument("-C", "--coverage", dest="action", action="store_const",
                      const="coverage", help="run the unit tests with coverage, write an "
                                             "HTML report")
    mode.add_argument("-t", "--tidy", dest="action", action="store_const", const="tidy",
                      help="run clang-tidy on the project")
    mode.add_argument("-f", "--fix", dest="action", action="store_const", const="fix",
                      help="apply the clang-tidy fixes, then run clang-format")
    mode.add_argument("-F", "--format", dest="action", action="store_const", const="format",
                      help=f"run clang-format {CLANG_FORMAT_VERSION} (the CI's version) on "
                           "the project")
    mode.add_argument("--check-format", nargs="*", metavar="FILE",
                      help="check the formatting like the CI (all the sources, or only "
                           "FILE...); used by the pre-commit hook")
    mode.add_argument("-d", "--docs", dest="action", action="store_const", const="docs",
                      help="generate the HTML documentation with Doxygen")
    return parser.parse_args()


ACTIONS = {
    "test": run_tests,
    "coverage": run_coverage,
    "tidy": run_tidy,
    "fix": lambda: run_tidy(fix=True),
    "format": run_format,
    "docs": run_docs,
}


def main() -> int:
    """Entry point: returns the exit status of the chosen mode."""
    global GRAPHICS
    if sys.version_info < (3, 8):
        print_error("Python 3.8 or newer is required")
        return 2
    args = parse_args()
    GRAPHICS = args.graphics
    if GRAPHICS not in GRAPHICS_CHOICES:  # argparse does not check the default
        print_error(f"GRAPHICS must be one of: {', '.join(GRAPHICS_CHOICES)}")
        return 2
    if GRAPHICS == "ncurses" and IS_WINDOWS:
        print_error("the ncurses client does not run on Windows: use raylib or sfml")
        return 2
    os.chdir(ROOT)
    use_tools_dir()
    # Turns `docker stop`/`kill` of the launcher into a normal exit, so the
    # server and the clients are stopped too.
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(128 + signal.SIGTERM))
    try:
        install_hooks()
        if args.check_format is not None:
            return check_format(args.check_format)
        if args.action:
            return ACTIONS[args.action]()
        return play(args)
    except LauncherError as error:
        print_error(str(error))
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
