# Étude technique et comparative

Cette page explique pourquoi R-Type est construit ainsi : pour chaque technologie, ce que nous
avons choisi, ce à quoi nous l'avons comparé et ce que ce choix nous coûte. Le fonctionnement du
code se trouve dans l'[architecture](ARCHITECTURE.md) ; cette page traite du pourquoi.

Les chiffres sur notre projet viennent du code ou de notre propre
[benchmark réseau](NETWORK_COMPARISON.md). Les comparaisons avec d'autres technologies sont
qualitatives : nous n'avons pas écrit le jeu une fois par alternative.

## Résumé

| Sujet | Choix | Principales alternatives | Raison décisive |
|-------|-------|--------------------------|-----------------|
| Langage | C++20 | Rust, C#, Go | Exigé par le sujet ; C++20 simplifie le code |
| Graphisme, entrées | Raylib, derrière une interface | SFML, SDL | L'API la plus simple pour nos besoins ; l'interface a gardé le choix ouvert |
| Bibliothèque réseau | Asio, derrière une interface | Sockets bruts, ENet | Des sockets portables sans protocole imposé |
| Transport | UDP avec notre propre fiabilité | TCP, QUIC | Un état de jeu en retard ne sert à rien : ne jamais attendre un paquet perdu |
| Sérialisation | Format binaire explicite | JSON, Protocol Buffers | Petits paquets, aucune dépendance, chaque champ validé à la main |
| Architecture du jeu | Notre propre ECS à sparse sets | Héritage, archétypes, EnTT | Des systèmes découplés, et le moteur est le cœur du projet |
| Modèle réseau | Serveur autoritaire, snapshots complets | Lockstep, compression delta | Résiste à la triche et tolère les pertes, au prix du débit |
| Stockage | Fichiers texte, sans base de données | SQLite, fichiers binaires | Le jeu n'a que de la configuration à garder, et on la modifie à la main |
| Build, paquets | CMake, Conan | Make, vcpkg, paquets système | Portabilité exigée, dépendances non installées sur tout le système |

## 1. Langage de programmation

Le sujet exige le C++ : le vrai choix portait donc sur la norme et sa façon de l'utiliser. Nous
utilisons **C++20**, pour ce qui rend le code plus court et plus sûr : les initialiseurs désignés
pour les structs de composants et de messages (`{.x = 1.0F, .y = 2.0F}`), `std::span` pour les
tampons avec leur taille, les algorithmes `std::ranges`, et les durées `std::chrono` plutôt que des
nombres bruts pour le temps.

Pour mémoire, voici comment les alternatives habituelles se comparent pour ce type de jeu :

