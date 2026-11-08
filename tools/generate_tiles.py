#!/usr/bin/env python3
"""Generates the sprite sheet of the level obstacles.

Usage: `./tools/generate_tiles.py` (standard library only).

Every pixel is drawn here, so the file contains no third-party content. It
goes to client/assets/sprites/obstacles.png: one row of 16 x 16 frames, read
by client/src/SpriteSheets.cpp (ObstacleFrames). The output only depends on
this file.

Frames, from left to right:
  0. wall: a riveted metal block that nothing breaks;
  1. tile: an intact destroyable block;
  2. cracked tile: the same block once it has lost half of its health.
"""

import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "client" / "assets" / "sprites" / "obstacles.png"

SIZE = 16
FRAMES = 3

TRANSPARENT = (0, 0, 0, 0)


def wall(x, y):
    """A grey metal plate with a bevel and four rivets."""
    if x in (0, SIZE - 1) or y in (0, SIZE - 1):
        return (40, 44, 52, 255)
    if x == 1 or y == 1:
        return (150, 158, 170, 255)
    if x == SIZE - 2 or y == SIZE - 2:
        return (70, 76, 88, 255)
    if (x, y) in ((3, 3), (12, 3), (3, 12), (12, 12)):
        return (200, 206, 214, 255)
    if (x, y) in ((4, 4), (13, 4), (4, 13), (13, 13)):
        return (50, 54, 62, 255)
    return (105, 112, 124, 255)


def tile(x, y):
    """A brown block made of four bricks."""
    if x in (0, SIZE - 1) or y in (0, SIZE - 1):
        return (60, 30, 15, 255)
    # Mortar between the bricks, offset on the two rows.
    if y == SIZE // 2 or (y < SIZE // 2 and x == SIZE // 2) or (
        y > SIZE // 2 and x in (4, 12)
    ):
        return (95, 60, 40, 255)
    if x == 1 or y == 1 or y == SIZE // 2 + 1:
        return (215, 130, 70, 255)
    return (175, 95, 45, 255)


# The crack of a damaged tile, as (x, y) pixels.
CRACK = {
    (7, 2), (7, 3), (8, 4), (8, 5), (7, 6), (6, 7), (6, 8), (7, 9),
    (8, 10), (9, 10), (10, 11), (9, 12), (9, 13), (5, 8), (4, 9),
    (3, 9), (10, 5), (11, 5),
}


def cracked(x, y):
    """The tile with a dark crack through it."""
    if (x, y) in CRACK:
        return (30, 15, 8, 255)
    return tile(x, y)


DRAWERS = (wall, tile, cracked)


def png(width, height, pixels):
    """Encodes RGBA pixels, row by row, as a PNG file."""

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(
            ">I", zlib.crc32(body) & 0xFFFFFFFF
        )

    raw = bytearray()
    for y in range(height):
        raw.append(0)  # No filter on this row.
        for x in range(width):
            raw.extend(pixels[y][x])
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + chunk(b"IEND", b"")
    )


def main():
    """Draws every frame side by side and writes the sheet."""
    width = SIZE * FRAMES
    pixels = [[TRANSPARENT] * width for _ in range(SIZE)]
    for frame, draw in enumerate(DRAWERS):
        for y in range(SIZE):
            for x in range(SIZE):
                pixels[y][frame * SIZE + x] = draw(x, y)
    OUTPUT.write_bytes(png(width, SIZE, pixels))
    print(f"Wrote {OUTPUT.relative_to(ROOT)} ({width} x {SIZE})")


if __name__ == "__main__":
    main()
