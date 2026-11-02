#!/usr/bin/env python3
"""R-Type launcher.

Starts the server (in Docker by default, or natively) and one or more clients.
Runs on Linux and Windows: `./rtype.py [options]` or `python rtype.py [options]`.

Environment variables override the defaults below. Relative paths are relative
to the repository root, whatever the current directory.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import signal
import subprocess
import sys
import urllib.request
from pathlib import Path

# ---------- Defaults (can be overridden by environment variables) ----------
ROOT = Path(__file__).resolve().parent
os.chdir(ROOT)

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
# Graphics library of the client (also --graphics): raylib or sfml.
GRAPHICS = os.environ.get("GRAPHICS", "raylib")
GRAPHICS_CHOICES = ("raylib", "sfml")
# Virtual environment where missing build tools (cmake, conan...) are installed.
TOOLS_DIR = Path(os.environ.get("TOOLS_DIR", ".venv"))

BUILD_TYPE = "Release"
JOBS = str(os.cpu_count() or 1)
EXE = ".exe" if os.name == "nt" else ""
TOOLS_BIN = TOOLS_DIR.resolve() / ("Scripts" if os.name == "nt" else "bin")
PIP_PYZ_URL = "https://bootstrap.pypa.io/pip/pip.pyz"


# ---------- Helpers ----------
class LauncherError(Exception):
    pass


if os.name == "nt":
    os.system("")  # enables ANSI colors in the legacy Windows console
COLORS = sys.stdout.isatty() and "NO_COLOR" not in os.environ


def log(message: str) -> None:
    prefix = "\033[1;34m[run]\033[0m" if COLORS else "[run]"
    print(f"{prefix} {message}", flush=True)


def print_error(message: str) -> None:
    prefix = "\033[1;31m[error]\033[0m" if COLORS else "[error]"
    print(f"{prefix} {message}", file=sys.stderr, flush=True)


def require(tool: str, hint: str = "") -> None:
    if shutil.which(tool) is None:
        raise LauncherError(f"{tool} is not installed" + (f" ({hint})" if hint else ""))


def run(*args: str | Path, error: str, quiet: bool = False) -> None:
    """Runs a command and raises `error` if it fails."""
    output = subprocess.DEVNULL if quiet else None
    if subprocess.run([str(a) for a in args], stdout=output).returncode != 0:
        raise LauncherError(error)


def use_tools_dir() -> None:
    """Makes the tools installed in TOOLS_DIR visible to this script and to the
    commands it runs. System tools come first."""
    if TOOLS_BIN.is_dir() and str(TOOLS_BIN) not in os.environ.get("PATH", ""):
        os.environ["PATH"] = os.environ.get("PATH", "") + os.pathsep + str(TOOLS_BIN)


def has_pip(python: Path) -> bool:
    return subprocess.run([str(python), "-m", "pip", "--version"],
                          capture_output=True).returncode == 0


def ensure_tools_env() -> Path:
    """Creates TOOLS_DIR if needed and makes sure it has pip (also repairs an
    environment left without pip). Returns its Python interpreter."""
    python = TOOLS_BIN / f"python{EXE}"
    if not python.is_file():
        log(f"Creating the tools environment in {TOOLS_DIR}...")
        # Without pip: Debian/Ubuntu strip ensurepip unless python3-venv is
        # installed, and venv fails halfway when it is missing.
        run(sys.executable, "-m", "venv", "--without-pip", TOOLS_DIR,
            error="Could not create a Python virtual environment")
        use_tools_dir()
    if has_pip(python):
        return python
    log(f"Installing pip in {TOOLS_DIR}...")
    if subprocess.run([str(python), "-m", "ensurepip", "--default-pip"],
                      capture_output=True).returncode != 0:
        # No ensurepip: use the official standalone pip to install pip.
        pip_pyz = TOOLS_DIR / "pip.pyz"
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


def ensure_tool(tool: str) -> None:
    """Installs a missing Python-packaged tool (cmake, conan, gcovr) from PyPI
    into the project's virtual environment: no pipx, no admin rights, same on
    Linux and Windows."""
    if shutil.which(tool):
        return
    python = ensure_tools_env()
    log(f"{tool} not found, installing it in {TOOLS_DIR}...")
    run(python, "-m", "pip", "install", "--quiet", "--disable-pip-version-check", tool,
        error=f"pip install {tool} failed")
    if shutil.which(tool) is None:
        raise LauncherError(f"{tool} still not found after installing it in {TOOLS_DIR}")


def has_msvc() -> bool:
    vswhere = (Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
               / "Microsoft Visual Studio" / "Installer" / "vswhere.exe")
    if not vswhere.is_file():
        return False
    found = subprocess.run(
        [str(vswhere), "-latest", "-products", "*", "-requires",
         "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
        capture_output=True, text=True).stdout.strip()
    return bool(found)


def ensure_compiler() -> None:
    """Windows only: installs the MSVC compiler (Visual Studio Build Tools) with
    winget if it is missing. Linux distributions ship a compiler."""
    if os.name != "nt" or shutil.which("cl") or has_msvc():
        return
    require("winget", 'install Visual Studio Build Tools with the "Desktop development '
                      'with C++" workload')
    log("No C++ compiler found, installing Visual Studio Build Tools "
        "(several GB, asks for admin rights)...")
    # Its exit code is not reliable (e.g. already installed without the C++
    # workload): check the result instead.
    subprocess.run(["winget", "install", "--id", "Microsoft.VisualStudio.2022.BuildTools",
                    "--exact", "--accept-package-agreements", "--accept-source-agreements",
                    "--override", "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools "
                    "--includeRecommended"])
    if not has_msvc():
        raise LauncherError('MSVC still not found: add the "Desktop development with C++" '
                            "workload in the Visual Studio Installer")


def find_binary(name: str, override: str | None) -> Path | None:
    """Single-config generators put binaries in the build folder, multi-config
    ones (Visual Studio) in a per-configuration subfolder."""
    if override:
        candidates = [Path(override)]
    else:
        candidates = [BUILD_DIR / f"{name}{EXE}", BUILD_DIR / BUILD_TYPE / f"{name}{EXE}"]
    return next((path for path in candidates if path.is_file()), None)


def build(target: str, build_dir: Path = BUILD_DIR) -> None:
    run("cmake", "--build", build_dir, "--target", target, "--config", BUILD_TYPE,
        "--parallel", JOBS, error=f"{target} build failed")


def configure(*cmake_args: str, build_dir: Path = BUILD_DIR) -> None:
    """Installs the Conan packages into build_dir, then configures it with the
    Conan toolchain. Extra arguments are passed to CMake."""
    # The compiler comes first: pip builds Conan's dependencies (e.g. PyYAML)
    # from source when PyPI has no prebuilt wheel for this Python version.
    ensure_compiler()
    ensure_tool("cmake")
    ensure_tool("conan")
    # A fresh Conan install has no profile: detect one from the local compiler.
    if subprocess.run(["conan", "profile", "path", "default"],
                      capture_output=True).returncode != 0:
        log("No default Conan profile, detecting one...")
        run("conan", "profile", "detect", quiet=True, error="conan profile detect failed")

    log("Installing the Conan packages...")
    build_dir.mkdir(parents=True, exist_ok=True)
    conan_log = build_dir / "conan-install.log"
    with conan_log.open("w") as output:
        # Conan asks for the sudo password only if a system package is missing.
        status = subprocess.run(
            ["conan", "install", ".", f"--output-folder={build_dir}", "--build=missing",
             "-s", "compiler.cppstd=20", "-s", f"build_type={BUILD_TYPE}",
             "-o", f"&:graphics={GRAPHICS}",
             "-c", f"tools.system.package_manager:mode={CONAN_SYSTEM_PACKAGES}",
             "-c", "tools.system.package_manager:sudo=True"],
            stdout=output, stderr=subprocess.STDOUT).returncode
    if status != 0:
        lines = conan_log.read_text(errors="replace").splitlines()
        print("\n".join(lines[-30:]), file=sys.stderr)
        raise LauncherError(f"conan install failed (full output: {conan_log})")

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


# ---------- Static analysis (clang-tidy) ----------
def run_tidy(fix: bool) -> int:
    require("clang-tidy")
    compile_commands = BUILD_DIR / "compile_commands.json"
    if not compile_commands.is_file():
        log(f"Generating {compile_commands}...")
        configure()

    if shutil.which("run-clang-tidy"):
        log("Running clang-tidy on every file of the project...")
        command = ["run-clang-tidy", "-p", str(BUILD_DIR), "-quiet"] + (["-fix"] if fix else [])
        status = subprocess.run(command).returncode
    else:
        log("run-clang-tidy not found, checking files one by one...")
        files = sorted({entry["file"] for entry in json.loads(compile_commands.read_text())})
        status = 0
        for file in files:
            command = ["clang-tidy", "-p", str(BUILD_DIR), "--quiet"] + (["--fix"] if fix else [])
            if subprocess.run(command + [file]).returncode != 0:
                status = 1

    if fix:
        run_format()
    return status


# Same files as the CI's formatting check.
def run_format() -> None:
    require("clang-format")
    files = sorted(path for folder in ("engine", "common", "server", "client", "tests")
                   for pattern in ("*.cpp", "*.hpp") for path in Path(folder).rglob(pattern))
    log("Running clang-format on every file of the project...")
    run("clang-format", "-i", *files, error="clang-format failed")


# ---------- Unit tests (GoogleTest + CTest) ----------
def run_tests() -> int:
    configure("-DBUILD_TESTS=ON")
    log("Building the unit tests...")
    build("r-type_tests")
    log("Running the unit tests...")
    return subprocess.run(["ctest", "--test-dir", str(BUILD_DIR), "-C", BUILD_TYPE,
                           "--output-on-failure"]).returncode


# ---------- Test coverage (gcov + gcovr) ----------
# Uses its own build directory: instrumented binaries must not mix with the
# normal build.
def run_coverage() -> int:
    ensure_tool("cmake")
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


# ---------- Documentation (Doxygen) ----------
# Settings come from the Doxyfile; the diagrams are turned off when Graphviz is
# missing instead of failing. Warnings go to a log file, not to the terminal.
def run_docs() -> int:
    require("doxygen", "winget install DimitriVanHeesch.Doxygen, or your package manager")
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
    if subprocess.run(["doxygen", "-"], input=config, text=True).returncode != 0:
        raise LauncherError(f"doxygen failed (warnings and errors: {warnings})")
    count = warnings.read_text(errors="replace").count("warning:") if warnings.is_file() else 0
    if count:
        log(f"{count} Doxygen warnings, see {warnings}")
    log(f"HTML documentation: {output / 'html' / 'index.html'}")
    return 0


# ---------- Server and clients ----------
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
    """Returns the binary, built from the current sources. A binary given by
    the user (SERVER_BIN, CLIENT_BIN) is used as is."""
    binary = find_binary(name, override)
    if binary is not None and override:
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
    ensure_tool("cmake")
    log(f"Building {name}...")
    build(name)
    binary = find_binary(name, override)
    if binary is None:
        raise LauncherError(f"{name} still not found after building it")
    return binary


def stop(process: subprocess.Popen) -> None:
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()


def play(args: argparse.Namespace) -> int:
    server: subprocess.Popen | None = None
    clients: list[subprocess.Popen] = []
    container_started = False
    try:
        client_bin = ensure_binary("r-type_client", CLIENT_BIN, "-DBUILD_CLIENT=ON")

        if args.native:
            server_bin = ensure_binary("r-type_server", SERVER_BIN)
            log(f"Starting native server on port {args.port}...")
            server = subprocess.Popen([str(server_bin), str(args.port)])
        else:
            require("docker", "use --native to skip it")
            log("Building server image...")
            run("docker", "build", "-t", IMAGE, ".", error="Docker build failed")
            subprocess.run(["docker", "rm", "-f", CONTAINER], capture_output=True)
            log(f"Starting server in Docker on port {args.port}/udp...")
            run("docker", "run", "-d", "--rm", "--init", "--name", CONTAINER,
                "-p", f"{args.port}:4242/udp", IMAGE, quiet=True,
                error="Could not start the server container")
            container_started = True

        for i in range(1, args.clients + 1):
            log(f"Starting client {i}/{args.clients} -> {args.host}:{args.port}")
            clients.append(subprocess.Popen([str(client_bin), args.host, str(args.port)]))

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
            subprocess.run(["docker", "stop", "-t", "5", CONTAINER], capture_output=True)


# ---------- Arguments ----------
def positive(value: str) -> int:
    if not value.isdigit() or int(value) < 1:
        raise argparse.ArgumentTypeError("must be a positive number")
    return int(value)


def port(value: str) -> int:
    if not value.isdigit() or not 1 <= int(value) <= 65535:
        raise argparse.ArgumentTypeError("must be between 1 and 65535")
    return int(value)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Starts the R-Type server (in Docker by default, or natively) "
                    "and one or more clients.")
    parser.add_argument("-n", "--native", action="store_true",
                        help="run the server natively instead of in Docker")
    parser.add_argument("-c", "--clients", type=positive, default=1, metavar="N",
                        help="number of clients to launch (default: 1)")
    parser.add_argument("-p", "--port", type=port, default=4242,
                        help="UDP port of the server (default: 4242)")
    parser.add_argument("-H", "--host", default="127.0.0.1",
                        help="address the clients connect to (default: 127.0.0.1)")
    parser.add_argument("-g", "--graphics", choices=GRAPHICS_CHOICES, default=GRAPHICS,
                        help=f"graphics library of the client (default: {GRAPHICS})")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("-t", "--tidy", action="store_true",
                      help="run clang-tidy on the project and exit")
    mode.add_argument("-f", "--fix", action="store_true",
                      help="apply the clang-tidy automatic fixes, run clang-format and exit")
    mode.add_argument("-T", "--test", action="store_true",
                      help="build and run the unit tests (GoogleTest) and exit")
    mode.add_argument("-C", "--coverage", action="store_true",
                      help="run the unit tests with coverage, write an HTML report and exit")
    mode.add_argument("-d", "--docs", action="store_true",
                      help="generate the HTML documentation with Doxygen and exit")
    return parser.parse_args()


def main() -> int:
    global GRAPHICS
    args = parse_args()
    GRAPHICS = args.graphics
    if GRAPHICS not in GRAPHICS_CHOICES:  # argparse does not check the default
        print_error(f"GRAPHICS must be one of: {', '.join(GRAPHICS_CHOICES)}")
        return 2
    use_tools_dir()
    # Turns `docker stop`/`kill` of the launcher into a normal exit, so the
    # server and the clients are stopped too.
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(128 + signal.SIGTERM))
    try:
        if args.tidy or args.fix:
            return run_tidy(args.fix)
        if args.test:
            return run_tests()
        if args.coverage:
            return run_coverage()
        if args.docs:
            return run_docs()
        return play(args)
    except LauncherError as error:
        print_error(str(error))
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
