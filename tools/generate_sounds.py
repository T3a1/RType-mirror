#!/usr/bin/env python3
"""Generates the sound effects and musics of the client.

Usage: `./tools/generate_sounds.py` (needs ffmpeg with libvorbis, for the
musics).

Everything is synthesized here from simple waveforms, so the files contain no
third-party content. They go to client/assets/sounds/ (WAV) and
client/assets/music/ (Ogg Vorbis), under the names listed in
client/src/audio/SoundBoard.cpp. The output only depends on this file.
"""

import array
import math
import random
import shutil
import subprocess
import sys
import tempfile
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOUNDS = ROOT / "client" / "assets" / "sounds"
MUSIC = ROOT / "client" / "assets" / "music"

RATE = 44100
OGG_QUALITY = "4"


def square(phase, duty=0.5):
    """A square wave at a phase counted in periods."""
    return 1.0 if phase % 1.0 < duty else -1.0


def triangle(phase):
    """A triangle wave at a phase counted in periods."""
    return 4.0 * abs((phase % 1.0) - 0.5) - 1.0


def note_frequency(note):
    """The frequency of a MIDI note."""
    return 440.0 * 2.0 ** ((note - 69) / 12.0)


def normalized(samples, peak):
    """Scales samples so that their loudest one reaches peak."""
    loudest = max(abs(sample) for sample in samples) or 1.0
    return [sample * peak / loudest for sample in samples]


def write_wav(path, samples):
    """Writes mono samples between -1 and 1 as a 16-bit WAV file."""
    data = array.array("h", (int(max(-1.0, min(1.0, s)) * 32767) for s in samples))
    if sys.byteorder == "big":
        data.byteswap()
    with wave.open(str(path), "wb") as file:
        file.setnchannels(1)
        file.setsampwidth(2)
        file.setframerate(RATE)
        file.writeframes(data.tobytes())


def sweep(duration, start, end, wave_function, decay):
    """A note sliding from one frequency to another as it fades out."""
    samples = []
    phase = 0.0
    count = int(duration * RATE)
    for i in range(count):
        progress = i / count
        phase += (start * (end / start) ** progress) / RATE
        samples.append(wave_function(phase) * math.exp(-decay * progress))
    return samples


def noise(duration, cutoff, decay, rng):
    """Filtered white noise fading out; a lower cutoff sounds deeper."""
    samples = []
    level = 0.0
    count = int(duration * RATE)
    smoothing = 1.0 - math.exp(-2.0 * math.pi * cutoff / RATE)
    for i in range(count):
        level += smoothing * (rng.uniform(-1.0, 1.0) - level)
        samples.append(level * math.exp(-decay * i / count))
    return samples


def melody(notes, length, wave_function):
    """A sequence of MIDI notes of the same length, each fading out."""
    samples = []
    for note in notes:
        frequency = note_frequency(note)
        count = int(length * RATE)
        for i in range(count):
            samples.append(
                wave_function(frequency * i / RATE) * math.exp(-3.0 * i / count)
            )
    return samples


def mixed(*layers):
    """Adds layers of samples of any length."""
    samples = [0.0] * max(len(layer) for layer in layers)
    for layer in layers:
        for i, sample in enumerate(layer):
            samples[i] += sample
    return samples


def effects():
    """Returns the samples of every sound effect, by file name."""
    rng = random.Random(4242)
    return {
        "player_shot.wav": normalized(
            sweep(0.13, 1500.0, 420.0, lambda p: square(p, 0.25), 3.5), 0.45
        ),
        "charge.wav": normalized(
            sweep(0.7, 170.0, 950.0, lambda p: square(p, 0.35), 0.4), 0.32
        ),
        "charged_shot.wav": normalized(
            mixed(
                sweep(0.5, 180.0, 1100.0, lambda p: square(p, 0.4), 2.5),
                [s * 0.35 for s in noise(0.5, 3000.0, 4.0, rng)],
            ),
            0.6,
        ),
        "enemy_shot.wav": normalized(
            sweep(0.16, 620.0, 210.0, lambda p: square(p, 0.5), 3.0), 0.3
        ),
        "explosion.wav": normalized(
            mixed(
                noise(0.7, 1400.0, 5.0, rng),
                [s * 0.9 for s in sweep(0.5, 110.0, 35.0, triangle, 4.0)],
            ),
            0.75,
        ),
        "power_up.wav": normalized(
            melody([72, 76, 79, 84], 0.075, lambda p: square(p, 0.25)), 0.5
        ),
        "level_start.wav": normalized(
            melody([64, 69, 71, 76, 76], 0.13, lambda p: square(p, 0.5)), 0.55
        ),
        "game_over.wav": normalized(
            melody([69, 65, 62, 57, 53], 0.24, triangle), 0.7
        ),
    }


