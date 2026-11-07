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
| Espace | Ctrl droit | Tirer : un tir par appui. Maintenu 1 seconde puis relâché : un tir chargé qui traverse les ennemis |
| F | Maj droite | Activer ou couper le tir automatique : le vaisseau tire seul, 4 fois par seconde |
| Échap | Échap | Quitter |
| T | T | Écrire à votre salle, en salle d'attente ou en jeu : Entrée envoie, Échap annule. Le vaisseau s'arrête pendant la saisie |

**Choisissez vos touches** : écrivez un fichier `controls.cfg` dans le dossier depuis
lequel vous lancez le jeu (ou donnez son chemin avec `--controls FILE`). Chaque ligne
donne les touches d'une action et remplace ses touches par défaut :

```text
# action = touche touche...
fire = Enter Space
autofire = Tab
```

- Actions : `up`, `down`, `left`, `right`, `fire`, `autofire`, `chat`.
- Touches : `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `LeftShift`,
  `RightShift`, `LeftCtrl`, `RightCtrl`, `LeftAlt`, `RightAlt`, les lettres `A` à `Z`
  et les chiffres `0` à `9`. Avec le client par défaut (Raylib), une lettre désigne
  la touche placée à cet endroit sur un clavier américain : W A S D correspondent aux
  touches Z Q S D d'un clavier AZERTY français.
- Une ligne que le jeu ne peut pas lire est signalée au démarrage, et l'action
  concernée garde ses touches par défaut.

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
| [Guides pratiques](HOWTO.md) | Ajouter un composant, un système, un ennemi ou un paquet ; compiler et déboguer. |
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