| Langage | Pour ce projet | Contre |
|---------|----------------|--------|
| **C++20** (choisi) | Le contrôle de la disposition en mémoire, sur lequel repose un ECS. Toutes les bibliothèques graphiques du sujet lui sont natives. | Pas de sûreté mémoire : les paquets malformés sont un vrai risque, traité à la [section 6](#6-sécurité). |
| Rust | La sûreté mémoire à la compilation, importante pour un analyseur réseau. | Non autorisé par le sujet. Moins de bibliothèques de jeu, et l'équipe aurait dû l'apprendre. |
| C# | Productif, avec ramasse-miettes. | Les pauses du ramasse-miettes dans une boucle à 60 Hz demandent de l'attention ; non autorisé par le sujet. |
| Go | Concurrence simple, bon pour les serveurs. | Écosystème faible pour le graphisme du client ; non autorisé par le sujet. |

## 2. Graphisme, audio et entrées

Le sujet suggère SFML et en autorise d'autres comme SDL ou Raylib, mais pas les moteurs de jeu ni les
bibliothèques au périmètre trop large.

| Bibliothèque | Pour | Contre |
|--------------|------|--------|
| **Raylib** (par défaut) | Très petite API : une fenêtre, des textures, du texte et le clavier tiennent en quelques appels. Inclut l'audio et une police par défaut. Simple à compiler avec Conan. | API C ; moins de fonctions pour les interfaces complexes. |
| SFML 3 | API C++, bien connue. | Demande un fichier de police pour dessiner du texte (nous fournissons DejaVu Sans). |
| SDL 3 | La plus portable, très utilisée dans les jeux commerciaux. | Plus bas niveau : le texte et le chargement d'images demandent d'autres bibliothèques, donc plus de code chez nous. |
| ncurses | Tourne dans un terminal, par SSH, sans GPU. | Cellules de caractères, pas de textures, une touche à la fois. Ce n'est pas une vraie alternative, mais la preuve que le rendu est découplé. |

**Décision.** Nous avons choisi Raylib pour sa simplicité. Le risque d'un mauvais choix a été écarté
en cachant la bibliothèque derrière `client::gfx::IGraphics`
(`client/include/graphics/IGraphics.hpp`). Seul `client/src/graphics/<backend>/` inclut une
bibliothèque graphique. Pour prouver que l'interface fonctionne vraiment, nous avons écrit trois
backends : Raylib, SFML et ncurses. On en choisit un à la compilation avec `./rtype.py -g sfml`.

**Les entrées** passent par la même interface, car les bibliothèques de fenêtrage possèdent les
événements du clavier.

**L'audio** suit le même principe avec sa propre interface, `client::audio::IAudio`
(`client/include/audio/IAudio.hpp`). Raylib et SFML incluent toutes deux l'audio : chaque client
utilise donc la bibliothèque à laquelle il est déjà lié, sans dépendance à choisir. ncurses n'en a
pas : son client reste silencieux, comme un client sur une machine sans périphérique audio. Les sons
sont générés par un script au lieu d'être téléchargés : ils ne contiennent aucun contenu tiers et ne
posent donc aucune question de licence.

## 3. Réseau

### 3.1 Bibliothèque réseau

| Option | Pour | Contre |
|--------|------|--------|
| **Asio** (choisie), autonome | Sockets portables sous Linux et Windows, uniquement des en-têtes, sans Boost. N'impose aucun protocole. | Son modèle asynchrone dépasse nos besoins : nous n'utilisons que ses sockets, avec notre propre boucle. |
| Sockets BSD bruts, Winsock | Aucune dépendance. | Deux versions à écrire et à maintenir, pour Linux et Windows. |
| ENet | Fiabilité sur UDP déjà faite et éprouvée. | Elle déciderait du protocole à notre place, et le protocole est une grande partie du projet. |
| Réseau de SFML | Fourni avec SFML. | Lie le serveur à SFML, alors que le serveur doit se compiler sans bibliothèque graphique. |

Asio est **cachée** derrière `engine::net::IUdpSocket`, et liée comme dépendance privée. Le code réseau
peut donc être testé sur un faux socket en mémoire, avec des pertes simulées et sans vrai réseau.

### 3.2 Transport : UDP ou TCP

Le sujet exige UDP pour le trafic en jeu. C'est aussi le bon choix :

| | TCP | UDP (choisi) |
|---|-----|--------------|
| Un paquet perdu | Bloque tout ce qui suit jusqu'à son renvoi | Seul ce paquet manque |
| Effet sur le jeu | Le joueur attend une position déjà périmée | Le prochain snapshot, 50 ms plus tard, le remplace |
| Ce que nous avons dû construire | Rien | La fiabilité pour les quelques messages qui en ont besoin |

QUIC offrirait des flux indépendants sur UDP, mais il apporte le chiffrement et une grosse bibliothèque
pour un jeu qui envoie des positions.

**Deux modes de livraison** suffisent ([protocole, section 7](PROTOCOL.md#7-fiabilité)) :

- Les entrées et les snapshots sont **non fiables** : chacun porte un état plus récent et remplace le
  précédent.
- Les événements comme `EntityDestroyed` sont **fiables et ordonnés** : acquittés, et renvoyés toutes les
  100 ms jusqu'à l'acquittement.

### 3.3 Format du protocole

| Format | Taille d'une entité | Pour | Contre |
|--------|--------------------:|------|--------|
| **Binaire maison** (choisi) | 16 octets | Le plus petit, aucune dépendance, format documenté comme une RFC | Écrit à la main, avec gestion des versions |
| JSON | Environ 60 octets | Lisible, facile à déboguer | Environ 4 fois plus gros à 20 mises à jour par seconde, et lent à analyser |
| Protocol Buffers | Entre les deux | Code généré, versions compatibles | Un générateur de code et une dépendance pour environ 16 messages |

Nous ne faisons jamais de `memcpy` d'une struct vers le réseau : le remplissage et l'ordre des octets
dépendraient du compilateur. Chaque champ est écrit et lu explicitement, en little-endian. Chaque champ
est donc aussi validé à sa lecture.

Tout le format se trouve dans le [document du protocole](PROTOCOL.md), écrit pour qu'on puisse écrire un
nouveau client à partir de lui seul.

### 3.4 Modèle réseau

| Modèle | Pour | Contre |
|--------|------|--------|
| **Serveur autoritaire** (choisi, exigé par le sujet) | Une seule vérité : un client modifié ne peut pas tricher sur les positions, les touches ou les scores | Chaque action demande un aller-retour, que la [prédiction](#35-masquer-la-latence) masque |
| Lockstep | Peu de débit, utilisé par les jeux de stratégie | Tout le monde attend le joueur le plus lent ; une perte fige la partie |
| Pair à pair | Pas de serveur à héberger | Pas d'autorité, et les joueurs voient les adresses des autres |

**Snapshots complets ou deltas.** Chaque snapshot envoie toutes les entités, 20 fois par seconde. La
compression delta enverrait moins, mais elle demande des références acquittées et beaucoup plus de code, et
les snapshots complets tolèrent les pertes sans cas particulier. Le coût est mesuré :
[à 5 ko/s, le jeu prend du retard](NETWORK_COMPARISON.md#5-analyse) quand les ennemis sont nombreux. Les
deltas seraient la première chose à ajouter pour les liens lents.

### 3.5 Masquer la latence

La piste réseau de la partie 2 ajoute trois techniques. Leur effet est [mesuré](NETWORK_COMPARISON.md) :

| Technique | Ce qu'elle résout | Résultat au benchmark |
|-----------|-------------------|-----------------------|
| Interpolation des entités | 20 snapshots par seconde paraissent saccadés sur un écran rapide | Les images figées passent de 92 % à moins de 2 % |
| Prédiction et réconciliation côté client | Le vaisseau local ne réagit qu'après un aller-retour | Le délai des entrées passe d'environ 150 ms à 0, pour 100 ms d'aller-retour |
| Plusieurs parties sur un serveur | Une partie par processus gaspille des machines | Jusqu'à 16 salles, chacune avec son thread et son registre |

**Volontairement pas fait : la compensation de latence côté serveur.** Elle ramènerait les cibles à ce que le
tireur a vu. Elle ajoute un historique des positions sur le serveur, et l'équipe a gardé son temps pour les
éléments notés. Les joueurs visent donc des ennemis dessinés 100 ms dans le passé, alors que le serveur décide
sur leurs vraies positions.

## 4. Algorithmes, patrons de conception et structures de données

### 4.1 Les techniques existantes que nous avons retenues

| Besoin | Choix | Comparé à | Pourquoi |
|--------|-------|-----------|----------|
| Organiser les objets du jeu | **Entity Component System** | Arbre d'héritage (`Enemy : Entity`) | Le sujet demande des sous-systèmes découplés. Le rendu n'a besoin que d'une position et d'un sprite, pas de la vie. Un nouveau comportement est un nouveau composant, pas une nouvelle branche d'arbre. |
| Stocker les composants | **Sparse sets** | Archétypes, `std::map` | Ajout, retrait et recherche en O(1), parcours compact. Les archétypes parcourent plus vite mais déplacent les entités entre tables à chaque changement de composant, ce qui n'en vaut pas la peine pour quelques milliers d'entités. |
| Référencer une entité en sécurité | **Indice + génération** | Pointeurs bruts, simples identifiants | Un identifiant périmé est détecté au lieu de désigner en silence une nouvelle entité. |
| Détruire pendant un parcours | **Destruction différée** | Retrait immédiat | Retirer invaliderait le parcours en cours. |
| Faire tourner la simulation | **Pas de temps fixe avec accumulateur** | Pas de temps variable | Le sujet interdit de dépendre de la vitesse du CPU. Les mêmes entrées donnent le même résultat sur toute machine. |
| Échanger des données entre threads | **Mutex autour d'une file, vidée d'un coup** | Files sans verrou | Le verrou n'est tenu que le temps de quelques déplacements, donc la contention est négligeable, et c'est assez simple pour être sûr que c'est correct. |
| Détecter les collisions | **Toutes les paires de boîtes alignées sur les axes**, filtrées par couche | Grille spatiale, quadtree | O(n²), mais le jeu a peu d'entités à l'écran. Une grille serait l'étape suivante si le profilage le demandait. |
| Comparer les numéros de séquence | **Arithmétique de numéros de série** (comme la RFC 1982) | Comparaison simple | Continue de fonctionner quand les numéros de 16 bits reviennent à zéro. |

### 4.2 Patrons de conception

| Patron | Où | Pourquoi |
|--------|----|----------|
| Pont (une interface et ses backends) | `IGraphics`, `IUdpSocket` | Le code des bibliothèques reste dans un seul dossier, et les tests peuvent utiliser un faux |
| Fabrique | `server::EntityFactory` | Un seul endroit sait quels composants forment un ennemi ou un missile |
| Commande | `SpawnCommands` | Les systèmes demandent des entités, créées à un seul moment du tick : le registre ne change jamais pendant un parcours |
| Ordonnanceur à étapes | `engine::Scheduler` | L'ordre des systèmes est une donnée, pas du code dispersé dans une boucle de jeu |
| Producteur et consommateur | `engine::MessageQueue` | Le thread de jeu ne touche jamais un socket et le thread réseau ne touche jamais le registre |

### 4.3 Ce que nous avons écrit nous-mêmes, et pourquoi

| Composant | Au lieu de | Pourquoi nous l'avons écrit |
|-----------|------------|-----------------------------|
| L'ECS (`engine/`) | EnTT, Flecs | Construire le moteur est le cœur du projet. Le nôtre n'a que ce que le jeu utilise. |
| `ReliableChannel` | ENet, KCP | Environ 70 lignes, testées, et exactement la sémantique que le protocole documente. |
| `InstanceManager` | Un processus par partie | Un seul port pour toutes les salles, avec du code indépendant du jeu dans le moteur. Le coût : un plantage arrête toutes les salles. |
| `CommandConsole` | Un protocole d'administration à distance | La console reste locale : elle n'ajoute rien à attaquer par le réseau. |
| Logger | spdlog | Adapté du projet d'un membre de l'équipe ([Blackbox](https://github.com/Thomas-Peze28/Blackbox)). Un thread d'écriture évite au jeu d'attendre le terminal ou le disque. |
| Trajectoires des ennemis | Un langage de script | Sinus, zigzag et plongeon sont des données dans les fichiers de niveau, ce qui suffit pour nos vagues. |

## 5. Stockage

Le jeu n'a **ni base de données ni compte** : une partie vit en mémoire et se termine avec sa salle. Ce qui reste
à stocker est de la configuration :

| Donnée | Où | Format | Écrit par |
|--------|----|--------|-----------|
| Niveaux | `server/levels/*.json` | JSON | Les développeurs |
| Réglages des armes | `server/weapons.json` | JSON | Les développeurs |
| Types d'ennemis et de boss | `server/enemies.json` | JSON | Les développeurs |
| Adresses bannies et autorisées | Fichiers `--banlist`, `--whitelist` | Texte, une adresse par ligne | Le serveur (commandes de console) |
| Touches | `controls.cfg` | Texte, `action = touches` | Le joueur |
| Résultats du benchmark | `build/netbench/*.csv` | CSV | Le client, avec `--bench` |

### Comparaison

| Technique | Persistance | Fiabilité | Contraintes | Verdict |
|-----------|-------------|-----------|-------------|---------|
| **Fichiers texte** (choisis) | Survivent aux redémarrages | Pas de transaction : un plantage pendant une écriture peut perdre un fichier | Modifiables avec tout éditeur, lisibles dans Git | Convient à une configuration qui change rarement |
| SQLite | Survit aux redémarrages | Transactions atomiques | Une dépendance, et l'image du serveur n'a pas de stockage inscriptible par défaut | Intéressant si le jeu stocke des scores ou des comptes |
| Fichiers binaires | Survivent aux redémarrages | Comme le texte, plus des problèmes de version | Petits, mais non modifiables à la main | Aucun avantage : ces fichiers sont minuscules |
| Base de données serveur (PostgreSQL...) | Durable, partagée | Très élevée | Un service à faire tourner pour chaque serveur | Bien trop lourd pour ce jeu |
| Mémoire seule | Perdue au redémarrage | Aucune | Le plus rapide et le plus simple | Ce que nous utilisons pour l'état du jeu, sans valeur après la partie |

### La fiabilité en pratique

- **Limites de taille** : un fichier de niveau est refusé au-delà de 1 Mio, le fichier des armes au-delà de 64 Kio,
  celui des ennemis au-delà de 256 Kio. Un mauvais fichier ne peut donc pas épuiser la mémoire.
- **Validation** : un niveau est vérifié champ par champ. Un niveau invalide est ignoré et signalé avec sa position
  (`waves[4].count: must be a whole number from 1 to 20`), et le serveur démarre quand même.
- **Limite** : la liste de bannissement est réécrite sur place. Un plantage en pleine écriture pourrait la perdre.
  Écrire dans un fichier temporaire puis le renommer rendrait l'opération atomique.
- **Limite** : dans Docker, l'image ne contient que le binaire et ses données. Pour garder une liste de bannissement
  entre deux redémarrages, il faut un volume monté.

## 6. Sécurité

### 6.1 Principales vulnérabilités et ce que nous faisons

| Technologie | Vulnérabilité | Ce que fait le projet | Ce qui reste |
|-------------|---------------|-----------------------|--------------|
| C++ | Erreurs mémoire à l'analyse de données non fiables | Tampon de réception de taille fixe ; compteurs vérifiés par rapport aux octets présents avant toute réservation ; tests aléatoires avec des centaines de milliers de datagrammes modifiés ([`packet_fuzz_test.cpp`](https://github.com/T3a1/RType-mirror/blob/dev/tests/src/packet_fuzz_test.cpp)) ; clang-tidy `bugprone-*` en erreurs dans la CI | La CI ne lance ni AddressSanitizer ni fuzzer guidé par la couverture |
| UDP | Adresse source falsifiée | Une session est liée à une adresse et un port | **Limite connue** : pas de jeton à la connexion. Un `Connect` falsifié occupe une session pendant 5 secondes ([protocole, section 10](PROTOCOL.md#10-règles-de-robustesse)) |
| UDP | Inondation de paquets | Files à capacité limitée ; 64 sessions au maximum ; 256 paquets fiables non acquittés par pair | Pas de limite de débit par adresse : une inondation coûte encore du CPU |
| Protocole maison | Paquets malformés ou trop gros | Ordre de validation documenté et testé ; un paquet invalide est rejeté en entier ; une version différente est refusée | |
| Protocole maison | Ni chiffrement ni authentification | Rien de sensible n'est envoyé : pas de compte, de mot de passe ni de donnée personnelle | N'importe qui sur le chemin peut lire ou falsifier le trafic d'une partie |
| Client | Triche avec un client modifié | Serveur autoritaire : un client n'envoie que les touches maintenues | Il peut encore automatiser ses entrées |
| Fichiers JSON | Fichier malveillant ou cassé | nlohmann-json, limites de taille, validation de chaque champ | Ces fichiers viennent de l'opérateur du serveur, pas des joueurs |
| Console | Administration non autorisée | Elle ne lit que le terminal du serveur, et rien n'écoute sur le réseau | Qui accède au terminal contrôle le serveur |
| Docker | Processus serveur compromis | Binaire statique dans une image `scratch` : pas de shell, pas de fichiers libc, tourne en `nobody` | Corriger une bibliothèque demande de reconstruire l'image |
| Dépendances | Paquet vulnérable ou remplacé | Versions exactes dans `conanfile.py`, aucune copie dans le dépôt | `alpine:3` suit la dernière 3.x, volontairement |
| CI | Vol de la clé de déploiement | La clé est un secret du dépôt, et le workflow ne tourne que sur `push` : une pull request depuis un fork ne l'obtient jamais | Les GitHub Actions sont fixées par tag, pas par commit |

### 6.2 Intégrité des données

- **En transit** : le magic, la version, le type et la taille exacte de la charge utile de l'en-tête doivent tous
  correspondre. Il n'y a pas de somme de contrôle à nous : UDP en a une, et chaque champ est validé.
- **État du jeu** : seul le serveur le modifie, avec un thread par partie, et les systèmes tournent en un seul thread.
  Les messages fiables arrivent dans l'ordre et une seule fois.
- **Sur disque** : voir la [section 5](#5-stockage).

### 6.3 Surveillance à long terme

| Quoi | Comment | État |
|------|---------|------|
| Chaque changement | CI à chaque push : compilation, tests unitaires, clang-tidy, format, image Docker. Une CI rouge bloque la fusion dans `main` et le push vers le dépôt Epitech. | En place |
| Image de base | L'image du serveur est reconstruite depuis `alpine:3` à chaque push et lancement : elle suit donc les correctifs d'Alpine. | En place |
| Un serveur en marche | Journaux horodatés (`docker logs`) et la console : `status`, `rooms`, `players`, `banlist`. Les adresses refusées sont journalisées, et les paquets invalides au niveau debug. | En place, lu à la main |
| Vulnérabilités des dépendances | Comparer les versions de `conanfile.py` à la [GitHub Advisory Database](https://github.com/advisories) avant chaque version, et mettre à jour avec Conan. | **Procédure manuelle** |
| Alertes automatiques | Dependabot pourrait surveiller les GitHub Actions et l'image Docker. Il ne connaît pas nos dépendances Conan. | Non configuré |
| Détection d'attaques | Compter les paquets invalides par adresse et afficher le compteur dans `status` montrerait une inondation ou un balayage. | Non fait |

## 7. Accessibilité

Le jeu s'inspire de ce que font les jeux existants. Ce tableau ne liste que ce qui existe aujourd'hui ; le reste est
suivi dans les issues du projet.

| Catégorie | Ce qui existe | Inspiré de | Reste à faire |
|-----------|---------------|------------|---------------|
| **Physique et moteur** | Toutes les touches sont configurables (`controls.cfg`). Des touches par défaut pour chaque main : le jeu se joue d'une seule main. Tir automatique, actif par défaut, pour ne pas avoir à appuyer sans cesse. | Options de configuration de la plupart des jeux PC | Vitesse du jeu, manette |
| **Visuel** | Couleurs des joueurs tirées de la palette Okabe-Ito, que les daltoniens distinguent. Jamais la couleur seule : chaque vaisseau affiche « P1 » et « (you) », et les notifications nomment le joueur. | Modes daltoniens des jeux multijoueurs | Interface du jeu redimensionnable, contraste élevé |
| **Audio** | Le son n'est jamais le seul indice : les tirs, les explosions, la lueur de charge, la bannière de niveau et l'écran de fin montrent à l'écran ce que disent les sons. | Indices visuels de la plupart des jeux d'action | Réglages de volume, coupure du son |
| **Mental et cognitif** | Des menus d'une page, quelques actions claires, une bannière « Level 2 » et les scores finaux. Trois niveaux de difficulté et le choix du niveau de départ, dans la salle d'attente. | Réglages de difficulté de la plupart des jeux d'action | Un rappel des commandes, la réduction des effets clignotants |

**La documentation** : des pages en ligne avec une barre de recherche et un plan ; un thème clair ou sombre qui suit le
système du lecteur ; des images avec une légende visible ; des titres dans l'ordre pour les lecteurs d'écran ; une
version en anglais et une en français. Chaque graphique du benchmark a aussi ses chiffres dans un tableau. Les diagrammes
Mermaid sont expliqués dans le texte qui les entoure.

**Pourquoi c'est important pour ces choix** : faire des commandes une donnée (`controls.cfg`) et de l'identité des joueurs
du texte (les étiquettes) n'a presque rien coûté à la conception. Les ajouter plus tard aurait touché tous les systèmes
d'entrée et de dessin.

## 8. Outils

| Sujet | Choix | Comparé à | Pourquoi |
|-------|-------|-----------|----------|
| Build | CMake | Make, Meson | Le standard du C++ sous Linux et Windows, et exigé par la chaîne d'outils de Conan |
| Paquets | Conan | vcpkg, paquets système, sous-modules Git | Le sujet interdit de dépendre de bibliothèques installées sur le système ou de copier les dépendances. Les paquets de Conan fonctionnent de la même façon sur nos plateformes. |
| Image du serveur | Alpine, binaire statique musl, `scratch` | Image basée sur Debian | Une très petite image, sans rien à attaquer. Le serveur ne peut donc pas y utiliser Conan, d'où les paquets Alpine. |
| Launcher | Python (`rtype.py`) | Script shell | Le même script sous Linux et Windows ; il a remplacé `rtype.sh` |
| Tests | GoogleTest | Catch2 | Inclut les mocks, très connu |
| Qualité du code | clang-format, clang-tidy | Relecture seule | Imposés par la CI, avec une seule version de clang-format pour tous |
| Documentation | Doxygen, pour les pages et la référence du code | MkDocs, Sphinx, un wiki | Un seul outil pour un seul site : les pages restent en Markdown à côté du code et sont relues dans les pull requests, et elles peuvent renvoyer à la référence du code. Il dessine nos diagrammes Mermaid et a une barre de recherche. Sa limite : la recherche ne couvre que les titres, les sections et les noms de code, pas le texte des pages. Un second générateur aurait apporté la recherche plein texte et un second outil à maintenir. Les pages françaises sont une seconde génération de Doxygen, sous `fr/`, et un petit script ajoute le bouton de langue. |

## 9. Avant et après : le prototype de la partie 1 et la version finale

Mesuré le 6 novembre 2026, par rapport au tag `v1.0` (18 octobre 2026).

| | Partie 1 (`v1.0`) | Maintenant |
|---|-------------------|------------|
| Code C++ (lignes, en-têtes compris) | 8 210 | 26 600 |
| Tests unitaires | 86 | 290 |
| Systèmes ECS (fichiers source) | 20 | 34 |
| Version du protocole | 1 | 11 |
| Parties sur un serveur | 1 | Jusqu'à 16 salles, avec un lobby |
| Bibliothèques graphiques | Raylib, appelée directement | Raylib, SFML, ncurses, derrière une interface |
| Bibliothèque réseau | Asio, visible dans le code | Cachée derrière une interface, testée sur un faux réseau |
| Boucle de jeu | Écrite à la main dans chaque programme | `engine::Scheduler`, avec des étapes et des conditions |
| Création d'entités | Directe, pendant le tick | Demandée, puis effectuée à un seul moment du tick |

**Architecture.** Le moteur contient maintenant ce qu'un autre jeu réutiliserait : ordonnanceur, gestionnaire d'instances,
interface de socket, console de commandes. Le code des bibliothèques est passé derrière des interfaces, et les boucles de
jeu sont devenues une liste de systèmes.

**Performances**, d'après [le benchmark](NETWORK_COMPARISON.md), pour le même build avec chaque technique désactivée, à
100 ms d'aller-retour :

| | Comportement de la partie 1 | Final |
|---|-----------------------------|-------|
| Délai entre un appui et le mouvement du vaisseau | Environ 150 ms | 0 ms |
| Images où une entité est figée | 92 % | Moins de 2 % |
| Plus grands déplacements d'une entité par image (99e centile) | 22,6 unités | 3,9 unités |
| Débit montant du client à 240 images par seconde | Environ 4 ko/s | Environ 0,5 ko/s |

**Fonctionnalités.** Planches de sprites et animations à la place de rectangles colorés ; nouveaux types d'ennemis avec des
trajectoires ; niveaux, armes et ennemis en fichiers JSON ; deux boss de fin de niveau ; obstacles et blocs destructibles ;
tir chargé et bonus ; fin de partie et scores finaux ; difficulté, niveau de départ et boss choisis dans la salle d'attente ;
effets sonores et musique ; lobby, salles et salle d'attente ; console d'administration avec bannissements et liste blanche ;
configuration des touches et tir automatique ; un launcher graphique ; un build Windows en CI.

## 10. Ce que nous ferions autrement

- **Snapshots** : penser à la compression delta dès le début, car le test à 5 ko/s montre la limite des snapshots complets.
- **Connexion** : ajouter le jeton de défi dès le début. Il coûte peu à concevoir et ferme la principale faiblesse connue du
  protocole.
- **Contenu** : les niveaux, les armes et les ennemis sont des fichiers de données, mais un nouveau sprite ou un nouveau
  comportement demande encore du C++. Un langage de script serait l'étape suivante.
- **Sanitizers en CI** : AddressSanitizer sur les tests coûte une tâche de CI et appuierait les affirmations de la
  [section 6](#6-sécurité).
