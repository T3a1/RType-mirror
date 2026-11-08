# Guides pratiques

Des recettes pour les changements les plus courants. Lisez d'abord
l'[architecture](ARCHITECTURE.md) si vous ne savez pas ce qu'est un ECS. Pour du contenu qui
ne demande pas de C++, voir [écrire des niveaux](LEVELS.md) et
[armes et bonus](WEAPONS.md).

## Sommaire

1. [Ajouter un composant et un système](#1-ajouter-un-composant-et-un-système)
2. [Ajouter un type d'ennemi](#2-ajouter-un-type-dennemi)
3. [Ajouter un type de paquet](#3-ajouter-un-type-de-paquet)
4. [Compiler et déboguer](#4-compiler-et-déboguer)
5. [Ajouter un son](#5-ajouter-un-son)
6. [Ajouter une page de menu](#6-ajouter-une-page-de-menu)
7. [Ajouter un réglage](#7-ajouter-un-réglage)

## 1. Ajouter un composant et un système

Cet exemple fait regagner de la vie à certaines entités au fil du temps, sur le serveur.

**1. Choisissez où il va.**

| Le code est utilisé par | Composant | Système |
|-------------------------|-----------|---------|
| Le serveur seul | `server/include/components/` | `server/include/systems/`, `server/src/systems/` |
| Le client seul | `client/include/components/` | `client/include/systems/`, `client/src/systems/` |
| Les deux | `common/include/common/components/` | `common/include/common/systems/`, `common/src/systems/` |

**2. Écrivez le composant** : des données simples, sans logique, dans
`server/include/components/Regeneration.hpp` :

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

**3. Écrivez le système**, une fonction qui prend le registre et le temps écoulé, dans
`server/src/systems/RegenerationSystem.cpp`, avec son en-tête dans
`server/include/systems/` :

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

**4. Ajoutez le fichier source à CMake** : `SOURCE_FILES` dans `server/CMakeLists.txt`, ou
les sources de la bibliothèque `r-type_server_spawn` si un test en a besoin (les tests ne
peuvent pas se lier à l'exécutable).

**5. Enregistrez-le** dans `Game::registerSystems()`, dans `server/src/Game.cpp`. L'étape et
l'ordre décident du moment où il tourne :

```cpp
m_scheduler.add(Stage::Simulation, "regeneration", server::regenerationSystem,
                playing);
```

`playing` l'arrête tant que la salle attend. Un système qui a besoin de plus que le registre
et le temps est enveloppé dans une lambda, comme les autres de cette fonction.

**6. Donnez le composant aux entités** : en général dans `EntityFactory::create()`
(`server/src/EntityFactory.cpp`), avec
`registry.emplace<Regeneration>(entity, Regeneration{.perSecond = 2.0F});`.

**7. Testez-le**, sans fenêtre ni socket : créez un registre, ajoutez une entité, lancez le
système, vérifiez les composants. Ajoutez le nouveau fichier à `SOURCE_FILES` dans
`tests/CMakeLists.txt` :

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

**Seul le serveur connaît ce composant.** Pour l'afficher sur les clients, il doit aussi
passer dans un paquet ([section 3](#3-ajouter-un-type-de-paquet)).

## 2. Ajouter un type d'ennemi

**La plupart des nouveaux ennemis ne demandent pas de code.** Un type d'ennemi ou de boss qui
réutilise le comportement et le sprite d'un type intégré (avec d'autres statistiques, une
autre taille, trajectoire ou un autre score) tient en quelques lignes de
[`server/enemies.json`](ENEMIES.md), puis son nom dans un fichier de niveau. N'écrivez du
code que pour un **nouveau sprite** ou un **nouveau comportement**.

Un nouveau sprite demande une nouvelle valeur des deux côtés, donc les clients et le serveur
doivent être recompilés ensemble. Voici ce que change chaque étape, pour un ennemi appelé
`Turret` :

| Étape | Fichier | À faire |
|-------|---------|---------|
| 1 | `common/include/common/components/Enemy.hpp` | Ajoutez `Turret` à `EnemyType`, avant `Boss` |
| 2 | `common/include/common/components/EntityType.hpp` | Ajoutez `EnemyTurret` à la **fin** de `EntityType` : les valeurs passent sur le réseau |
| 3 | `common/src/protocol/Messages.cpp` | Si c'est la nouvelle dernière valeur, changez la borne de `readEnum(in, EntityType::..., entity.type)`, sinon clients et serveur la rejettent |
| 4 | `server/include/EnemyDefs.hpp` | Ajoutez ses statistiques par défaut à `ENEMY_DEFS`, **dans l'ordre de `EnemyType`**, et changez la taille du tableau |
| 5 | `server/src/EnemyCatalog.cpp` | Ajoutez `"turret"` à `BuiltinNames`, dans le même ordre, pour que `enemies.json` et les fichiers de niveau puissent l'utiliser |
| 6 | `client/src/SpriteSheets.cpp` | Ajoutez ses images et un `case` dans `lookOf()`. Une nouvelle image demande aussi une valeur dans `Sheet` (`client/include/SpriteSheets.hpp`) et une ligne dans la liste des fichiers, dans le même ordre |
| 7 | `common/include/common/protocol/PacketHeader.hpp` | Ajoutez 1 à `ProtocolVersion` |
| 8 | `docs/PROTOCOL.md` | Ajoutez le type d'entité, la version dans le tableau de l'en-tête et une ligne dans la [table des versions](PROTOCOL.md#12-gestion-des-versions) |
| 9 | `docs/ENEMIES.md` | Ajoutez-le aux types intégrés et aux tailles des apparences |

Le collider dans `ENEMY_DEFS` doit correspondre à la taille que dessine le client, sinon les
tirs sembleront le rater. Utilisez-le ensuite dans un niveau ([écrire des niveaux](LEVELS.md)) :

```json
{ "time": 2, "enemy": "turret", "count": 2, "stepY": 300, "y": 540 }
```

Une fois qu'il existe, `enemies.json` peut aussi en dériver de nouveaux types (`"base":
"turret"`) sans code.

## 3. Ajouter un type de paquet

Pour un paquet appelé `Ping`, envoyé par les clients, avec une valeur de 32 bits :

**1. Déclarez le type** dans `common/include/common/protocol/PacketType.hpp` : une valeur dans
`PacketType`, une entrée dans `AllPacketTypes`, un `case` dans `canSend()` (qui peut l'envoyer)
et un `case` dans `isReliable()`. Le compilateur signale un `case` manquant.

**2. Déclarez la charge utile** dans `common/include/common/protocol/Messages.hpp` :

```cpp
struct Ping {
    static constexpr PacketType Type = PacketType::Ping;
    std::uint32_t time = 0;
    bool operator==(const Ping &) const = default;
};
```

ainsi que les déclarations de `encode()` et `decode()` à côté des autres.

**3. Écrivez-la et lisez-la** dans `common/src/protocol/Messages.cpp`, champ par champ et
jamais avec un `memcpy`. `decode()` renvoie `false` pour une valeur hors limites :

```cpp
void encode(const Ping &message, ByteWriter &out) { out.u32(message.time); }

bool decode(ByteReader &in, Ping &message)
{
    message.time = in.u32();
    return true;
}
```

**4. Ajoutez-le à la couche paquet** : à la variante `Payload` de
`common/include/common/protocol/Packet.hpp`, et un `case` dans `decodePacket()` de
`common/src/protocol/Packet.cpp` :

```cpp
case PacketType::Ping:
    return decodePayload<Ping>(header, in);
```

**5. Traitez-le là où il arrive** :

- sur le serveur, dans `NetworkServer::handleSessionPacket()`
  (`server/src/network/NetworkServer.cpp`), sur le thread réseau. Si le jeu en a besoin,
  poussez un `InboundMessage` dans la file de la salle, sans jamais toucher au registre ;
- sur le client, dans `NetworkClient::handlePacket()`, qui donne au jeu ce qui le concerne,
  puis dans `networkSyncSystem()` ou le traitement du lobby de `Game`.

**6. Testez-le** : ajoutez un exemple à la liste d'aller-retour de `tests/src/packet_test.cpp`.
Les tests aléatoires de `packet_fuzz_test.cpp` couvrent ensuite ce qu'ils génèrent à partir des
types connus.

**7. Documentez-le** : le type et la charge utile dans [PROTOCOL.md](PROTOCOL.md), et ajoutez 1
à `ProtocolVersion` avec une ligne dans la [table des versions](PROTOCOL.md#12-gestion-des-versions).
Quelqu'un doit pouvoir écrire un client à partir de ce seul document.

## 4. Compiler et déboguer

### Compiler

`./rtype.py` fait tout pour un usage normal ([guide d'installation](INSTALLATION.md)). Pour
déboguer, compilez un dossier séparé avec les symboles de débogage :

```bash
conan install . --output-folder=build-debug --build=missing -s compiler.cppstd=20 -s build_type=Debug
```

```bash
cmake -B build-debug -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
```

```bash
cmake --build build-debug -j8
```

### Lancer sous un débogueur

Lancez le serveur, puis un client dans la même salle dans un second terminal :

```bash
gdb --args ./build-debug/r-type_server --no-console
```

```bash
gdb --args ./build-debug/r-type_client 127.0.0.1 4242 --room 1
```

`--no-console` empêche la console d'administration du serveur de lire le terminal qu'utilise
gdb. `--invincible` sur le serveur garde votre vaisseau en vie pendant que vous regardez
quelque chose. Ne restez pas plus de 5 secondes sur un point d'arrêt si la connexion compte :
l'autre côté déclare un timeout.

### Journaux

Utilisez `BB_DEBUG`, `BB_INFO`, `BB_WARN` et `BB_ERROR` (`engine/Logger.hpp`), pas `std::cout` :
sur le serveur, ils ne font jamais attendre le jeu après le terminal. Le niveau minimal est
`Info`. Pour en voir plus, par exemple pourquoi un datagramme a été refusé, passez à
`engine::LogLevel::Debug` l'appel `engine::Logger::instance().start(...)` de
`server/src/main.cpp`. Le client n'appelle pas `start()` : ajoutez cet appel au début de son
`main()` pour le même résultat.

### Problèmes courants

| Symptôme | Cause probable |
|----------|----------------|
| « The server runs another version of the game » | Les deux programmes ne viennent pas du même code : recompilez les deux après un changement de protocole. |
| Une entité est invisible | Le client n'a pas de `case` pour elle dans `lookOf()`, ou l'entité n'a pas de `NetworkId`, d'`EntityType` ou de `Transform`, et n'est donc pas dans les snapshots. |
| Un système semble ne rien faire | Il n'est pas enregistré dans `registerSystems()`, sa condition `when` est fausse, ou il manque à l'entité un de ses composants. |
| Une entité réapparaît après sa destruction | Un snapshot l'a recréée : le client doit recevoir `EntityDestroyed`, et les identifiants ne sont jamais réutilisés. |
| Un problème n'apparaît que pour les joueurs distants | Reproduisez-le avec le [guide des tests réseau](NETWORK_TESTING.md). |

### Tests

```bash
./rtype.py --test
```

Pour n'en lancer que certains, après une première compilation :

```bash
./build/r-type_tests --gtest_filter='ReliableChannel.*'
```

La CI lance aussi clang-tidy avec les avertissements en erreurs : lancez donc `./rtype.py --tidy`
avant une pull request qui change beaucoup de code.

## 5. Ajouter un son

L'exemple ajoute une alerte à l'arrivée du boss. Voir l'[architecture audio](ARCHITECTURE.md#44-audio)
pour la façon dont les sons sont joués.

**1. Nommez-le** dans `client/include/audio/SoundEvent.hpp`, dans l'enum `SoundEvent` et son
commentaire Doxygen :

```cpp
BossAlert,
```

**2. Donnez-lui un fichier** dans la table `SoundFiles` de `client/src/audio/SoundBoard.cpp`.
L'ordre n'a pas d'importance, et le build échoue si un son n'a aucun fichier ou en a deux :

```cpp
SoundFile{SoundEvent::BossAlert, "assets/sounds/boss_alert.wav"},
```

**3. Ajoutez le fichier** à `client/assets/sounds/`, en WAV. Soit vous le décrivez dans
`effects()` de `tools/generate_sounds.py` et vous lancez le script, soit vous ajoutez votre
propre fichier et vous indiquez sa provenance et sa licence dans
`client/assets/AUDIO_SOURCES.txt`. CMake copie les ressources à côté du client à la
compilation.

**4. Jouez-le** là où ça se passe, avec le `SoundBoard` que le système a reçu :

```cpp
sounds.play(audio::SoundEvent::BossAlert);
```

Tout ce que le serveur dit au client peut déclencher un son : un nouveau son ne demande donc
pas de nouveau paquet. Une musique fonctionne de la même façon, avec `MusicTrack` et
`MusicFiles`, et `sounds.setMusic()` pour la jouer.

**5. Ne vous reposez pas sur lui seul** : un joueur qui n'entend pas doit quand même recevoir
l'information, donc vérifiez que quelque chose à l'écran montre la même chose.

## 6. Ajouter une page de menu

L'exemple ajoute une page Records au menu principal. Voir
l'[architecture des menus](ARCHITECTURE.md#45-menus) pour le fonctionnement des pages et
des widgets.

**1. Nommez la page** dans `client/include/MenuPages.hpp`, dans l'enum `MenuPage` et son
commentaire Doxygen, et donnez-lui un curseur dans `MenuState` :

```cpp
enum class MenuPage : std::uint8_t { /* ... */ Records };

struct MenuState {
    // ...
    MenuCursor recordsCursor;
};
```

**2. Écrivez la page** dans `client/src/MenuPages.cpp` et déclarez-la dans l'en-tête. Une
page dessine avec un `MenuPanel` et renvoie ce qu'a fait le joueur, ce qui la garde
testable. Ici, `RecordLines` est une table de lignes à écrire à côté de `CreditsLines` :

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

**3. Ajoutez son bouton** : une valeur dans l'enum `MainChoice` et une ligne dans
`MainEntries`, dans l'ordre du menu :

```cpp
MainEntry{.label = "Records", .choice = MainChoice::Records},
```

**4. Branchez-la** dans `client/src/Game.cpp`, dans `Game::applyMainChoice()` et
`Game::drawMenu()` :

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

Échap revient déjà en arrière depuis toute page autre que le menu principal, et le
clavier, la manette, la souris et les sons viennent avec les widgets. Testez la page dans
`tests/src/menu_test.cpp`, qui fait tourner les menus sur un faux client graphique.

Pour ajouter un widget, ajoutez une méthode à `MenuPanel` dans `client/src/Menu.cpp`.
Elle doit appeler `nextIsSelected()` exactement une fois, pour que la sélection au clavier
le compte. Les pages passent toujours par le panneau : un bouton dessiné à la main
échapperait à la sélection au clavier.

## 7. Ajouter un réglage

Les réglages sont dans la structure `Settings` (`client/include/Settings.hpp`), qui ne
contient que des données. Ils sont enregistrés dans `settings.cfg`.

### Un réglage activé ou désactivé

**1. Ajoutez le champ** à `Settings`, avec sa valeur par défaut et son commentaire
Doxygen :

```cpp
bool reduceFlashing = false;
```

**2. Ajoutez une ligne** à la table `Toggles`. Le fichier des réglages et la page
Accessibility lisent tous deux cette table, donc rien d'autre n'est à écrire de ce côté :

```cpp
ToggleDef{.key = "reduce_flashing",
          .label = "Reduce flashing",
          .field = &Settings::reduceFlashing},
```

**3. Utilisez-le** dans `Game`, depuis `m_settings`. Un système qui ne tourne que lorsque
l'option est active peut utiliser la condition `.when` de ses `SystemOptions`, comme les
étiquettes des joueurs.

### Tout autre réglage

Un volume sert de modèle pour une valeur qui n'est pas activée ou désactivée :

1. Ajoutez le champ à `Settings`.
2. Lisez-le dans `readLine()` et écrivez-le dans `writeSettings()`, dans
   `client/src/Settings.cpp`. Une valeur incorrecte doit renvoyer une erreur et garder
   l'ancienne.
3. Ajoutez son widget à une page, par exemple un `panel.slider()` dans `audioTab()` de
   `client/src/MenuPages.cpp`. Mettez `changed` dans le résultat, pour que `Game`
   enregistre le fichier.
4. Appliquez-le dans `Game`, comme le fait `applyVolumes()` pour les volumes.

Testez le fichier dans `tests/src/settings_test.cpp`, et décrivez l'option dans le
[README](README.md) si les joueurs l'utilisent.
