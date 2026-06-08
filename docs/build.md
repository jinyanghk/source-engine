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

### run

```sh
cd hl2
./hl2_launcher
```

### other source games

* hl1 = Half-Life 1: Source
* hl2 = Half-Life 2
* episodic = Half-Life 2 Episode 1 and 2
* hl2mp = Half-Life 2: Deathmatch
* dod = Day of Defeat
* cstrike = Counter-Strike: Source
* portal = Portal

```sh
python3 ./waf configure -T debug --prefix=portal --build-games=portal --disable-warns

python3 ./waf build -p -v 

python3 ./waf install

cd portal

./hl2_launcher -game portal
```

### clean
```sh
python3 ./waf clean
```