#!/usr/bin/env bash
#
# R-Type launcher
# Starts the server (in Docker by default, or natively) and one or more clients.
#
# Usage: ./rtype.sh [options]
#   -n, --native        Run the server natively instead of in Docker
#   -c, --clients N     Number of clients to launch (default: 1)
#   -p, --port PORT     UDP port of the server (default: 4242)
#   -H, --host HOST     Address the clients connect to (default: 127.0.0.1)
#   -t, --tidy          Run clang-tidy on the project and exit
#   -f, --fix           Run clang-tidy, apply its automatic fixes and exit
#   -T, --test          Build and run the unit tests (GoogleTest) and exit
#   -C, --coverage      Run the unit tests with coverage, write an HTML report and exit
#   -h, --help          Show this help

set -euo pipefail

# ---------- Defaults (can be overridden by environment variables) ----------
BUILD_DIR="${BUILD_DIR:-./build}"
SERVER_BIN="${SERVER_BIN:-$BUILD_DIR/r-type_server}"
CLIENT_BIN="${CLIENT_BIN:-$BUILD_DIR/r-type_client}"
IMAGE="${IMAGE:-rtype-server}"
CONTAINER="${CONTAINER:-rtype-server}"

NATIVE=false
TIDY=false
TIDY_FIX=false
TESTS=false
COVERAGE=false
CLIENTS=1
PORT=4242
HOST="127.0.0.1"

SERVER_PID=""
CLIENT_PIDS=()
CONTAINER_STARTED=false

# ---------- Helpers ----------
log()   { printf '\033[1;34m[run]\033[0m %s\n' "$*"; }
error() { printf '\033[1;31m[error]\033[0m %s\n' "$*" >&2; exit 1; }

