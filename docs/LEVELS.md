# Levels

The levels of R-Type are data, not code: each level is a JSON file in
[`server/levels/`](https://github.com/T3a1/RType-mirror/blob/dev/server/levels/). Adding or changing a level needs no
rebuild of the code, only a restart of the server.

## How the server plays them

1. When it starts, the server reads every `.json` file of its levels folder,
   in the order of the file names (hence the `01-`, `02-` prefixes).
2. Every room plays the levels one after the other. A level starts its waves
   at the times written in its file, counted from its start.
3. Once all its waves have arrived and no enemy is left, the level's boss
   comes, if it has one. The level ends when the boss is destroyed, or right
   away without a boss. The next level starts 3 seconds later.
4. After the last level comes the **endless mode**: random waves, until the
   game is over.

Each level is announced to the players (`GameEvent{LevelStarted}`, see the
[protocol](PROTOCOL.md)): a large "LEVEL 2" banner, after "Level cleared",
during the 3-second pause, and the current level at the top of the screen.

The levels folder is `levels/` next to the server executable (`build/levels/`,
copied there by the build, or `/levels` in the Docker image). Another folder
can be given with `--levels DIR`:

```bash
./build/r-type_server --levels my-levels/
```

A file that cannot be read is skipped and reported in the server's output,
with what is wrong and where; the other levels are still played. Without any
valid level, the server plays the endless mode only. A level file never
crashes the server.

```text
[WARN] Level skipped: 03-boss.json: waves[4].count: must be a whole number from 1 to 20
```

## File format

```json
{
    "name": "First contact",
    "boss": "warship",
    "waves": [
        { "time": 2, "enemy": "basic", "count": 3, "spacing": 0.6, "y": 300 },
        { "time": 15, "enemy": "basic", "count": 5, "spacing": 0.3, "stepY": 90, "fromCenter": true },
        {
            "time": 40, "enemy": "basic", "count": 3, "spacing": 0.5, "stepY": 140,
            "pattern": { "kind": "dive", "diveDelay": 2, "diveSpeed": 650 }
        }
    ]
}
```

### Level

| Key | Required | Value |
|-----|:--------:|-------|
| `name` | yes | The name of the level, 1 to 64 characters. |
| `waves` | yes | The list of its waves, 1 to 1000. Their order in the file does not matter: they are sorted by `time`. |
| `boss` | no | The boss that comes after the waves: `warship`. Without it, the level ends with its last wave. |

### Bosses

| Name | What it does |
|------|--------------|
| `warship` | Comes in from the right, stops, then moves up and down. 2500 health points, shown in a bar at the bottom of the screen. Three phases as it is damaged: aimed shots, then a spread of 3 shots plus an aimed one, then a faster fan of 5 shots while moving faster. |

### Wave

A wave is a group of enemies of one type, arriving from the right edge of the
screen. The playfield is 1920 units wide and 1080 high, `y` going down.

| Key | Required | Default | Value |
|-----|:--------:|---------|-------|
| `time` | yes | | When the wave starts, in seconds from the start of the level (0 to 3600). |
| `enemy` | yes | | The enemy type: `basic`, `wave`, `shooter`, `small`, `medium` or `large`. |
| `count` | no | `1` | How many enemies, a whole number from 1 to 20. |
| `spacing` | no | `0` | Seconds between two enemies (0 to 10). With the same height, a spacing makes a snake. |
| `stepY` | no | `0` | Vertical distance between two enemies (0 to 1080). Without spacing, it makes a column. |
| `fromCenter` | no | `false` | `true`: the middle enemy comes first and the others after it, by distance (a V). |
| `y` | no | random | Height of the center of the wave (0 to 1080). Without it, a random height that keeps the whole wave on screen. |
| `pattern` | no | the enemy's | The path of the enemies, see below. Without it, each enemy follows the path of its type. |

### Pattern

| Key | Required | Default | Value |
|-----|:--------:|---------|-------|
| `kind` | yes | | `straight`, `sine` (smooth waves), `zigzag` or `dive` (goes straight, then rushes at the nearest player). |
| `amplitude` | no | `0` | Height of the waves of `sine` and `zigzag` (0 to 1080). |
| `frequency` | no | `0` | Waves per second of `sine` and `zigzag` (0 to 10). |
| `diveDelay` | no | `0` | Seconds before a `dive` rushes (0 to 60). |
| `diveSpeed` | no | `0` | Speed of the rush of a `dive` (0 to 5000). |

Any other key, a value of the wrong type or out of its range makes the file
invalid: a typo is reported instead of being silently ignored.

## Limits

To protect the server, a level file larger than 1 MiB is not read, and the
limits above (waves per level, enemies per wave, times and distances) are
checked when the file is loaded.

## Code

- [`server/include/Level.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/Level.hpp): the `Level` and
  `Wave` structures, `parseLevel()` and `loadLevels()` (JSON read with
  nlohmann-json).
- [`server/include/systems/LevelSystem.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/systems/LevelSystem.hpp):
  `levelSystem()`, which plays the levels, and the `LevelProgress` component
  on the spawner entity. While it is there, `spawnSystem()` sends no random
  waves.
