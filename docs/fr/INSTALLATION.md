# Installation

Ce guide explique comment compiler et lancer R-Type sous Linux, macOS et Windows, avec le script
`rtype.py` (recommandé) ou à la main avec Conan et CMake.

## Sommaire

- [1. Prérequis](#1-prérequis)
- [2. Démarrage rapide](#2-démarrage-rapide)
- [3. Le launcher : rtype.py](#3-le-launcher--rtypepy)
- [4. Compiler à la main](#4-compiler-à-la-main)
- [5. Lancer les programmes](#5-lancer-les-programmes)
- [6. Outils de développement](#6-outils-de-développement)
- [7. Dépannage](#7-dépannage)

## 1. Prérequis

| Outil | Sert à | Remarques |
|-------|--------|-----------|
| Git | Récupérer les sources | |
| Python 3.8 ou plus récent | `rtype.py` | `python3` sous Linux et macOS, `python` sous Windows. Bibliothèque standard uniquement. |
| Compilateur C++20 | Compiler | Voir le tableau ci-dessous. |
| CMake 3.15+ et Conan 2 | Compiler | Installés automatiquement par `rtype.py` s'ils manquent. |
| Docker | Lancer le serveur dans un conteneur (mode par défaut) | Facultatif : utilisez `--native` pour s'en passer. |
| clang-tidy | `--tidy`, `--fix` | Facultatif. |
| Doxygen, Graphviz | Générer le site de documentation | Facultatif. |
| tkinter | Le launcher graphique, `--gui` | Facultatif. Fait partie de Python, mais empaqueté à part sous Linux (`sudo apt install python3-tk`, `sudo dnf install python3-tkinter`, `sudo pacman -S tk`) et avec le Python de Homebrew (`brew install python-tk`). Le Python de python.org l'inclut déjà sous Windows et macOS. |
| ffmpeg | `tools/generate_sounds.py`, pour régénérer les sons | Facultatif : les fichiers générés sont dans le dépôt. |

Ce qu'il faut installer à la main sur chaque système (tout le reste est installé par `rtype.py`) :

| Système | Compilateur | Docker | clang-tidy | Doxygen, Graphviz |
|---------|-------------|--------|------------|-------------------|
| Debian, Ubuntu | `sudo apt install build-essential` | Docker Engine | `sudo apt install clang-tidy` | `sudo apt install doxygen graphviz` |
| Fedora | `sudo dnf install gcc-c++` | Docker Engine | `sudo dnf install clang-tools-extra` | `sudo dnf install doxygen graphviz` |
| Arch | `sudo pacman -S base-devel` | Docker Engine | `sudo pacman -S clang` | `sudo pacman -S doxygen graphviz` |
| macOS (Intel ou Apple Silicon) | `xcode-select --install` | Docker Desktop | `brew install llvm` | `brew install doxygen graphviz` |
| Windows 10/11 | Installé par `rtype.py` (MSVC, avec winget) | Docker Desktop (conteneurs Linux) | `winget install LLVM.LLVM` | `winget install DimitriVanHeesch.Doxygen Graphviz.Graphviz` |

Quand il manque quelque chose, `rtype.py` s'arrête avec un message qui dit quoi installer.

Plateformes prises en charge :

- **Linux** : exigé par le projet, et testé en CI sous Ubuntu 24.04.
- **macOS** : pris en charge par `rtype.py` (testé à la main sur Apple Silicon, pas en CI).
- **Windows 10/11** : pris en charge par `rtype.py`. Le jeu croisé avec Linux est encore en discussion.

Toutes les bibliothèques tierces (Raylib, SFML ou ncurses, Asio, nlohmann-json, GoogleTest) viennent du
gestionnaire de paquets Conan. Rien n'est copié dans le dépôt, et aucune bibliothèque n'a besoin d'être
installée sur tout le système.

## 2. Démarrage rapide

```bash
git clone https://github.com/T3a1/RType-mirror.git rtype
cd rtype
./rtype.py              # on Windows: python rtype.py
```

Sous Windows, remplacez `./rtype.py` par `python rtype.py` dans toutes les commandes de ce guide.

Le premier lancement prend quelques minutes : il installe les outils de build manquants, télécharge les
bibliothèques, compile le client, construit l'image Docker du serveur, puis lance le serveur et un client.

Pour jouer sans Docker, ou entre amis sur la même machine :

```bash
./rtype.py --native          # server built and run on your machine
./rtype.py --native -c 2     # same, with two clients
./rtype.py --native --room 3 # same, directly in room 3, without the lobby
./rtype.py --graphics sfml   # client built with SFML instead of Raylib
./rtype.py --native -g ncurses   # client drawn in this terminal (not on Windows)
```

Pour jouer sur plusieurs machines, lancez le serveur seul sur l'une d'elles, puis uniquement les clients
sur les autres :

```bash
./rtype.py --server-only --native         # on the host: no client, runs until quit or Ctrl+C
./rtype.py --clients-only -H 192.168.1.20 # on each player's machine: the host's address
./rtype.py --clients-only -H 192.168.1.20 -c 2   # two players on the same machine
```

Le serveur écoute sur toutes les adresses réseau de la machine. Son pare-feu doit laisser entrer le port
UDP 4242 (ou le port donné avec `-p`, le même des deux côtés).

## 3. Le launcher : rtype.py

`rtype.py` fonctionne depuis n'importe quel dossier et sur tous les systèmes : il n'utilise que la
bibliothèque standard de Python et ne passe jamais par un shell. `./rtype.py --help` liste les options.
Avant de compiler, il met en place ce qui manque :

- `cmake`, `conan` et `gcovr` sont installés avec pip dans un environnement virtuel local, `.venv/` (sans
  droits d'administrateur, ignoré par Git).
- Un profil Conan par défaut est créé à partir de votre compilateur si vous n'en avez pas.
- Sous Linux, Conan installe avec votre gestionnaire de paquets (apt, dnf, pacman...) les paquets de
  développement X11 et OpenGL dont la bibliothèque graphique a besoin. Il ne demande votre mot de passe
  `sudo` que si l'un d'eux manque. macOS et Windows n'ont besoin d'aucun paquet système.
- Sous Windows, si aucun compilateur MSVC n'est trouvé, il installe Visual Studio 2022 Build Tools avec la
  charge de travail C++ par `winget` (plusieurs Go, demande les droits d'administrateur). Sous Linux et
  macOS, il s'arrête et indique comment installer un compilateur (voir le tableau ci-dessus).

### Options

| Option | Effet |
|--------|-------|
| *(aucune)* | Lance le serveur dans Docker et un client. |
| `-G`, `--gui` | Ouvre le [launcher graphique](#launcher-graphique). |
| `-S`, `--server-only` | Lance seulement le serveur, dans Docker ou avec `--native`, jusqu'à Ctrl+C (ou `quit` dans sa console). Affiche la commande à lancer pour les joueurs. Refuse les options du client (`-c`, `-r`, `-H`). |
| `-O`, `--clients-only` | Lance seulement les clients, connectés à un serveur déjà lancé à `-H` et `-p`. Refuse les options du serveur (`-n`, `--banlist`, `--whitelist`). |
| `-n`, `--native` | Lance le serveur nativement au lieu de Docker. |
| `-c N`, `--clients N` | Lance `N` clients (1 par défaut). |
| `-r N`, `--room N` | Les clients entrent directement dans la salle `N`, créée au besoin (par défaut, ils arrivent dans le lobby, où ils choisissent une salle). Un serveur fait tourner jusqu'à 16 parties indépendantes. |
| `-p PORT`, `--port PORT` | Le port UDP utilisé par les clients (4242 par défaut). |
| `-H HOST`, `--host HOST` | L'adresse à laquelle les clients se connectent (127.0.0.1 par défaut). |
| `-g LIB`, `--graphics LIB` | La bibliothèque graphique du client : `raylib` (par défaut), `sfml` ou `ncurses` (voir [Client terminal](#client-terminal-ncurses)). La changer reconfigure et recompile `build/`. |
| `-T`, `--test` | Compile et lance les tests unitaires, puis s'arrête. |
| `-C`, `--coverage` | Lance les tests unitaires avec la couverture et écrit un rapport HTML. |
| `-t`, `--tidy` | Lance clang-tidy sur le projet. |
| `-f`, `--fix` | Applique les corrections de clang-tidy, puis lance clang-format. |
| `-F`, `--format` | Lance clang-format sur le projet (même version que la CI). |
| `--check-format [FILE...]` | Vérifie le format comme la CI, sans rien changer. |
| `-d`, `--docs` | Génère le site de documentation avec Doxygen : les pages de `docs/` et `docs/fr/`, et la référence du code. |

> En mode Docker, `--port` change le port publié sur votre machine ; dans le conteneur, le serveur reste
> sur 4242.

Le client (et le serveur avec `--native`) est recompilé depuis les sources à chaque lancement, ce qui
prend une seconde quand rien n'a changé. Le build est reconfiguré quand `build/` est nouveau, quand
`conanfile.py` a changé, ou quand `--graphics` désigne une autre bibliothèque. Le mode Docker compile le
serveur dans l'image, jamais dans `build/`.

### Launcher graphique

`./rtype.py --gui` ouvre une fenêtre avec les options de jeu de la ligne de commande : quoi lancer (serveur
et clients, serveur seul ou clients seuls), serveur Docker ou natif, nombre de clients, adresse et port du
serveur, salle et bibliothèque graphique.

- La fenêtre affiche la **commande** qui correspond à ses réglages, et grise les options que le mode choisi
  n'utilise pas. Tout ce qu'elle fait peut aussi se taper dans un terminal.
- **Start** lance cette commande et affiche sa sortie dans la fenêtre. **Stop** l'arrête comme Ctrl+C : le
  serveur et les clients qu'elle a lancés s'arrêtent aussi. Fermer la fenêtre l'arrête également.
- **Server console** : une ligne tapée là va à la console d'administration du serveur, Docker ou natif
  (`help`, `status`, `kick`, `ban`...).
- **Sur un écran de très haute résolution**, la fenêtre s'agrandit d'elle-même selon la résolution et le DPI
  de l'écran. Si le résultat ne vous convient pas, réglez `RTYPE_GUI_SCALE`, par exemple
  `RTYPE_GUI_SCALE=2 ./rtype.py --gui`.
- Pour **créer une partie**, lancez le serveur et les clients avec un numéro de salle, ou la salle 0 pour en
  choisir une dans le lobby. Pour **rejoindre une partie** sur une autre machine, choisissez « Clients only »
  avec son adresse. Plusieurs salles sont plusieurs parties jouées en parallèle par le même serveur.

Sans tkinter, `--gui` indique comment l'installer (voir les [prérequis](#1-prérequis)) ; la ligne de commande
fait les mêmes choses.

### Variables d'environnement

| Variable | Défaut | Signification |
|----------|--------|---------------|
| `BUILD_DIR` | `build` | Le dossier de build. |
| `COVERAGE_DIR` | `build-coverage` | Le dossier de build de `--coverage`. |
| `SERVER_BIN`, `CLIENT_BIN` | *(trouvés dans `BUILD_DIR`)* | Les chemins de binaires existants. |
| `IMAGE`, `CONTAINER` | `rtype-server` | Les noms de l'image et du conteneur Docker. |
| `RTYPE_GUI_SCALE` | *(selon l'écran)* | L'agrandissement de la fenêtre du launcher graphique, de `0.5` à `4`. |
| `GRAPHICS` | `raylib` | La valeur par défaut de `--graphics`. |
| `CONAN_SYSTEM_PACKAGES` | `install` | Linux uniquement : `check` vérifie seulement les paquets X11/OpenGL, `report` les liste seulement. |
| `TOOLS_DIR` | `.venv` | L'environnement virtuel des outils de build. |
| `NO_COLOR` | *(non définie)* | Désactive les couleurs de la sortie. |

## 4. Compiler à la main

Depuis la racine du dépôt :

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Release
cmake -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Les exécutables `r-type_server` et `r-type_client` sont écrits dans `build/` (`build/Release/` avec Visual
Studio).

Le client utilise Raylib par défaut. Pour le compiler avec SFML (ou ncurses), choisissez la bibliothèque pour
Conan, qui l'installe et transmet le choix à CMake. Sur un dossier de build déjà configuré, passez-la aussi à
CMake :

```bash
conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -s build_type=Release -o "&:graphics=sfml"
cmake -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DRTYPE_GRAPHICS_BACKEND=sfml
```

Options de CMake :

| Option | Défaut | Effet |
|--------|--------|-------|
| `BUILD_CLIENT` | `ON` | Compile le client. `OFF` ne compile que le serveur (il a quand même besoin de Conan pour Asio). |
| `RTYPE_GRAPHICS_BACKEND` | `raylib` | La bibliothèque graphique du client : `raylib`, `sfml` ou `ncurses`. Elle doit correspondre à l'option Conan `graphics`. |
| `RTYPE_NETWORK_BACKEND` | `asio` | La bibliothèque réseau derrière `engine::net::IUdpSocket`. Seul `asio` existe. |
| `BUILD_TESTS` | `OFF` | Compile les tests unitaires (`r-type_tests`). |
| `ENABLE_COVERAGE` | `OFF` | Instrumente le code pour la couverture de tests (GCC/Clang). |

## 5. Lancer les programmes

### Serveur

```bash
./build/r-type_server
```

Le serveur écoute sur le port UDP 4242, ou sur le port donné en premier argument
(`./build/r-type_server 5000`), et s'arrête proprement avec Ctrl+C.

Pour le lancer dans Docker sans le launcher :

```bash
docker build -t rtype-server .
docker run -dit --rm --init --name rtype-server -p 4242:4242/udp rtype-server
docker logs -f rtype-server     # follow its output
docker attach rtype-server      # type administration commands (leave: Ctrl+P, Ctrl+Q)
docker stop rtype-server        # stop and remove it
```

#### Console d'administration

Tapez ces commandes dans le terminal du serveur, pendant qu'il tourne. Avec `rtype.py`, sous Docker comme en
natif, c'est le terminal depuis lequel il a été lancé (ou la **Server console** du launcher graphique). À côté
d'un client ncurses, ou pour un serveur lancé à la main dans Docker, le conteneur est détaché : utilisez
`docker attach rtype-server`.

| Commande | Effet |
|----------|-------|
| `help` | Liste les commandes. |
| `status` | Temps de fonctionnement, nombre de salles et de clients, règles d'accès. |
| `rooms` | Liste les salles, leur état et leurs joueurs. |
| `players [room]` | Liste les clients : identifiant, adresse, salle, dernier paquet. |
| `kick <id>` | Déconnecte un client. |
| `ban <id or ip>` | Bannit une adresse et déconnecte ses clients. |
| `unban <ip>`, `banlist` | Retire un bannissement, liste les bannissements. |
| `stop-room <room>` | Ferme une salle ; ses joueurs retournent au lobby. |
| `whitelist on`, `off` | N'accepte que les adresses listées, ou tout le monde. |
| `whitelist add <ip>`, `remove <ip>`, `list` | Modifie ou affiche les adresses listées. |
| `quit` | Arrête le serveur. |

Les bannissements et les listes blanches utilisent des adresses IPv4, et peuvent être gardés dans des fichiers
texte, une adresse par ligne (`#` commence un commentaire). Ils sont lus au démarrage et réécrits à chaque
changement :

```bash
./build/r-type_server --banlist bans.txt --whitelist allowed.txt
```

Il n'y a pas de comptes joueurs, donc ces listes ne peuvent nommer que des adresses : un bannissement touche
tous ceux qui partagent la même adresse, et un joueur qui change d'adresse le contourne.

`--whitelist` active aussi la liste blanche. `--no-console` ne lit pas l'entrée standard, pour un serveur lancé
par un script. Le launcher a les mêmes options `--banlist` et `--whitelist`, avec `--native` uniquement.

L'image contient un seul binaire statique et tourne avec un utilisateur sans privilèges.

### Client

```bash
./build/r-type_client [host] [port]
```

`host` vaut `127.0.0.1` par défaut et `port` vaut `4242`. Pour rejoindre un serveur sur une autre machine,
passez son adresse IP et vérifiez que le port UDP 4242 est ouvert sur le pare-feu de cette machine.

Les clients Raylib et SFML jouent des effets sonores et de la musique. Le client ncurses n'a pas de son.

### Client terminal (ncurses)

Compilé avec `--graphics ncurses`, le client dessine le jeu dans le terminal depuis lequel il a été lancé. Cela
montre que le client ne dépend pas de sa bibliothèque graphique, mais le jeu est dégradé :

- Non disponible sous Windows ; testé sous Linux. Un grand terminal (environ 160x45) donne le meilleur résultat.
- Les vaisseaux, les missiles et les ennemis sont des blocs de caractères colorés, et il n'y a pas d'image de
  fond. Les couleurs sont arrondies à la palette du terminal (256 couleurs, ou 8).
- Un terminal n'envoie que les appuis de touches et leurs répétitions, jamais les relâchements. Une touche compte
  comme maintenue environ une demi-seconde après son appui, puis tant qu'elle se répète. Une seule touche
  fonctionne à la fois : impossible de bouger et de tirer ensemble.
- Le chat de la salle s'affiche, mais on ne peut pas y écrire : `q` quitte le jeu dans ce client, donc les touches ne
  peuvent pas servir à la saisie.
- Quittez avec Échap, `q` ou Ctrl+C.
- Les journaux vont dans `r-type_client.log` dans le dossier courant au lieu du terminal. Avec `--native`,
  `rtype.py` écrit la sortie du serveur dans `build/server.log`, et ne lance qu'un client (lancez les autres
  depuis d'autres terminaux avec `./build/r-type_client`).
- Les descriptions de terminal viennent du système (`/usr/share/terminfo`...), sauf si `TERMINFO` ou
  `TERMINFO_DIRS` est défini.

## 6. Outils de développement

| Commande | Résultat |
|----------|----------|
| `./rtype.py --test` | Tests unitaires (GoogleTest). Équivaut à `cmake -B build -DBUILD_TESTS=ON`, compiler, puis `ctest --test-dir build`. |
| `./rtype.py --coverage` | Rapport de couverture dans `build-coverage/coverage/index.html`. |
| `./rtype.py --tidy` | clang-tidy, avec les mêmes règles que la CI. |
| `./rtype.py --fix` | Corrections de clang-tidy, puis clang-format. |
| `./rtype.py --format` | clang-format seul. |
| `./rtype.py --docs` | Site de documentation dans `build/docs/html/index.html`, la version française sous `fr/`. Un lien cassé entre les pages, ou une page sans version française, est une erreur. |
| `./tools/generate_sounds.py` | Régénère les effets sonores et les musiques, dans `client/assets/`. |

Chaque version de clang-format formate un peu différemment : le launcher, le hook pre-commit et la CI utilisent
donc tous clang-format 18.1.3, installé depuis PyPI dans `.venv/`. Le clang-format de votre système n'est jamais
utilisé.

Le premier lancement de `rtype.py` active les hooks Git du projet (`git config core.hooksPath .githooks`). Le hook
pre-commit refuse un commit dont les fichiers C++ sont mal formatés : lancez `./rtype.py --format`, refaites
`git add` sur les fichiers et commitez. Utilisez `git commit --no-verify` pour l'ignorer une fois.

Sous Windows, le hook tourne dans le shell de Git et utilise le premier de `python3`, `python` et `py` qui
fonctionne vraiment (il ignore le raccourci du Microsoft Store).

`--tidy` et `--fix` ont besoin de clang-tidy (voir le tableau de la [section 1](#1-prérequis)). Le `llvm` de Homebrew
et l'installateur LLVM de Windows ne l'ajoutent pas toujours au `PATH` : `rtype.py` le cherche aussi dans
`/opt/homebrew/opt/llvm/bin`, `/usr/local/opt/llvm/bin` et `C:\Program Files\LLVM\bin`. clang-tidy lit
`build/compile_commands.json`, que le générateur Visual Studio n'écrit pas : sous Windows, configurez `build/` avec
Ninja pour l'utiliser. `--coverage` a besoin de GCC ou de Clang (gcov) et ne fonctionne donc pas avec MSVC.

`--docs` a besoin de Doxygen, en version 1.18 pour dessiner les diagrammes Mermaid des pages (la CI utilise cette
version). Installez aussi Graphviz pour avoir les diagrammes de classes ; sans lui, la documentation est générée
sans eux.

## 7. Dépannage

**CMake ne trouve pas raylib.** Le dossier de build a été configuré sans la chaîne d'outils Conan. Supprimez `build/`,
ou laissez `rtype.py` le reconfigurer (il le fait automatiquement).

**CMake ne trouve pas SFML, ncurses ou raylib après un changement de bibliothèque.** Conan a installé l'autre
bibliothèque : `RTYPE_GRAPHICS_BACKEND` et l'option Conan `graphics` doivent nommer la même. Relancez `conan install`
avec `-o "&:graphics=..."`, ou utilisez `./rtype.py --graphics ...`, qui les garde synchronisés.

**`conan install` a échoué.** La sortie complète est dans `build/conan-install.log`. Sous Linux, la cause habituelle est
un paquet X11/OpenGL manquant qui n'a pas pu être installé : lancez `CONAN_SYSTEM_PACKAGES=report ./rtype.py` pour les
lister.

**`Docker is installed but not running`.** Démarrez Docker Desktop (macOS, Windows) ou le service Docker
(`sudo systemctl start docker` sous Linux), ou utilisez `--native`.

**`your user cannot use Docker`.** Sous Linux, votre utilisateur n'est pas dans le groupe `docker`. Ajoutez-le
(`sudo usermod -aG docker $USER`, puis reconnectez-vous) ou utilisez `--native`.

**`no C++ compiler`.** Installez-en un comme indiqué à la [section 1](#1-prérequis), puis relancez `rtype.py`. Sous macOS,
`rtype.py` vérifie la présence des Xcode Command Line Tools.

**Le client ne se connecte pas.** Vérifiez que le serveur tourne (`docker logs rtype-server` en mode Docker), que l'hôte
et le port correspondent, et qu'aucun pare-feu ne bloque le port UDP 4242.

**Pas de son.** Sans périphérique audio, le client affiche `No audio device: the game runs without sound` et le jeu tourne
normalement. Si un fichier ne peut pas être lu, il affiche `Cannot load the sound` ou `Cannot load the music` avec son nom :
recompilez le client pour copier `client/assets/` à côté de lui. Le client ncurses ne joue jamais de son, et `--bench` coupe
le son du jeu.

**Windows : MSVC toujours introuvable.** Ouvrez Visual Studio Installer et ajoutez la charge de travail « Desktop development
with C++ » à Build Tools 2022.
