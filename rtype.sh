#!/usr/bin/env bash
#
# R-Type launcher
# Starts the server (in Docker by default, or natively) and one or more clients.
#
# Usage: ./run.sh [options]
#   -n, --native        Run the server natively instead of in Docker
#   -c, --clients N     Number of clients to launch (default: 1)
#   -p, --port PORT     UDP port of the server (default: 4242)
#   -H, --host HOST     Address the clients connect to (default: 127.0.0.1)
#   -t, --tidy          Run clang-tidy on the project and exit
#   -f, --fix           Run clang-tidy, apply its automatic fixes and exit
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
    sed -n '4,13p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

is_number() { [[ "$1" =~ ^[0-9]+$ ]]; }

# ---------- Parse arguments ----------
while [[ $# -gt 0 ]]; do
    case "$1" in
        -n|--native)  NATIVE=true; shift ;;
        -c|--clients) CLIENTS="${2:-}"; shift 2 ;;
        -p|--port)    PORT="${2:-}"; shift 2 ;;
        -H|--host)    HOST="${2:-}"; shift 2 ;;
        -t|--tidy)    TIDY=true; shift ;;
        -f|--fix)     TIDY=true; TIDY_FIX=true; shift ;;
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
        command -v cmake >/dev/null || error "cmake is not installed, cannot generate compile_commands.json"
        log "Generating $BUILD_DIR/compile_commands.json..."
        cmake -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null \
            || error "CMake configuration failed, cannot run clang-tidy"
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
    command -v cmake >/dev/null || error "cmake is not installed, cannot build the client"
    log "Client binary not found, building it..."
    cmake -B "$BUILD_DIR" -DBUILD_CLIENT=ON >/dev/null \
        && cmake --build "$BUILD_DIR" --target r-type_client -j"$(nproc)" \
        || error "Client build failed"
    [[ -x "$CLIENT_BIN" ]] || error "Client binary still not found after building: $CLIENT_BIN"
fi

# ---------- Start the server ----------
if $NATIVE; then
    [[ -x "$SERVER_BIN" ]] || error "Server binary not found: $SERVER_BIN (did you build the project?)"
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