class Loop:
    """A music buffer whose end wraps to its start, so that it loops cleanly."""

    def __init__(self, beats, tempo):
        self.beat = 60.0 / tempo
        self.samples = [0.0] * int(beats * self.beat * RATE)

    def note(self, start, length, note, volume, wave_function):
        """Adds a fading note, with its start and length in beats."""
        frequency = note_frequency(note)
        first = int(start * self.beat * RATE)
        count = int(length * self.beat * RATE)
        attack = int(0.004 * RATE)
        for i in range(count):
            envelope = min(1.0, i / attack) * min(1.0, (count - i) / attack)
            value = wave_function(frequency * i / RATE) * envelope
            value *= volume * math.exp(-1.6 * i / count)
            self.samples[(first + i) % len(self.samples)] += value

    def hit(self, start, length, volume, cutoff, rng):
        """Adds a drum hit made of filtered noise."""
        first = int(start * self.beat * RATE)
        layer = noise(length * self.beat, cutoff, 6.0, rng)
        for i, value in enumerate(layer):
            self.samples[(first + i) % len(self.samples)] += value * volume


def menu_music():
    """A slow, calm loop of 16 bars at 92 beats per minute."""
    music = Loop(64, 92)
    # One chord per bar, repeated twice: Am, F, C, G, Am, F, Dm, E.
    chords = [
        (45, [57, 60, 64]),
        (41, [53, 57, 60]),
        (48, [55, 60, 64]),
        (43, [55, 59, 62]),
        (45, [57, 60, 64]),
        (41, [53, 57, 60]),
        (38, [53, 57, 62]),
        (40, [56, 59, 64]),
    ]
    tune = [
        [76, None, 72, None, 74, None, 72, 69],
        [72, None, 69, None, 65, None, 69, 72],
        [79, None, 76, None, 72, None, 74, 76],
        [74, None, 71, None, 67, None, None, 71],
        [76, None, 81, None, 79, None, 76, 72],
        [77, None, 72, None, 69, None, 72, 77],
        [74, None, 77, None, 81, None, 77, 74],
        [76, None, None, None, 71, None, 68, None],
    ]
    for repeat in range(2):
        for bar, (bass, chord) in enumerate(chords):
            start = (repeat * 8 + bar) * 4
            music.note(start, 2.0, bass, 0.34, triangle)
            music.note(start + 2.0, 2.0, bass, 0.28, triangle)
            for step in range(8):
                arpeggio = chord[[0, 1, 2, 1][step % 4]]
                music.note(
                    start + step * 0.5, 0.5, arpeggio + 12, 0.1,
                    lambda p: square(p, 0.25),
                )
            if repeat == 1:
                for step, note in enumerate(tune[bar]):
                    if note is not None:
                        music.note(start + step * 0.5, 1.0, note, 0.17, triangle)
    return normalized(music.samples, 0.7)


def game_music():
    """A fast, driving loop of 16 bars at 148 beats per minute."""
    rng = random.Random(1987)
    music = Loop(64, 148)
    # One root per bar, repeated twice: Em, Em, C, D, Em, G, Am, B.
    roots = [40, 40, 36, 38, 40, 43, 45, 47]
    riff = [0, 0, 12, 0, 7, 0, 10, 12]
    tune = [
        [76, None, 79, 76, 74, None, 71, None],
        [76, None, 71, 74, 76, None, None, 79],
        [79, None, 76, 72, 76, None, 79, None],
        [78, None, 74, 69, 74, None, 78, None],
        [76, 79, 83, None, 81, 79, 76, None],
        [79, None, 83, 86, 83, None, 79, None],
        [81, None, 84, 81, 76, None, 72, 76],
        [75, None, 78, 83, 81, 78, 75, None],
    ]
    for repeat in range(2):
        for bar, root in enumerate(roots):
            start = (repeat * 8 + bar) * 4
            for step, offset in enumerate(riff):
                music.note(
                    start + step * 0.5, 0.5, root + offset, 0.3,
                    lambda p: square(p, 0.5),
                )
                music.hit(start + step * 0.5, 0.12, 0.09, 7000.0, rng)
            for beat in (0, 2):
                music.note(start + beat, 0.4, 33, 0.5, triangle)
            for beat in (1, 3):
                music.hit(start + beat, 0.4, 0.42, 3200.0, rng)
            for step, note in enumerate(tune[bar]):
                if note is not None:
                    music.note(
                        start + step * 0.5, 0.75, note, 0.2 + 0.03 * repeat,
                        lambda p: square(p, 0.25),
                    )
    return normalized(music.samples, 0.75)


def write_ogg(path, samples):
    """Writes mono samples between -1 and 1 as an Ogg Vorbis file."""
    with tempfile.TemporaryDirectory() as directory:
        source = Path(directory) / "music.wav"
        write_wav(source, samples)
        subprocess.run(
            [
                "ffmpeg", "-loglevel", "error", "-y", "-i", str(source),
                "-c:a", "libvorbis", "-q:a", OGG_QUALITY,
                "-map_metadata", "-1", "-fflags", "+bitexact",
                "-flags:a", "+bitexact", str(path),
            ],
            check=True,
        )


def main():
    """Writes every file."""
    if shutil.which("ffmpeg") is None:
        sys.exit("ffmpeg is needed to encode the musics")
    SOUNDS.mkdir(parents=True, exist_ok=True)
    MUSIC.mkdir(parents=True, exist_ok=True)
    for name, samples in effects().items():
        write_wav(SOUNDS / name, samples)
    write_ogg(MUSIC / "menu.ogg", menu_music())
    write_ogg(MUSIC / "game.ogg", game_music())


if __name__ == "__main__":
    main()
