## Build, run and Debug Half Life 2 on Ubuntu 24.04

https://www.jamesfmackenzie.com/howto/how-to-install-half-life-2-halflife-2-on-raspberry-pi/

### dependencies

```sh
sudo apt install build-essential pkg-config ccache libsdl2-dev libfontconfig1-dev libopenal-dev libjpeg-dev libpng-dev libcurl4-gnutls-dev libbz2-dev libedit-dev
```
### build

```sh
python3 ./waf configure -T debug --prefix=hl2 --build-games=hl2 --disable-warns

python3 ./waf build -p -v 

python3 ./waf install
```

### game asset

```sh
wget https://github.com/SteamRE/DepotDownloader/releases/download/DepotDownloader_3.4.0/DepotDownloader-linux-x64.zip
unzip DepotDownloader-linux-x64.zip

./DepotDownloader -app 220 -depot 221 -manifest 3666218991449795038 -username <steam_username>
```

```sh
rsync -ah --progress depots/221/19307283/hl2/* source-engine/hl2/hl2
scp -r depots/221/19307283/platform source-engine/hl2/
```