# Testing under a degraded network

This guide shows how to play R-Type on one machine as if the players were far
from the server: added latency, jitter and packet loss. It uses `tc netem`,
the Linux network emulator, on the loopback interface, and only slows down the
game's traffic (UDP port 4242). Everything else on the machine keeps its normal
speed.

## Contents

- [1. Requirements](#1-requirements)
- [2. Quick start](#2-quick-start)
- [3. Scenarios](#3-scenarios)
- [4. What to check](#4-what-to-check)
- [5. Troubleshooting](#5-troubleshooting)

## 1. Requirements

- **Linux**, with `tc` (package `iproute2`, installed on almost every
  distribution).
- **sudo** rights: changing the network rules needs root.
- A built project (see the [installation guide](INSTALLATION.md)).

## 2. Quick start

**1. Set up the rules** (once). The first command creates the queues, the next
two send the game's UDP traffic to the netem queue, the last one sets the
network conditions:

```bash
sudo tc qdisc add dev lo root handle 1: prio
sudo tc filter add dev lo parent 1: protocol ip u32 match ip protocol 17 0xff match ip dport 4242 0xffff flowid 1:3
sudo tc filter add dev lo parent 1: protocol ip u32 match ip protocol 17 0xff match ip sport 4242 0xffff flowid 1:3
sudo tc qdisc add dev lo parent 1:3 handle 30: netem delay 50ms loss 5%
```

**2. Play** with the native server (in Docker mode the traffic does not only go
through `lo`, so the rules may not apply):

```bash
./rtype.py --native -c 2
```

**3. Change the conditions** at any time, even during a game: see
[3. Scenarios](#3-scenarios).

**4. Remove everything** when you are done:

```bash
sudo tc qdisc del dev lo root
```

> [!WARNING]
> The rules stay active until you remove them or reboot. Do not forget step 4.

## 3. Scenarios

Every scenario is one command that replaces the current conditions. It takes
effect immediately, without restarting the game.

`change` replaces **all** the settings: a setting left out goes back to zero.
To combine latency and loss, put both on the same line.

On `lo`, each packet goes through the delay once: `delay 50ms` gives a round
trip time (RTT) of about 100 ms.

| Scenario | Command |
|----------|---------|
| Normal network | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 0ms` |
| Latency (RTT ~100 ms) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms` |
| High latency (RTT ~300 ms) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 150ms` |
| Jitter (50 ms ± 20 ms, packets reordered) | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms 20ms` |
| 5 % packet loss | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss 5%` |
| 20 % packet loss | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss 20%` |
| Loss in bursts | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem delay 50ms loss gemodel 1% 30%` |
| Connection cut | `sudo tc qdisc change dev lo parent 1:3 handle 30: netem loss 100%` |

"Loss in bursts" uses the Gilbert-Elliott model: each packet has a 1 % chance
of starting a burst of losses, and each lost packet a 30 % chance of ending it.
It is closer to a real Wi-Fi or mobile network than random loss.

For a connection cut, leave it a few seconds, then switch back to another
scenario. After 5 seconds without any packet, the client and the server
consider the connection lost.

To see the conditions currently active (the `netem` line):

```bash
tc qdisc show dev lo
```

## 4. What to check

| Scenario | Expected result |
|----------|-----------------|
| All | No crash of the server or the clients. |
| Latency, jitter | The enemies, the missiles and the other player's ship move smoothly, without stutter or going backwards. Your own ship answers the keys at once. |
| Packet loss | Entities keep moving through the gaps, without visible teleports. |
| Loss in bursts | During a long gap an entity stops, then catches up smoothly. |
| Connection cut (less than 5 s) | The game resumes and everything gets back in place. |
| Connection cut (more than 5 s) | The client shows "Connection to the server lost", the server frees the slot and tells the other players. |

To compare with the Part 1 prototype, build the `v1.0` tag in a separate folder
and run the same scenarios on both versions:

```bash
git worktree add ../rtype-v1 v1.0
```

## 5. Troubleshooting

| Error | Cause and fix |
|-------|---------------|
| `Error: Exclusivity flag on, cannot modify.` or `RTNETLINK answers: File exists` | The rules are already set up. Remove them (`sudo tc qdisc del dev lo root`) and start again from step 1, or only use `change`. |
| `Error: Specified qdisc kind is unknown.` | The netem kernel module is not loaded: `sudo modprobe sch_netem`. |
| `RTNETLINK answers: No such file or directory` on `change` or `del` | The rules are not set up (or were already removed): run step 1 first. |
| The game is not slower | Check that the server runs with `--native` and listens on port 4242, and that `tc qdisc show dev lo` lists the netem line and `tc filter show dev lo` the two filters. |
