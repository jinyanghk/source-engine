### game asset

| game | app | depot | manifest |
| ---- | --- | ----- | -------- |
| Half Life 2 | 220 | 221 | 3666218991449795038 |
| Portal | 400 | 401 | 3566636281151658894 |
| Counter Strike: Source | 240 | 241 | 3261854995366250836 |

```sh
wget https://github.com/SteamRE/DepotDownloader/releases/download/DepotDownloader_3.4.0/DepotDownloader-linux-x64.zip
unzip DepotDownloader-linux-x64.zip

./DepotDownloader -app 220 -depot 221 -manifest 3666218991449795038 -username <steam_username>
```

```sh
rsync -ah --progress depots/221/19307283/hl2/* source-engine/hl2/hl2
cp -r depots/221/19307283/platform source-engine/hl2/
```

```sh
./DepotDownloader -app 400 -depot 401 -manifest 3566636281151658894 -username <steam_username>

rsync -ah --progress depots/401/19017868/* source-engine/portal
```

```sh
./DepotDownloader -app 240 -depot 241 -manifest 3261854995366250836 -username <steam_username>

rsync -ah --progress depots/241/17399420/* source-engine/cstrike
```