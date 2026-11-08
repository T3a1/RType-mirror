# Niveaux

Les niveaux de R-Type sont des données, pas du code : chaque niveau est un fichier
JSON de [`server/levels/`](https://github.com/T3a1/RType-mirror/blob/dev/server/levels/).
Ajouter ou modifier un niveau ne demande pas de recompiler, seulement de relancer le
serveur.

## Comment le serveur les joue

1. Au démarrage, le serveur lit chaque fichier `.json` de son dossier de niveaux, dans
   l'ordre des noms de fichiers (d'où les préfixes `01-`, `02-`).
2. Chaque salle joue les niveaux l'un après l'autre. Un niveau lance ses vagues aux
   instants écrits dans son fichier, comptés depuis son début.
3. Une fois toutes ses vagues arrivées et plus aucun ennemi présent, le boss du niveau
   arrive s'il en a un. Le niveau se termine quand le boss est détruit, ou aussitôt
   s'il n'y a pas de boss. Le niveau suivant commence 3 secondes plus tard.
4. Après le dernier niveau vient le **mode sans fin** : des vagues aléatoires, jusqu'à
   la fin de la partie.
5. Une fois la partie terminée, le niveau s'arrête où il en est : aucune nouvelle
   vague, aucun obstacle ni boss n'arrive pendant les scores finaux.

