# Enemies and bosses

The enemies of R-Type are data: their stats, and new kinds of enemies and
bosses, are written in
[`server/enemies.json`](https://github.com/T3a1/RType-mirror/blob/dev/server/enemies.json). Adding an enemy or a
boss needs no C++ code, no rebuild and no new protocol version: restart the
server and use the new name in a [level file](LEVELS.md).

## How the server reads it

1. When it starts, before the levels, the server reads `enemies.json` next to
   its executable (`build/enemies.json`, copied there at every build, or
   `/enemies.json` in the Docker image).
2. It starts from the 8 **built-in** kinds, then applies the file: new stats
   for a built-in kind, and new kinds of enemies.
3. Every room plays with the same kinds. The level files call them by name, in
   their waves (`"enemy"`) and as their boss (`"boss"`).

If the file is missing, too large (more than 256 KiB) or invalid, the server
keeps the built-in enemies and says why. An invalid file never crashes the
server, and a level that names a missing enemy is skipped and reported:

```text
[WARN] Built-in enemies used: enemies.json: enemies.drone.health: must be from 1 to 40000
```

## File format

```json
{
    "enemies": {
        "basic": { "health": 120 },
        "kamikaze": {
            "base": "basic",
            "look": "small",
            "width": 51,
            "height": 51,
            "health": 30,
            "speed": 380,
            "score": 60,
            "pattern": { "kind": "dive", "diveDelay": 1.2, "diveSpeed": 900 }
        },
        "dreadnought": { "base": "hive", "health": 3500, "speed": 90, "score": 8000 }
    }
}
```

Each key of `enemies` is the name of a kind, used by the level files:

- **A built-in name** changes the stats of that kind. Every key is optional;
  `base` is not allowed.
- **Another name** adds a kind. It needs a `base`, the built-in kind it starts
  from: it takes its behavior and every stat the file does not give.

### Built-in kinds

| Name | Behavior | Default stats |
|------|----------|---------------|
| `basic` | Flies straight, does not shoot. | 100 health, speed 200, 100 points |
| `wave` | Follows a sine wave. | 50 health, speed 250, 50 points |
| `shooter` | Flies straight and shoots. | 100 health, speed 100, a shot every 1.5 s, 150 points |
| `small` | Fast, follows a sine wave. | 50 health, speed 300, 25 points |
| `medium` | Follows a zigzag and shoots, drops a power-up. | 150 health, speed 150, 200 points |
| `large` | Slow and tough, shoots, drops a power-up. | 500 health, speed 70, 500 points |
| `warship` | **Boss**: comes in, stops, moves up and down, three attack phases. | 2500 health, 5000 points |
| `hive` | **Boss**: larger and slower, sits on the ground and never moves once in place, same three phases. | 2500 health, 5000 points |
| `spinner` | Level 3. A spinning ring that follows a sine wave and shoots. | 120 health, speed 160, a shot every 1.6 s, 180 points |
| `bubble` | Level 3. Fast, follows a wide sine wave. | 60 health, speed 300, 80 points |
| `spore` | Level 4. Slow and tough, follows a zigzag, drops a power-up. | 260 health, speed 110, 250 points |
| `jelly` | Level 4. Waits, then rushes at the nearest player. | 90 health, speed 220, 120 points |
| `crab` | Level 4. Flies straight and shoots, drops a power-up. | 180 health, speed 130, a shot every 1.2 s, 260 points |
| `walker` | **Boss** of level 3: walks on the ground ([LEVELS.md](LEVELS.md#bosses)). | 3000 health, 7000 points |
| `twin` | **Boss** of level 4: a half on the ground and one on the ceiling. | 3600 health, 8000 points |
| `behemoth` | **Boss** of level 5: body, head (3 times the damage) and tail. | 4500 health, 10000 points |

A kind based on `warship`, `hive`, `walker`, `twin` or `behemoth` is a boss: it can only be the `boss` of a
level, and the other kinds only come in waves.

### Keys

| Key | Value |
|-----|-------|
| `base` | New kinds only, required: the built-in kind it starts from. |
| `look` | The built-in kind whose sprite the players see. The clients only know the sprites of the built-in kinds, so a new enemy reuses one; this is what lets it need no new protocol version. |
| `health` | Health points, a whole number from 1 to 40000. The difficulty of the game scales it. |
| `speed` | Speed towards the left, 0 to 3000 units per second. |
| `width`, `height` | Size of its collider, 1 to 1080 units. Keep the size of its `look` (see the table below), or shots seem to miss. |
| `score` | Points for the player who destroys it, 0 to 1000000. |
| `fireInterval` | Seconds between two shots, 0 to 60; 0 does not shoot. Bosses ignore it: their attack phases are code. |
| `pattern` | Its path, with the keys of a wave's [pattern](LEVELS.md#pattern) (`kind`, `amplitude`, `frequency`, `diveDelay`, `diveSpeed`). A wave's own pattern still replaces it. |
| `dropsPowerUp` | `true` if it leaves a power-up when destroyed. |

Any other key, a value of the wrong type or out of its range makes the file
invalid.

Sizes of the looks, to copy into `width` and `height`:

| Look | Width × height |
|------|----------------|
| `basic` | 63 × 72 |
| `wave` | 78 × 65 |
| `shooter` | 80 × 60 |
| `small` | 51 × 51 |
| `medium` | 96 × 63 |
| `large` | 126 × 96 |
| `warship` | 380 × 230 |
| `hive` | 400 × 230 |
| `spinner` | 76 × 70 |
| `bubble` | 76 × 66 |
| `spore` | 90 × 84 |
| `jelly` | 76 × 76 |
| `crab` | 90 × 80 |
| `walker` | 180 × 200 |
| `twin` | 200 × 210 (each half) |
| `behemoth` | 220 × 400 (the body, without its head and tail) |

## Add an enemy without writing C++

1. Add it to `server/enemies.json`, starting from the built-in kind that
   behaves the closest:

   ```json
   "turret": { "base": "shooter", "look": "large", "width": 126, "height": 96,
               "speed": 40, "health": 400, "fireInterval": 0.6, "score": 400 }
   ```

2. Call it in a level file:

   ```json
   { "time": 12, "enemy": "turret", "count": 2, "stepY": 400, "y": 540 }
   ```

3. Restart the server (`./rtype.py --native` copies the files and restarts
   it). The clients need nothing.

## Add a boss without writing C++

1. Add it, based on `warship` or `hive`:

   ```json
   "dreadnought": { "base": "hive", "health": 3500, "speed": 90, "score": 8000 }
   ```

2. Make it the boss of a level: `"boss": "dreadnought"`. It comes once every
   wave of the level is cleared, and the level ends when it is destroyed. Its
   health bar fills from the health it arrives with.

The boss chosen in the game settings (warship or hive) still replaces the boss
of every level.

## What still needs code

- **A new sprite**: the clients draw the sprites of the built-in kinds only. A
  new image needs a new `EntityType`, so a protocol version (see the
  [how-to](HOWTO.md#2-add-an-enemy-type)).
- **A new behavior**: a movement or an attack that no built-in kind has, such
  as new boss attack phases (`server/src/systems/BossSystem.cpp`).

## Code

- [`server/include/EnemyCatalog.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/EnemyCatalog.hpp): the
  `EnemyCatalog`, filled once at startup and then read by every room;
  `parseEnemies()` and `loadEnemies()` read the file (`server/src/EnemyFile.cpp`).
- [`server/include/EnemyDefs.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/EnemyDefs.hpp): the
  `EnemyDef` stats and the defaults of the built-in kinds (`ENEMY_DEFS`).
- Each enemy keeps its kind in `rtype::Enemy::kind`; `enemyDef(enemy)` gives
  its stats.
