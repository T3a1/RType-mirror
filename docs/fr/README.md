# R-Type

Un remake multijoueur en réseau de **R-Type**, le shoot'em up d'arcade de 1987.
Jusqu'à quatre joueurs font équipe en ligne, chacun aux commandes d'un vaisseau,
face à des vagues d'ennemis.

> Ceci est la version française de la documentation. En cas de différence, la
> [version anglaise](https://t3a1.github.io/RType-mirror/) fait foi. La référence
> du code, générée à partir des commentaires, n'existe qu'en anglais.

## Le jeu

- **Multijoueur coopératif** : un serveur héberge la partie, et jusqu'à quatre
  joueurs la rejoignent par le réseau depuis leur ordinateur.
- **De l'action façon arcade** : déplacez votre vaisseau, tirez sur les ennemis qui
  arrivent par la droite et évitez d'être touché.
- **Un coup, une vie** : un seul tir ennemi ou une seule collision détruit votre
  vaisseau, et chaque joueur gagne des points pour chaque ennemi abattu. Si vous
  mourez, vous regardez les autres jusqu'à ce que toute l'équipe soit éliminée et
  que les scores finaux s'affichent.
- **Fair-play** : le serveur est le seul juge de ce qui se passe, donc tous les
  joueurs voient la même partie.
- **Votre jeu, vos règles** : avant de commencer, choisissez la difficulté (facile,
  normale, difficile), le niveau de départ (ou le mode sans fin) et le boss.

### Commandes

Chaque action a une touche pour chaque main, donc le jeu se joue d'une seule main.

| Main gauche | Main droite | Action |
|-------------|-------------|--------|
| W A S D | Flèches | Déplacer le vaisseau |
| Espace | Ctrl droit | Tir chargé : maintenir 1 seconde, puis relâcher. Un rayon qui traverse les ennemis |
| Échap | Échap | Quitter la salle. Dans la liste des salles : retour au menu principal |
| T | T | Écrire à votre salle, en salle d'attente ou en jeu : Entrée envoie, Échap annule. Le vaisseau s'arrête pendant la saisie |

Dans les menus, les flèches déplacent la sélection, et Espace ou Entrée choisit.

**Jouer à la manette** : branchez une manette Xbox ou PlayStation, avant ou pendant la
partie. Elle fonctionne en même temps que le clavier :

| Xbox | PlayStation | Action |
|------|-------------|--------|
| Croix directionnelle ou stick gauche | Croix directionnelle ou stick gauche | Déplacer le vaisseau. Dans les menus : déplacer la sélection |
| A ou RT | Croix ou R2 | Tir chargé. Dans les menus : A (Croix) choisit |
| B | Rond | Dans les menus : revenir en arrière. Ferme le chat. Ne quitte jamais une partie |
| Back | Create | Quitter la salle |

Il faut un clavier pour écrire dans le chat. Le stick déplace le vaisseau à pleine
vitesse dès qu'il dépasse la moitié de sa course, dans huit directions comme le clavier.

**Choisissez vos touches** : ouvrez Settings dans le menu principal, puis l'onglet
Keyboard. Chaque action a deux touches : choisissez-en une, puis appuyez sur la
nouvelle touche, ou sur Échap pour annuler. Le jeu vous signale si une autre action
utilise la même touche, et Reset keys remet les touches par défaut. L'onglet Gamepad
fait de même pour les boutons de la manette, et Échap ou B (Rond) y annule. Les
flèches, Espace, Entrée et Échap, ainsi que la croix directionnelle, A (Croix) et
B (Rond), fonctionnent toujours dans les menus.

Votre choix est enregistré dans un fichier `controls.cfg`, dans le dossier depuis
lequel vous lancez le jeu (ou dans le fichier donné avec `--controls FILE`), que vous
pouvez aussi modifier à la main. Chaque ligne donne les touches d'une action et
remplace ses touches par défaut. Une ligne qui commence par `pad_` donne plutôt ses
boutons de manette :

```text
# action = touche touche...
fire = Enter Space
chat = Tab
pad_fire = PadA PadRT
```

- Actions : `up`, `down`, `left`, `right`, `fire`, `chat`, et les mêmes
  noms après `pad_` pour la manette.
- Touches : `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `LeftShift`,
  `RightShift`, `LeftCtrl`, `RightCtrl`, `LeftAlt`, `RightAlt`, les lettres `A` à `Z`
  et les chiffres `0` à `9`. Avec le client par défaut (Raylib), une lettre désigne
  la touche placée à cet endroit sur un clavier américain : W A S D correspondent aux
  touches Z Q S D d'un clavier AZERTY français.
- Boutons de manette, seulement sur les lignes `pad_` : `PadUp`, `PadDown`,
  `PadLeft`, `PadRight` (croix directionnelle), `PadStickUp`, `PadStickDown`,
  `PadStickLeft`, `PadStickRight` (stick gauche), `PadA`, `PadB` (Croix, Rond),
  `PadLB`, `PadRB`, `PadLT`, `PadRT` (L1, R1, L2, R2), `PadStart`, `PadSelect`
  (Options, Create).
- Une ligne que le jeu ne peut pas lire est signalée au démarrage, et l'action
  concernée garde ses touches par défaut.

**Changer le volume ou la fenêtre** : ouvrez Settings dans le menu principal.
L'onglet Audio contient les volumes. L'onglet Video contient le plein écran, la
taille de la fenêtre, la synchronisation verticale (V-Sync) et la limite d'images par
seconde, que V-Sync remplace tant qu'elle est active. Le jeu enregistre votre choix
dans un fichier `settings.cfg`, dans le dossier depuis lequel vous le lancez, que vous
pouvez aussi modifier à la main. Un volume va de 0 à 1 :

```text
# nom = valeur
music_volume = 0.4
sound_volume = 1
fullscreen = off
window_size = 1280x720
vsync = on
frame_limit = 0
player_labels = on
background = on
reduce_flashing = off
```

Tailles de fenêtre : `1280x720`, `1600x900`, `1920x1080`, `2560x1440`. Limites
d'images : `15`, `30`, `60`, `120`, ou `0` pour aucune limite. Une ligne que le jeu ne
peut pas lire est signalée au démarrage, et ce réglage garde sa valeur. Un
enregistrement remplace tout le fichier, donc vos commentaires ne sont pas conservés.

**Options d'accessibilité** : ouvrez Accessibility dans le menu principal.

- **Player labels** : affiche « P1 » et « (you) » au-dessus des vaisseaux. Sans ces
  étiquettes, seule la couleur distingue les vaisseaux pendant une partie.
- **Background** : dessine le fond d'étoiles et de planètes qui défile, par défaut.
  Désactivé, le jeu est dessiné sur un écran noir immobile, pour les joueurs gênés par
  le mouvement ou qui ont besoin de plus de contraste.
- **Reduce flashing** : désactivé par défaut. Activé, les vaisseaux et les ennemis
  détruits disparaissent sans explosion (leur son est toujours joué) et la lueur d'un
  tir en charge ne scintille plus. Le jeu n'a pas de tremblement d'écran.

## Pour commencer

R-Type fonctionne sous **Linux** et **Windows**. Un seul script compile et lance tout :

```bash
./rtype.py              # sous Windows : python rtype.py
```

Il lui faut Python 3, un compilateur C++20 et Docker (ou `--native` pour s'en passer) ;
il installe le reste lui-même, comme CMake et Conan. Le jeu utilise
[Raylib](https://www.raylib.com/) et [Asio](https://think-async.com/Asio/), que Conan
télécharge. Consultez le [guide d'installation](INSTALLATION.md) pour les prérequis et
toutes les options.

Pour jouer sur plusieurs machines, l'une ne lance que le serveur et les autres que les
clients :

```bash
./rtype.py --server-only --native     # sur la machine hôte
./rtype.py --clients-only -H <hôte>   # sur la machine de chaque joueur
```

Les mêmes options existent aussi dans une fenêtre : `./rtype.py --gui`.

## Documentation

Toute la documentation est en ligne, avec un plan et une barre de recherche :
**<https://t3a1.github.io/RType-mirror/fr/>** (ou `./rtype.py --docs` pour la générer
en local). Un bouton sur chaque page passe à l'autre langue.

| Document | Ce que vous y trouverez |
|----------|-------------------------|
| [Guide d'installation](INSTALLATION.md) | Prérequis, compilation, lancement du serveur et des clients, dépannage. |
| [Architecture](ARCHITECTURE.md) | L'organisation du code : moteur de jeu, sous-systèmes, boucle de jeu, threads. |
| [Protocole réseau](PROTOCOL.md) | Les messages échangés entre le serveur et les clients. |
| [Étude technique](TECHNICAL_STUDY.md) | Le pourquoi de ces technologies : comparaisons, stockage, sécurité, accessibilité, avant et après. |
| [Guides pratiques](HOWTO.md) | Ajouter un composant, un système, un ennemi, un paquet, un son, une page de menu ou un réglage ; compiler et déboguer. |
| [Niveaux](LEVELS.md) | Écrire un niveau : le fichier JSON de ses vagues d'ennemis, et comment le serveur joue les niveaux. |
| [Armes](WEAPONS.md) | Le tir chargé, les bonus et le fichier de réglage des armes. |
| [Ennemis](ENEMIES.md) | Le fichier des ennemis : les statistiques de chacun, et de nouveaux ennemis et boss sans C++. |
| [Tests réseau](NETWORK_TESTING.md) | Jouer avec de la latence, des pertes, de la duplication de paquets et un faible débit sur une seule machine. |
| [Comparaison des performances réseau](NETWORK_COMPARISON.md) | L'effet mesuré de l'interpolation et de la prédiction sur un réseau dégradé, par rapport au prototype de la partie 1. |
| [Contribuer](CONTRIBUTING.md) | Comment travailler sur le projet : méthode, conventions, outils. |
| [Contributeurs](CONTRIBUTORS.md) | Qui a réalisé le projet, et comment nous joindre. |

## État du projet

| Étape | Date | Contenu |
|-------|------|---------|
| Partie 1 | 18 octobre 2026 | Moteur de jeu, réseau et prototype jouable. |
| Partie 2 | 8 novembre 2026 | Fonctionnalités avancées, construites sur le prototype. |

## Licence

R-Type est un logiciel libre, sous [licence publique générale GNU v3.0](https://github.com/T3a1/RType-mirror/blob/dev/LICENSE).

La licence couvre notre code et notre documentation, pas les ressources tierces de
`client/assets/` : les planches de sprites de R-Type restent la propriété de leurs
ayants droit, et DejaVu Sans a sa propre licence, à côté de la police.

## Auteurs

Léandre Fouret, Romane Cauvez, Thomas Peze et David Caron : voir les
[contributeurs](CONTRIBUTORS.md).