Un niveau peut aussi faire entrer des **obstacles**, aux instants de son fichier : des
murs et des blocs destructibles qui défilent depuis le bord droit (voir
[Obstacles](#obstacles)). Ils n'empêchent pas un niveau de se terminer, et le mode sans
fin n'en a pas.

Chaque niveau est annoncé aux joueurs (`GameEvent{LevelStarted}`, voir le
[protocole](PROTOCOL.md)) : une grande bannière « LEVEL 2 », après « Level cleared »,
pendant la pause de 3 secondes, et le niveau en cours en haut de l'écran.

Le dossier des niveaux est `levels/` à côté de l'exécutable du serveur (`build/levels/`,
copié là par le build, ou `/levels` dans l'image Docker). Un autre dossier peut être
donné avec `--levels DIR` :

```bash
./build/r-type_server --levels my-levels/
```

Un fichier illisible est ignoré et signalé dans la sortie du serveur, avec ce qui ne va
pas et où ; les autres niveaux sont quand même joués. Sans aucun niveau valide, le
serveur ne joue que le mode sans fin. Un fichier de niveau ne fait jamais planter le
serveur.

```text
[WARN] Level skipped: 03-boss.json: waves[4].count: must be a whole number from 1 to 20
```

## Format du fichier

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
    ],
    "obstacles": [
        { "time": 8, "kind": "tile", "y": 96, "rows": 3, "columns": 2 },
        { "time": 30, "kind": "wall", "y": 32, "rows": 4, "columns": 3 }
    ]
}
```

### Niveau

| Clé | Obligatoire | Valeur |
|-----|:-----------:|--------|
| `name` | oui | Le nom du niveau, de 1 à 64 caractères. |
| `waves` | oui | La liste de ses vagues, de 1 à 1000. Leur ordre dans le fichier n'a pas d'importance : elles sont triées par `time`. |
| `boss` | non | Le boss qui arrive après les vagues : `warship`, `hive`, `walker`, `twin`, `behemoth`, ou un boss de [`enemies.json`](ENEMIES.md) comme `dreadnought`. Sans lui, le niveau se termine avec sa dernière vague. Les réglages de partie peuvent le remplacer par un autre boss. |
| `obstacles` | non | La liste de ses groupes de murs et de blocs, 1000 au maximum, voir [Obstacles](#obstacles). Leur ordre n'a pas d'importance : ils sont triés par `time`. |

### Boss

| Nom | Ce qu'il fait |
|-----|---------------|
| `warship` | Arrive par la droite, s'arrête, puis monte et descend. 2500 points de vie, affichés dans une barre en bas de l'écran. Trois phases selon ses dégâts : des tirs visés, puis un éventail de 3 tirs plus un tir visé, puis un éventail plus rapide de 5 tirs en bougeant plus vite. |
| `hive` | Un boss organique plus grand et plus lent, posé au sol et immobile une fois en place, avec les mêmes trois phases d'attaque. |
| `walker` | Niveau 3. Un robot au sol qui marche de gauche à droite, plus vite à chaque phase, et tire des boules de feu : un éventail visé et un arc au-dessus de lui à tour de rôle, puis un jet qui balaie le ciel, puis deux jets et de larges arcs. 3000 points de vie. |
| `twin` | Niveau 4. Deux moitiés immobiles une fois en place, l'une au sol et l'autre accrochée au plafond ; toutes deux font baisser la même barre de vie. Des éventails visés depuis chaque moitié à tour de rôle, puis deux jets qui se croisent comme des ciseaux, puis des murs de tirs qui se décalent. 3600 points de vie. |
| `behemoth` | Niveau 5. Une créature géante qui monte et descend lentement, avec une tête qui sort de son ventre et une queue qui ondule. Un tir sur sa **tête lui inflige 3 fois ses dégâts**, un tir sur sa queue blindée la moitié. Des rafales de tirs visés depuis sa tête, puis des anneaux depuis le bout de sa queue, puis une spirale. 4500 points de vie. |

## Réglages de partie

Le joueur qui lance la partie choisit, dans la salle d'attente :

- **La difficulté** : Facile (les ennemis ont 60 % de leur vie et tirent 1,5 fois moins
  souvent), Normale, ou Difficile (150 % de leur vie, tirs 30 % plus fréquents, vagues
  aléatoires du mode sans fin 30 % plus rapprochées).
- **Le départ** : un niveau, ce qui saute les niveaux précédents, ou le mode sans fin.
- **Le boss** : celui de chaque fichier de niveau, ou le warship ou le hive pour tous
  les niveaux qui ont un boss.

### Vague

Une vague est un groupe d'ennemis d'un même type, qui arrive par le bord droit de
l'écran. L'aire de jeu fait 1920 unités de large et 1080 de haut, `y` allant vers le bas.

| Clé | Obligatoire | Défaut | Valeur |
|-----|:-----------:|--------|--------|
| `time` | oui | | Le départ de la vague, en secondes depuis le début du niveau (0 à 3600). |
| `enemy` | oui | | L'ennemi : `basic`, `wave`, `shooter`, `small`, `medium`, `large`, ou un ennemi de [`enemies.json`](ENEMIES.md) comme `kamikaze` ou `gunship`. |
| `count` | non | `1` | Le nombre d'ennemis, un nombre entier de 1 à 20. |
| `spacing` | non | `0` | Les secondes entre deux ennemis (0 à 10). À même hauteur, cet écart forme un serpent. |
| `stepY` | non | `0` | La distance verticale entre deux ennemis (0 à 1080). Sans écart, cela forme une colonne. |
| `fromCenter` | non | `false` | `true` : l'ennemi du milieu arrive d'abord et les autres après lui, selon leur distance (un V). |
| `y` | non | aléatoire | La hauteur du centre de la vague (0 à 1080). Sans elle, une hauteur aléatoire qui garde toute la vague à l'écran. |
| `pattern` | non | celle de l'ennemi | La trajectoire des ennemis, voir ci-dessous. Sans elle, chaque ennemi suit la trajectoire de son type. |

### Trajectoire

| Clé | Obligatoire | Défaut | Valeur |
|-----|:-----------:|--------|--------|
| `kind` | oui | | `straight`, `sine` (vagues douces), `zigzag` ou `dive` (va tout droit, puis fonce sur le joueur le plus proche). |
| `amplitude` | non | `0` | La hauteur des vagues de `sine` et `zigzag` (0 à 1080). |
| `frequency` | non | `0` | Les vagues par seconde de `sine` et `zigzag` (0 à 10). |
| `diveDelay` | non | `0` | Les secondes avant qu'un `dive` ne fonce (0 à 60). |
| `diveSpeed` | non | `0` | La vitesse de la charge d'un `dive` (0 à 5000). |

### Obstacles

Un groupe d'obstacles est un rectangle de blocs de 64 × 64 unités qui entre par le bord
droit à 120 unités par seconde, un peu moins vite que les ennemis.

| Type | Ce qu'il fait |
|------|---------------|
| `wall` | Un bloc de métal. Il arrête tous les tirs, y compris le tir chargé, et détruit un vaisseau qui le touche. Rien ne le casse. |
| `tile` | Un bloc de briques. Il arrête aussi tous les tirs et détruit un vaisseau qui le touche, mais il casse après 3 tirs normaux (150 points de vie). Il paraît fissuré à la moitié de sa vie, et donne 20 points au joueur dont le tir le casse. |

| Clé | Obligatoire | Défaut | Valeur |
|-----|:-----------:|--------|--------|
| `time` | oui | | L'entrée de la première colonne, en secondes depuis le début du niveau (0 à 3600). Les colonnes suivantes suivent sans espace, une toutes les 0,53 s. |
| `kind` | oui | | `wall` ou `tile`. |
| `y` | oui | | La hauteur du centre de la rangée du haut (0 à 1080). Un bloc centré sur 32 touche le bord haut, un bloc sur 1048 le bord bas. |
| `rows` | non | `1` | Le nombre de blocs de haut en bas, de 1 à 16. |
| `columns` | non | `1` | Le nombre de blocs de gauche à droite, de 1 à 30. |

Laissez un passage aux vaisseaux : un passage plus étroit qu'un vaisseau (64 unités) ne
se franchit qu'en cassant des blocs. Des murs en haut et en bas au même `time` forment un
couloir.

Toute autre clé, une valeur du mauvais type ou hors limites rend le fichier invalide :
une faute de frappe est signalée au lieu d'être ignorée en silence.

## Limites

Pour protéger le serveur, un fichier de niveau de plus de 1 Mio n'est pas lu, et les
limites ci-dessus (vagues par niveau, ennemis par vague, groupes d'obstacles, rangées et
colonnes, instants et distances) sont vérifiées au chargement du fichier.

## Code

- [`server/include/Level.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/Level.hpp) :
  les structures `Level` et `Wave`, `parseLevel()` et `loadLevels()` (JSON lu avec
  nlohmann-json).
- [`server/include/systems/LevelSystem.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/systems/LevelSystem.hpp) :
  `levelSystem()`, qui joue les niveaux, et le composant `LevelProgress` sur l'entité
  du générateur. Tant qu'il est là, `spawnSystem()` n'envoie pas de vagues aléatoires.
- [`server/include/systems/ObstacleSystem.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/systems/ObstacleSystem.hpp) :
  `obstacleSystem()`, qui fait entrer les colonnes de `Level::obstacles` (l'analyseur
  découpe chaque groupe du fichier en colonnes). Les blocs sont construits par
  `EntityFactory` à partir d'une commande `SpawnObstacle` ; `damageSystem()` leur fait
  arrêter les tirs.
- [`tools/generate_tiles.py`](https://github.com/T3a1/RType-mirror/blob/dev/tools/generate_tiles.py) :
  dessine les sprites des obstacles (`client/assets/sprites/obstacles.png`).
