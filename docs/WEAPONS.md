# Weapons and power-ups

## In the game

- **Normal shot**: hold the fire key to shoot continuously.
- **Charged shot**: hold the fire key for 1 second, then release it. While
  it charges, a glow appears in front of the ship; on release, a large beam
  flies across the screen, goes **through** every enemy on its way and deals
  each one heavy damage (300, against 50 for a normal shot). With auto-fire
  on, the key is never released: the ship keeps its rapid fire instead.
- **Power-ups**: medium and large enemies, and the boss, leave an orb when
  they are destroyed. It slowly floats to the left; the ship that touches it
  gets one more **weapon level**:

  | Level | Shots at once |
  |------:|---------------|
  | 1 | 1 |
  | 2 | 3, in a spread |
  | 3 | 5, in a wider spread |

  A new ship starts at level 1.

## Weapon settings: `server/weapons.json`

The weapons are data, read by the server when it starts, from `weapons.json`
next to its executable (`build/weapons.json`, copied there by the build, or
`/weapons.json` in the Docker image). Every key is optional: a missing key
keeps its default. A missing, too large (over 64 KiB) or invalid file is
reported in the server's output and the defaults are used; it never stops
the server.

```json
{
    "fireInterval": 0.25,
    "shotSpeed": 900,
    "shotDamage": 50,
    "enemyShotSpeed": 450,
    "chargeTime": 1.0,
    "beamSpeed": 1300,
    "beamDamage": 300,
    "spreadAngle": 0.12,
    "maxLevel": 3
}
```

| Key | Default | Range | Meaning |
|-----|--------:|-------|---------|
| `fireInterval` | 0.25 | 0.05 to 5 | Seconds between two shots of a player |
| `shotSpeed` | 900 | 100 to 5000 | Speed of a player's shot, in playfield units per second (the playfield is 1920 wide) |
| `shotDamage` | 50 | 1 to 10000 | Damage of a player's shot (a basic enemy has 100 health points) |
| `enemyShotSpeed` | 450 | 100 to 5000 | Speed of an enemy's shot |
| `chargeTime` | 1.0 | 0.1 to 10 | Seconds the fire key must be held for a charged shot |
| `beamSpeed` | 1300 | 100 to 5000 | Speed of a charged shot |
| `beamDamage` | 300 | 1 to 10000 | Damage of a charged shot to each enemy it goes through |
| `spreadAngle` | 0.12 | 0 to 1 | Angle between two shots of a spread, in radians |
| `maxLevel` | 3 | 1 to 5 | Highest weapon level; level N fires 2N - 1 shots |

Unknown keys and values of the wrong type or out of range make the file
invalid, so that a typo is reported instead of silently ignored.

## Code

- [`server/include/Weapons.hpp`](../server/include/Weapons.hpp):
  `WeaponStats`, `parseWeapons()` and `loadWeapons()`.
- [`server/include/systems/ShootSystem.hpp`](../server/include/systems/ShootSystem.hpp):
  the spread, the charge and the charged shot (`Piercing` projectile).
- [`server/include/systems/PowerUpSystem.hpp`](../server/include/systems/PowerUpSystem.hpp):
  dropping power-ups (`EnemyDef::dropsPowerUp`) and picking them up.
- The `Charging` flag of a player ship's snapshot record makes the clients
  draw the glow (see the [protocol](PROTOCOL.md)).
