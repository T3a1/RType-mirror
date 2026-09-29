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
#   -h, --help          Show this help

set -euo pipefail

# ---------- Defaults (can be overridden by environment variables) ----------
BUILD_DIR="${BUILD_DIR:-./build}"
SERVER_BIN="${SERVER_BIN:-$BUILD_DIR/r-type_server}"
CLIENT_BIN="${CLIENT_BIN:-$BUILD_DIR/r-type_client}"

NATIVE=false
CLIENTS=1
PORT=4242
HOST="127.0.0.1"

SERVER_PID=""
CLIENT_PIDS=()
COMPOSE=()

# ---------- Helpers ----------
log()   { printf '\033[1;34m[run]\033[0m %s\n' "$*"; }
error() { printf '\033[1;31m[error]\033[0m %s\n' "$*" >&2; exit 1; }

usage() {
    sed -n '4,11p' "$0" | sed 's/^# \{0,1\}//'
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
        -h|--help)    usage ;;
        *)            error "Unknown option: $1 (see --help)" ;;
    esac
done

is_number "$CLIENTS" && [[ "$CLIENTS" -ge 1 ]] || error "--clients must be a positive number"
is_number "$PORT" && [[ "$PORT" -ge 1 && "$PORT" -le 65535 ]] || error "--port must be between 1 and 65535"

# ---------- Cleanup: runs on exit, Ctrl+C, or error ----------
cleanup() {
    trap - EXIT INT TERM   # avoid running twice
    log "Shutting down..."

    for pid in "${CLIENT_PIDS[@]:-}"; do
        [[ -n "$pid" ]] && kill "$pid" 2>/dev/null || true
    done

    if $NATIVE; then
        [[ -n "$SERVER_PID" ]] && kill "$SERVER_PID" 2>/dev/null || true
    elif [[ ${#COMPOSE[@]} -gt 0 ]]; then
        "${COMPOSE[@]}" down
    fi
}
trap cleanup EXIT INT TERM

# ---------- Start the server ----------
if $NATIVE; then
    [[ -x "$SERVER_BIN" ]] || error "Server binary not found: $SERVER_BIN (did you build the project?)"
    log "Starting native server on port $PORT..."
    "$SERVER_BIN" "$PORT" &
    SERVER_PID=$!
else
    command -v docker >/dev/null || error "Docker is not installed (use --native to skip it)"

    if docker compose version >/dev/null 2>&1; then
        COMPOSE=(docker compose)
    elif command -v docker-compose >/dev/null; then
        COMPOSE=(docker-compose)
    else
        error "Docker Compose is not available"
    fi

    log "Starting server in Docker on port $PORT/udp..."
    RTYPE_PORT="$PORT" "${COMPOSE[@]}" up -d --build
fi

# ---------- Start the clients ----------
[[ -x "$CLIENT_BIN" ]] || error "Client binary not found: $CLIENT_BIN (did you build the project?)"

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