usage() {
    sed -n '4,15p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

is_number() { [[ "$1" =~ ^[0-9]+$ ]]; }

# Installs the Conan packages into $BUILD_DIR, then configures it with the
# Conan toolchain. Extra arguments are passed to CMake.
configure() {
    command -v cmake >/dev/null || error "cmake is not installed"
    command -v conan >/dev/null || error "conan is not installed (pipx install conan, then conan profile detect)"
    log "Installing the Conan packages..."
    conan install . --output-folder="$BUILD_DIR" --build=missing \
        -s compiler.cppstd=20 -s build_type=Release >/dev/null \
        || error "conan install failed (see the output of: conan install . --output-folder=$BUILD_DIR --build=missing)"
    # CMake only reads the toolchain on a build folder's first configure: a
    # folder first configured without it must be configured from scratch.
    local fresh=()
    if [[ -f "$BUILD_DIR/CMakeCache.txt" ]] \
        && ! grep -qs 'conan_toolchain\.cmake' "$BUILD_DIR"/CMakeFiles/*/CMakeSystem.cmake; then
        log "$BUILD_DIR was configured without Conan, reconfiguring it from scratch..."
        fresh=(--fresh)
    fi
    cmake "${fresh[@]}" -B "$BUILD_DIR" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "$@" >/dev/null \
        || error "CMake configuration failed"
}

# ---------- Parse arguments ----------
while [[ $# -gt 0 ]]; do
    case "$1" in
        -n|--native)  NATIVE=true; shift ;;
        -c|--clients) CLIENTS="${2:-}"; shift 2 ;;
        -p|--port)    PORT="${2:-}"; shift 2 ;;
        -H|--host)    HOST="${2:-}"; shift 2 ;;
        -t|--tidy)    TIDY=true; shift ;;
        -f|--fix)     TIDY=true; TIDY_FIX=true; shift ;;
        -T|--test)    TESTS=true; shift ;;
        -C|--coverage) COVERAGE=true; shift ;;
        -h|--help)    usage ;;
        *)            error "Unknown option: $1 (see --help)" ;;
    esac
done

is_number "$CLIENTS" && [[ "$CLIENTS" -ge 1 ]] || error "--clients must be a positive number"
is_number "$PORT" && [[ "$PORT" -ge 1 && "$PORT" -le 65535 ]] || error "--port must be between 1 and 65535"

# ---------- Static analysis (clang-tidy) ----------
run_tidy() {
    command -v clang-tidy >/dev/null || error "clang-tidy is not installed"

    if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
        log "Generating $BUILD_DIR/compile_commands.json..."
        configure
    fi

    local fix_flag=()
    $TIDY_FIX && fix_flag=(-fix)

    if command -v run-clang-tidy >/dev/null; then
        log "Running clang-tidy on every file of the project..."
        run-clang-tidy -p "$BUILD_DIR" -quiet "${fix_flag[@]}"
    else
        log "run-clang-tidy not found, checking files one by one..."
        local status=0
        while IFS= read -r -d '' file; do
            clang-tidy -p "$BUILD_DIR" --quiet "${fix_flag[@]/-fix/--fix}" "$file" || status=1
        done < <(find client server -name '*.cpp' -print0)
        return "$status"
    fi
}

if $TIDY; then
    run_tidy
    exit $?
fi

# ---------- Unit tests (GoogleTest + CTest) ----------
run_tests() {
    configure -DBUILD_TESTS=ON
    log "Building the unit tests..."
    cmake --build "$BUILD_DIR" --target r-type_tests -j"$(nproc)" || error "Test build failed"
    log "Running the unit tests..."
    ctest --test-dir "$BUILD_DIR" --output-on-failure
}

if $TESTS; then
    run_tests
    exit $?
fi

# ---------- Test coverage (gcov + gcovr) ----------
# Uses its own build directory: instrumented binaries must not mix with the
# normal build.
COVERAGE_DIR="${COVERAGE_DIR:-./build-coverage}"

run_coverage() {
    command -v cmake >/dev/null || error "cmake is not installed, cannot build the tests"
    command -v gcovr >/dev/null || error "gcovr is not installed (pip install gcovr, or your package manager)"
    log "Building the unit tests with coverage..."
    cmake -B "$COVERAGE_DIR" -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON -DBUILD_CLIENT=OFF >/dev/null \
        || error "CMake configuration failed (is GoogleTest installed?)"
    cmake --build "$COVERAGE_DIR" --target r-type_tests -j"$(nproc)" || error "Test build failed"
    # Counters add up across runs: start from zero.
    find "$COVERAGE_DIR" -name '*.gcda' -delete
    log "Running the unit tests..."
    ctest --test-dir "$COVERAGE_DIR" --output-on-failure || error "Some tests failed"
    mkdir -p "$COVERAGE_DIR/coverage"
    gcovr --root . --object-directory "$COVERAGE_DIR" \
        --filter 'engine/' --filter 'common/' \
        --html-details "$COVERAGE_DIR/coverage/index.html" --print-summary
    log "HTML report: $COVERAGE_DIR/coverage/index.html"
}

if $COVERAGE; then
    run_coverage
    exit $?
fi

# ---------- Cleanup: runs on exit, Ctrl+C, or error ----------
cleanup() {
    trap - EXIT INT TERM   # avoid running twice
    log "Shutting down..."

    for pid in "${CLIENT_PIDS[@]:-}"; do
        [[ -n "$pid" ]] && kill "$pid" 2>/dev/null || true
    done

    if $NATIVE; then
        [[ -n "$SERVER_PID" ]] && kill "$SERVER_PID" 2>/dev/null || true
    elif $CONTAINER_STARTED; then
        docker stop -t 5 "$CONTAINER" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT INT TERM

# ---------- Build the client if missing ----------
if [[ ! -x "$CLIENT_BIN" ]]; then
    log "Client binary not found, building it..."
    configure -DBUILD_CLIENT=ON
    cmake --build "$BUILD_DIR" --target r-type_client -j"$(nproc)" \
        || error "Client build failed"
    [[ -x "$CLIENT_BIN" ]] || error "Client binary still not found after building: $CLIENT_BIN"
fi

# ---------- Start the server ----------
if $NATIVE; then
    if [[ ! -x "$SERVER_BIN" ]]; then
        log "Server binary not found, building it..."
        configure
        cmake --build "$BUILD_DIR" --target r-type_server -j"$(nproc)" \
            || error "Server build failed"
        [[ -x "$SERVER_BIN" ]] || error "Server binary still not found after building: $SERVER_BIN"
    fi
    log "Starting native server on port $PORT..."
    "$SERVER_BIN" "$PORT" &
    SERVER_PID=$!
else
    command -v docker >/dev/null || error "Docker is not installed (use --native to skip it)"

    log "Building server image..."
    docker build -t "$IMAGE" . || error "Docker build failed"

    docker rm -f "$CONTAINER" >/dev/null 2>&1 || true

    log "Starting server in Docker on port $PORT/udp..."
    docker run -d --rm --init --name "$CONTAINER" -p "$PORT:4242/udp" "$IMAGE" >/dev/null \
        || error "Could not start the server container"
    CONTAINER_STARTED=true
fi

# ---------- Start the clients ----------
for ((i = 1; i <= CLIENTS; i++)); do
    log "Starting client $i/$CLIENTS -> $HOST:$PORT"
    "$CLIENT_BIN" "$HOST" "$PORT" &
    CLIENT_PIDS+=($!)
done

# ---------- Wait until every client window is closed ----------
log "Running. Close all clients or press Ctrl+C to stop."
for pid in "${CLIENT_PIDS[@]}"; do
    wait "$pid" || true
done