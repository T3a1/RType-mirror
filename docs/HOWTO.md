# How-to guides

Practical recipes for the most common changes. Read the
[architecture](ARCHITECTURE.md) first if you do not know what an ECS is. For
content that needs no C++, see [writing levels](LEVELS.md) and
[weapons and power-ups](WEAPONS.md).

## Contents

1. [Add a component and a system](#1-add-a-component-and-a-system)
2. [Add an enemy type](#2-add-an-enemy-type)
3. [Add a packet type](#3-add-a-packet-type)
4. [Build and debug](#4-build-and-debug)
5. [Add a sound](#5-add-a-sound)
6. [Add a menu page](#6-add-a-menu-page)
7. [Add a setting](#7-add-a-setting)

## 1. Add a component and a system

This example makes some entities regain health over time, on the server.

**1. Choose where it goes.**

| The code is used by | Component | System |
|---------------------|-----------|--------|
| The server only | `server/include/components/` | `server/include/systems/`, `server/src/systems/` |
| The client only | `client/include/components/` | `client/include/systems/`, `client/src/systems/` |
| Both | `common/include/common/components/` | `common/include/common/systems/`, `common/src/systems/` |

**2. Write the component**: plain data, no logic, in
`server/include/components/Regeneration.hpp`:

```cpp
namespace server {

/**
 * @struct Regeneration
 * @brief An entity that regains health over time.
 * @param perSecond The health points regained per second.
 * @param stored The fraction of a point not applied yet.
 **/
struct Regeneration {
    float perSecond = 1.0F;
    float stored = 0.0F;
};

} // namespace server
```

**3. Write the system**, a function that takes the registry and the elapsed time,
in `server/src/systems/RegenerationSystem.cpp`, with its header in
`server/include/systems/`:

```cpp
void regenerationSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<Regeneration, rtype::Health>(
        [dt](engine::Entity, Regeneration &regeneration, rtype::Health &health) {
            regeneration.stored += regeneration.perSecond * dt.count();
            const int points = static_cast<int>(regeneration.stored);
            regeneration.stored -= static_cast<float>(points);
            health.current = std::min(health.current + points, health.max);
        });
}
```

**4. Add the source file to CMake**: `SOURCE_FILES` in `server/CMakeLists.txt`, or
the sources of the `r-type_server_spawn` library if a test needs it (tests cannot
link the executable).

**5. Register it** in `Game::registerSystems()` in `server/src/Game.cpp`. The
stage and the order decide when it runs:

```cpp
m_scheduler.add(Stage::Simulation, "regeneration", server::regenerationSystem,
                playing);
```

`playing` stops it while the room is waiting. A system that needs more than the
registry and the time is wrapped in a lambda, like the others in that function.

**6. Give the component to entities**: usually in `EntityFactory::create()`
(`server/src/EntityFactory.cpp`), with
`registry.emplace<Regeneration>(entity, Regeneration{.perSecond = 2.0F});`.

**7. Test it**, with no window and no socket: create a registry, add an entity, run
the system, check the components. Add the new file to `SOURCE_FILES` in
`tests/CMakeLists.txt`:

```cpp
TEST(RegenerationSystem, RegainsHealthUpToTheMaximum)
{
    engine::Registry registry;
    const engine::Entity entity = registry.create();
    registry.emplace<server::Regeneration>(
        entity, server::Regeneration{.perSecond = 10.0F});
    registry.emplace<rtype::Health>(entity, rtype::Health{.current = 95, .max = 100});

    server::regenerationSystem(registry, engine::Duration{1.0F});

    EXPECT_EQ(registry.get<rtype::Health>(entity).current, 100);
}
```

**Only the server knows this component.** To show it on the clients, it must also
travel in a packet ([section 3](#3-add-a-packet-type)).

## 2. Add an enemy type

**Most new enemies need no code.** A kind of enemy or boss that reuses the
behavior and the sprite of a built-in one (with other stats, size, path or
points) is a few lines of
[`server/enemies.json`](ENEMIES.md), then its name
in a level file. Write code only for a **new sprite** or a **new behavior**.

A new sprite needs a new value on both sides, so clients and server must be
rebuilt together. Here is what each step changes, for an enemy called `Turret`:

| Step | File | What to do |
|------|------|------------|
| 1 | `common/include/common/components/Enemy.hpp` | Add `Turret` to `EnemyType`, before `Boss` |
| 2 | `common/include/common/components/EntityType.hpp` | Add `EnemyTurret` at the **end** of `EntityType`: the values are sent on the network |
| 3 | `common/src/protocol/Messages.cpp` | If it is the new last value, change the upper bound of `readEnum(in, EntityType::..., entity.type)`, or clients and server reject it |
| 4 | `server/include/EnemyDefs.hpp` | Add its default stats to `ENEMY_DEFS`, **in the order of `EnemyType`**, and change the size of the array |
| 5 | `server/src/EnemyCatalog.cpp` | Add `"turret"` to `BuiltinNames`, in the same order, so that `enemies.json` and the level files can use it |
| 6 | `client/src/SpriteSheets.cpp` | Add its frames and a `case` in `lookOf()`. A new image also needs a value in `Sheet` (`client/include/SpriteSheets.hpp`) and a line in the list of files, in the same order |
| 7 | `common/include/common/protocol/PacketHeader.hpp` | Add 1 to `ProtocolVersion` |
| 8 | `docs/PROTOCOL.md` | Add the entity type, the version in the header table and a line in the [versioning table](PROTOCOL.md#12-versioning) |
| 9 | `docs/ENEMIES.md` | Add it to the built-in kinds and to the sizes of the looks |

The collider in `ENEMY_DEFS` should match the size the client draws, or shots will
seem to miss. Then use it in a level ([writing levels](LEVELS.md)):

```json
{ "time": 2, "enemy": "turret", "count": 2, "stepY": 300, "y": 540 }
```

Once it exists, `enemies.json` can also derive new kinds from it (`"base":
"turret"`) without code.

## 3. Add a packet type

For a packet called `Ping`, sent by clients, with one 32-bit value:

**1. Declare the type** in `common/include/common/protocol/PacketType.hpp`: a value
in `PacketType`, an entry in `AllPacketTypes`, a `case` in `canSend()` (who may send
it) and a `case` in `isReliable()`. The compiler warns about a missing `case`.

**2. Declare the payload** in `common/include/common/protocol/Messages.hpp`:

```cpp
struct Ping {
    static constexpr PacketType Type = PacketType::Ping;
    std::uint32_t time = 0;
    bool operator==(const Ping &) const = default;
};
```

and the declarations of `encode()` and `decode()` beside the others.

**3. Write it and read it** in `common/src/protocol/Messages.cpp`, field by field and
never with a `memcpy`. `decode()` returns `false` for an out-of-range value:

```cpp
void encode(const Ping &message, ByteWriter &out) { out.u32(message.time); }

bool decode(ByteReader &in, Ping &message)
{
    message.time = in.u32();
    return true;
}
```

**4. Add it to the packet layer**: to the `Payload` variant in
`common/include/common/protocol/Packet.hpp`, and a `case` in `decodePacket()` in
`common/src/protocol/Packet.cpp`:

```cpp
case PacketType::Ping:
    return decodePayload<Ping>(header, in);
```

**5. Handle it where it arrives**:

- on the server, in `NetworkServer::handleSessionPacket()`
  (`server/src/network/NetworkServer.cpp`), on the network thread. If the game needs
  it, push an `InboundMessage` to the room's queue, never touch the registry;
- on the client, in `NetworkClient::handlePacket()`, which gives the game what
  concerns it, and then in `networkSyncSystem()` or the lobby handling of `Game`.

**6. Test it**: add a sample to the round-trip list in `tests/src/packet_test.cpp`.
The randomized tests of `packet_fuzz_test.cpp` then cover what they generate from
the known types.

**7. Document it**: the type and the payload in [PROTOCOL.md](PROTOCOL.md), and add 1
to `ProtocolVersion` with a line in the [versioning table](PROTOCOL.md#12-versioning).
Someone must be able to write a client from this document alone.

## 4. Build and debug

### Build

`./rtype.py` does it all for normal use ([installation guide](INSTALLATION.md)). To
debug, build a separate folder with debug symbols:

```bash
conan install . --output-folder=build-debug --build=missing -s compiler.cppstd=20 -s build_type=Debug
```

```bash
cmake -B build-debug -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
```

```bash
cmake --build build-debug -j8
```

### Run under a debugger

Start the server, then a client in the same room in a second terminal:

```bash
gdb --args ./build-debug/r-type_server --no-console
```

```bash
gdb --args ./build-debug/r-type_client 127.0.0.1 4242 --room 1
```

`--no-console` keeps the server's administration console from reading the terminal
that gdb uses. `--invincible` on the server keeps your ship alive while you look at
something. Do not stay on a breakpoint for more than 5 seconds if the connection
matters: the other side declares a timeout.

### Logs

Use `BB_DEBUG`, `BB_INFO`, `BB_WARN` and `BB_ERROR` (`engine/Logger.hpp`), not
`std::cout`: on the server, they never make the game wait for the terminal. The
minimum level is `Info`. To see more, for example why a datagram was refused, change
the `engine::Logger::instance().start(...)` call of `server/src/main.cpp` to
`engine::LogLevel::Debug`. The client does not call `start()`: add this call at the
beginning of its `main()` for the same result.

### Common problems

| Symptom | Likely cause |
|---------|--------------|
| "The server runs another version of the game" | The two programs were not built from the same code: rebuild both after a protocol change. |
| An entity is invisible | The client has no `lookOf()` case for it, or the entity has no `NetworkId`, `EntityType` or `Transform`, so it is not in the snapshots. |
| A system seems to do nothing | It is not registered in `registerSystems()`, its `when` condition is false, or the entity is missing one of its components. |
| An entity reappears after being destroyed | It was recreated by a snapshot: the client must receive `EntityDestroyed`, and ids are never reused. |
| A problem only appears for remote players | Reproduce it with the [network testing guide](NETWORK_TESTING.md). |

### Tests

```bash
./rtype.py --test
```

To run only some of them, after a first build:

```bash
./build/r-type_tests --gtest_filter='ReliableChannel.*'
```

The CI also runs clang-tidy with warnings as errors, so run `./rtype.py --tidy`
before a pull request that changes a lot of code.

## 5. Add a sound

The example adds an alert when the boss arrives. See the
[audio architecture](ARCHITECTURE.md#44-audio) for how sounds are played.

**1. Name it** in `client/include/audio/SoundEvent.hpp`, in the `SoundEvent` enum and
its Doxygen comment:

```cpp
BossAlert,
```

**2. Give it a file** in the `SoundFiles` table of `client/src/audio/SoundBoard.cpp`.
The order does not matter, and the build fails if a sound has no file or two:

```cpp
SoundFile{SoundEvent::BossAlert, "assets/sounds/boss_alert.wav"},
```

**3. Add the file** to `client/assets/sounds/`, in WAV. Either describe it in
`effects()` in `tools/generate_sounds.py` and run the script, or add your own file
and write where it comes from and its license in `client/assets/AUDIO_SOURCES.txt`.
CMake copies the assets next to the client when it builds.

**4. Play it** where it happens, with the `SoundBoard` the system received:

```cpp
sounds.play(audio::SoundEvent::BossAlert);
```

Everything the server tells the client can trigger a sound, so a new sound does not
need a new packet. A music works the same way, with `MusicTrack` and `MusicFiles`,
and `sounds.setMusic()` to play it.

**5. Do not rely on it alone**: a player who cannot hear must still get the
information, so make sure something on screen shows the same thing.

## 6. Add a menu page

The example adds a Records page to the main menu. See the
[menu architecture](ARCHITECTURE.md#45-menus) for how pages and widgets work.

**1. Name the page** in `client/include/MenuPages.hpp`, in the `MenuPage` enum and
its Doxygen comment, and give it a cursor in `MenuState`:

```cpp
enum class MenuPage : std::uint8_t { /* ... */ Records };

struct MenuState {
    // ...
    MenuCursor recordsCursor;
};
```

**2. Write the page** in `client/src/MenuPages.cpp` and declare it in the header. A
page draws with a `MenuPanel` and returns what the player did, so it stays testable.
Here, `RecordLines` is a table of lines to write beside `CreditsLines`:

```cpp
bool recordsPage(const MenuContext &context, MenuCursor &cursor)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;

    const float top = drawMenuTitle(graphics, "RECORDS");
    const float y = drawLines(graphics, RecordLines, top);
    return panel.backButton(menuRow(graphics.screenSize(), y + MenuLineHeight),
                            "Back");
}
```

**3. Add its button**: a value in the `MainChoice` enum and a line in `MainEntries`,
in the order of the menu:

```cpp
MainEntry{.label = "Records", .choice = MainChoice::Records},
```

**4. Connect it** in `client/src/Game.cpp`, in `Game::applyMainChoice()` and
`Game::drawMenu()`:

```cpp
case MainChoice::Records:
    m_menu.page = MenuPage::Records;
    break;
```

```cpp
case MenuPage::Records:
    if (recordsPage(context, m_menu.recordsCursor)) {
        m_menu.page = MenuPage::Main;
    }
    break;
```

Escape already goes back from any page but the main menu, and the keyboard, the mouse
and the sounds come with the widgets. Test the page in `tests/src/menu_test.cpp`,
which runs the menus on a fake graphics backend.

To add a widget, add a method to `MenuPanel` in `client/src/Menu.cpp`. It must call
`nextIsSelected()` exactly once, so that the keyboard selection counts it. Pages always
use the panel: a button drawn by hand would escape the keyboard selection.

## 7. Add a setting

Settings live in the `Settings` struct (`client/include/Settings.hpp`), which only
holds data. They are saved in `settings.cfg`.

### A setting that is on or off

**1. Add the field** to `Settings`, with its default value and its Doxygen comment:

```cpp
bool reduceFlashing = false;
```

**2. Add a line** to the `Toggles` table. The settings file and the Accessibility
page both read this table, so it needs no other code there:

```cpp
ToggleDef{.key = "reduce_flashing",
          .label = "Reduce flashing",
          .field = &Settings::reduceFlashing},
```

**3. Use it** in `Game`, from `m_settings`. A system that only runs when the option
is on can use the `.when` condition of its `SystemOptions`, like the player labels.

### Any other setting

A volume is the model for a value that is not on or off:

1. Add the field to `Settings`.
2. Read it in `readLine()` and write it in `writeSettings()`, in
   `client/src/Settings.cpp`. A bad value must return an error and keep the old one.
3. Add its widget to a page, such as a `panel.slider()` in `audioTab()` in
   `client/src/MenuPages.cpp`. Set `changed` in the result, so that `Game` saves the
   file.
4. Apply it in `Game`, as `applyVolumes()` does for the volumes.

Test the file in `tests/src/settings_test.cpp`, and describe the option in the
[README](https://github.com/T3a1/RType-mirror#readme) if players use it.
