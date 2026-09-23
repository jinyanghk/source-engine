```sh
ulimit -c

ulimit -c unlimited
sudo sysctl -w kernel.core_pattern=core

ulimit -c 0
```

### debug

| breakpoint | file | description |
| ---------- | ---- | ---------- |
| CBasePlayer::Spawn | game/server/player.cpp | Called everytime the player respawns |

### .gdbinit

```sh
nano ~/.gdbninit
```

```
set debuginfod enabled off
handle SIGPIPE nostop noprint
```