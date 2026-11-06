# Tester sur un réseau dégradé

Ce guide montre comment jouer à R-Type sur une seule machine comme si les joueurs étaient
loin du serveur : latence ajoutée, gigue, perte de paquets, duplication et faible débit.
Il utilise `tc netem`, l'émulateur réseau de Linux, sur l'interface de boucle locale, et
ne ralentit que le trafic du jeu (port UDP 4242). Tout le reste de la machine garde sa
vitesse normale.

## Sommaire

- [1. Prérequis](#1-prérequis)
- [2. Démarrage rapide](#2-démarrage-rapide)
- [3. Scénarios](#3-scénarios)
- [4. Ce qu'il faut vérifier](#4-ce-quil-faut-vérifier)
- [5. Dépannage](#5-dépannage)

## 1. Prérequis

- **Linux**, avec `tc` (paquet `iproute2`, installé sur presque toutes les distributions).
- Les droits **sudo** : modifier les règles réseau demande d'être root.
- Un projet compilé (voir le [guide d'installation](INSTALLATION.md)).

## 2. Démarrage rapide

**1. Mettez en place les règles** (une seule fois). La première commande crée les files,
les deux suivantes envoient le trafic UDP du jeu vers la file netem, la dernière fixe les
conditions réseau :

```bash
sudo tc qdisc add dev lo root handle 1: prio
sudo tc filter add dev lo parent 1: protocol ip u32 match ip protocol 17 0xff match ip dport 4242 0xffff flowid 1:3
sudo tc filter add dev lo parent 1: protocol ip u32 match ip protocol 17 0xff match ip sport 4242 0xffff flowid 1:3
sudo tc qdisc add dev lo parent 1:3 handle 30: netem delay 50ms loss 5%
```

**2. Jouez** avec le serveur natif (en mode Docker, le trafic ne passe pas seulement par
`lo`, donc les règles peuvent ne pas s'appliquer) :

```bash
./rtype.py --native -c 2
```

**3. Changez les conditions** à tout moment, même en pleine partie : voir
[3. Scénarios](#3-scénarios).

**4. Retirez tout** quand vous avez fini :

```bash
sudo tc qdisc del dev lo root
```

> **Attention :** les règles restent actives jusqu'à ce que vous les retiriez ou que vous
> redémarriez. N'oubliez pas l'étape 4.

## 3. Scénarios

Chaque scénario est une commande qui remplace les conditions en cours. Elle s'applique
immédiatement, sans relancer le jeu.

`change` remplace **tous** les réglages : un réglage omis revient à zéro. Pour combiner
latence et perte, mettez les deux sur la même ligne.

Sur `lo`, chaque paquet subit le délai une seule fois : `delay 50ms` donne un temps
d'aller-retour (RTT) d'environ 100 ms.

| Scénario | Commande |
|----------|----------|
| Réseau normal | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 0ms` |
| Latence (RTT ~100 ms) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms` |
| Forte latence (RTT ~300 ms) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 150ms` |
| Gigue (50 ms ± 20 ms, paquets réordonnés) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms 20ms` |
| 2 % de perte de paquets | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss 2%` |
| 5 % de perte de paquets | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss 5%` |
| 20 % de perte de paquets | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss 20%` |
| Perte en rafales | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss gemodel 1% 30%` |
| 2 % de duplication de paquets | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms duplicate 2%` |
| Faible débit (5 ko/s) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms rate 40kbit` |
| Coupure de connexion | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem loss 100%` |

La « perte en rafales » utilise le modèle de Gilbert-Elliott : chaque paquet a 1 % de
chances de démarrer une rafale de pertes, et chaque paquet perdu 30 % de chances d'y
mettre fin. C'est plus proche d'un vrai réseau Wi-Fi ou mobile qu'une perte aléatoire.

Le « faible débit » limite les deux sens ensemble, car ils partagent cette unique file
sur `lo` : `rate` est en bits par seconde, donc 40 kbit/s font 5 ko/s, en-têtes de paquets
compris. Ce qui ne passe pas attend dans la file, jusqu'à 1000 paquets : une surcharge se
voit donc comme un délai croissant plutôt que comme des paquets perdus.

Pour une coupure, laissez-la quelques secondes, puis repassez à un autre scénario. Après
5 secondes sans aucun paquet, le client et le serveur considèrent la connexion perdue.

Pour voir les conditions actives (la ligne `netem`) :

```bash
tc qdisc show dev lo
```

## 4. Ce qu'il faut vérifier

| Scénario | Résultat attendu |
|----------|------------------|
| Tous | Aucun plantage du serveur ni des clients. |
| Latence, gigue | Les ennemis, les missiles et le vaisseau de l'autre joueur bougent de façon fluide, sans saccade ni retour en arrière. Votre vaisseau répond aux touches immédiatement. |
| Perte de paquets | Les entités continuent de bouger malgré les trous, sans téléportation visible. |
| Perte en rafales | Lors d'un long trou, une entité s'arrête, puis rattrape son retard en douceur. |
| Duplication de paquets | Aucune différence visible avec la même latence sans duplication : les doublons sont ignorés, et aucun événement n'arrive deux fois. |
| Faible débit | Voir les limites mesurées dans la [comparaison des performances réseau](NETWORK_COMPARISON.md#5-analyse). Aucun plantage, aucune déconnexion. |
| Coupure (moins de 5 s) | La partie reprend et tout se remet en place. |
| Coupure (plus de 5 s) | Le client affiche « Connection to the server lost », le serveur libère la place et prévient les autres joueurs. |

Pour des chiffres mesurés plutôt qu'une vérification à l'œil, la
[comparaison des performances réseau](NETWORK_COMPARISON.md) exécute ces scénarios
automatiquement. Pour comparer à l'œil avec le prototype de la partie 1, compilez le tag
`v1.0` dans un dossier séparé et lancez les mêmes scénarios sur les deux versions :

```bash
git worktree add ../rtype-v1 v1.0
```

## 5. Dépannage

| Erreur | Cause et solution |
|--------|-------------------|
| `Error: Exclusivity flag on, cannot modify.` ou `RTNETLINK answers: File exists` | Les règles sont déjà en place. Retirez-les (`sudo tc qdisc del dev lo root`) et reprenez à l'étape 1, ou utilisez seulement `change`. |
| `Error: Specified qdisc kind is unknown.` | Le module noyau netem n'est pas chargé : `sudo modprobe sch_netem`. |
| `RTNETLINK answers: No such file or directory` sur `change` ou `del` | Les règles ne sont pas en place (ou ont déjà été retirées) : faites d'abord l'étape 1. |
| Le jeu n'est pas ralenti | Vérifiez que le serveur tourne avec `--native` et écoute sur le port 4242, que `tc qdisc show dev lo` affiche la ligne netem et `tc filter show dev lo` les deux filtres. |
