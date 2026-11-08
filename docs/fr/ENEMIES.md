# Ennemis et boss

Les ennemis de R-Type sont des données : leurs statistiques, et de nouveaux types
d'ennemis et de boss, sont écrits dans
[`server/enemies.json`](https://github.com/T3a1/RType-mirror/blob/dev/server/enemies.json).
Ajouter un ennemi ou un boss ne demande ni code C++, ni recompilation, ni nouvelle
version du protocole : relancez le serveur et utilisez le nouveau nom dans un
[fichier de niveau](LEVELS.md).

## Comment le serveur le lit

1. Au démarrage, avant les niveaux, le serveur lit `enemies.json` à côté de son
   exécutable (`build/enemies.json`, copié là à chaque build, ou `/enemies.json` dans
   l'image Docker).
2. Il part des 8 types **intégrés**, puis applique le fichier : de nouvelles
   statistiques pour un type intégré, et de nouveaux types d'ennemis.
3. Toutes les salles jouent avec les mêmes types. Les fichiers de niveau les appellent
   par leur nom, dans leurs vagues (`"enemy"`) et comme boss (`"boss"`).

Si le fichier est absent, trop gros (plus de 256 Kio) ou invalide, le serveur garde les
ennemis intégrés et dit pourquoi. Un fichier invalide ne fait jamais planter le serveur,
et un niveau qui nomme un ennemi absent est ignoré et signalé :

```text
[WARN] Built-in enemies used: enemies.json: enemies.drone.health: must be from 1 to 40000
```

## Format du fichier

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

Chaque clé de `enemies` est le nom d'un type, utilisé par les fichiers de niveau :

- **Un nom intégré** modifie les statistiques de ce type. Chaque clé est facultative ;
  `base` n'est pas autorisée.
- **Un autre nom** ajoute un type. Il lui faut une `base`, le type intégré dont il part :
  il en reprend le comportement et toutes les statistiques que le fichier ne donne pas.

### Types intégrés

| Nom | Comportement | Statistiques par défaut |
|-----|--------------|-------------------------|
| `basic` | Vole tout droit, ne tire pas. | 100 de vie, vitesse 200, 100 points |
| `wave` | Suit une onde sinusoïdale. | 50 de vie, vitesse 250, 50 points |
| `shooter` | Vole tout droit et tire. | 100 de vie, vitesse 100, un tir toutes les 1,5 s, 150 points |
| `small` | Rapide, suit une onde sinusoïdale. | 50 de vie, vitesse 300, 25 points |
| `medium` | Suit un zigzag et tire, laisse un bonus. | 150 de vie, vitesse 150, 200 points |
| `large` | Lent et résistant, tire, laisse un bonus. | 500 de vie, vitesse 70, 500 points |
| `warship` | **Boss** : entre, s'arrête, monte et descend, trois phases d'attaque. | 2500 de vie, 5000 points |
| `hive` | **Boss** : plus grand et plus lent, posé au sol et immobile une fois en place, mêmes trois phases. | 2500 de vie, 5000 points |
| `spinner` | Niveau 3. Un anneau qui tourne, suit une onde sinusoïdale et tire. | 120 de vie, vitesse 160, un tir toutes les 1,6 s, 180 points |
| `bubble` | Niveau 3. Rapide, suit une large onde sinusoïdale. | 60 de vie, vitesse 300, 80 points |
| `spore` | Niveau 4. Lent et résistant, suit un zigzag, laisse un bonus. | 260 de vie, vitesse 110, 250 points |
| `jelly` | Niveau 4. Attend, puis fonce sur le joueur le plus proche. | 90 de vie, vitesse 220, 120 points |
| `crab` | Niveau 4. Vole tout droit et tire, laisse un bonus. | 180 de vie, vitesse 130, un tir toutes les 1,2 s, 260 points |
| `walker` | **Boss** du niveau 3 : marche au sol ([LEVELS.md](LEVELS.md)). | 3000 de vie, 7000 points |
| `twin` | **Boss** du niveau 4 : une moitié au sol et une au plafond. | 3600 de vie, 8000 points |
| `behemoth` | **Boss** du niveau 5 : corps, tête (3 fois les dégâts) et queue. | 4500 de vie, 10000 points |

Un type basé sur `warship`, `hive`, `walker`, `twin` ou `behemoth` est un boss : il ne peut être que le `boss` d'un
niveau, et les autres types n'arrivent qu'en vagues.

### Clés

| Clé | Valeur |
|-----|--------|
| `base` | Nouveaux types uniquement, obligatoire : le type intégré dont il part. |
| `look` | Le type intégré dont les joueurs voient le sprite. Les clients ne connaissent que les sprites des types intégrés, donc un nouvel ennemi en réutilise un ; c'est ce qui lui évite une nouvelle version du protocole. |
| `health` | Les points de vie, un nombre entier de 1 à 40000. La difficulté de la partie les modifie. |
| `speed` | La vitesse vers la gauche, de 0 à 3000 unités par seconde. |
| `width`, `height` | La taille de son collider, de 1 à 1080 unités. Gardez la taille de son `look` (voir le tableau ci-dessous), sinon les tirs semblent le rater. |
| `score` | Les points du joueur qui le détruit, de 0 à 1000000. |
| `fireInterval` | Les secondes entre deux tirs, de 0 à 60 ; 0 ne tire pas. Les boss l'ignorent : leurs phases d'attaque sont du code. |
| `pattern` | Sa trajectoire, avec les clés de la [trajectoire](LEVELS.md#trajectoire) d'une vague (`kind`, `amplitude`, `frequency`, `diveDelay`, `diveSpeed`). La trajectoire propre à une vague la remplace toujours. |
| `dropsPowerUp` | `true` s'il laisse un bonus une fois détruit. |

Toute autre clé, une valeur du mauvais type ou hors limites rend le fichier invalide.

Tailles des apparences, à reporter dans `width` et `height` :

| Apparence | Largeur × hauteur |
|-----------|-------------------|
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
| `twin` | 200 × 210 (chaque moitié) |
| `behemoth` | 220 × 400 (le corps, sans sa tête ni sa queue) |

## Ajouter un ennemi sans écrire de C++

1. Ajoutez-le à `server/enemies.json`, à partir du type intégré au comportement le plus
   proche :

   ```json
   "turret": { "base": "shooter", "look": "large", "width": 126, "height": 96,
               "speed": 40, "health": 400, "fireInterval": 0.6, "score": 400 }
   ```

2. Appelez-le dans un fichier de niveau :

   ```json
   { "time": 12, "enemy": "turret", "count": 2, "stepY": 400, "y": 540 }
   ```

3. Relancez le serveur (`./rtype.py --native` copie les fichiers et le relance). Les
   clients n'ont besoin de rien.

## Ajouter un boss sans écrire de C++

1. Ajoutez-le, à partir de `warship` ou `hive` :

   ```json
   "dreadnought": { "base": "hive", "health": 3500, "speed": 90, "score": 8000 }
   ```

2. Faites-en le boss d'un niveau : `"boss": "dreadnought"`. Il arrive une fois toutes les
   vagues du niveau éliminées, et le niveau se termine quand il est détruit. Sa barre de
   vie se remplit à partir de la vie avec laquelle il arrive.

Le boss choisi dans les réglages de partie (warship, hive, walker, twin ou behemoth) remplace toujours le boss de
chaque niveau.

## Ce qui demande encore du code

- **Un nouveau sprite** : les clients ne dessinent que les sprites des types intégrés.
  Une nouvelle image demande un nouvel `EntityType`, donc une version du protocole (voir
  le [guide pratique](HOWTO.md#2-ajouter-un-type-dennemi)).
- **Un nouveau comportement** : un mouvement ou une attaque qu'aucun type intégré n'a,
  comme de nouvelles phases d'attaque de boss (`server/src/systems/BossSystem.cpp`).

## Code

- [`server/include/EnemyCatalog.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/EnemyCatalog.hpp) :
  l'`EnemyCatalog`, rempli une fois au démarrage puis lu par toutes les salles ;
  `parseEnemies()` et `loadEnemies()` lisent le fichier (`server/src/EnemyFile.cpp`).
- [`server/include/EnemyDefs.hpp`](https://github.com/T3a1/RType-mirror/blob/dev/server/include/EnemyDefs.hpp) :
  les statistiques `EnemyDef` et les valeurs par défaut des types intégrés (`ENEMY_DEFS`).
- Chaque ennemi garde son type dans `rtype::Enemy::kind` ; `enemyDef(enemy)` donne ses
  statistiques.
