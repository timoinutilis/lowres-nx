Build for LINUX
===============

## Installing Dependencies

### Ubuntu
> Tested with Ubuntu 18.04

```bash
sudo apt install git make build-essential libsdl2-dev
```

### Debian
> Tested with Debian 11 (Bullseye)

```bash
sudo apt install git make build-essential libsdl2-dev libdrm-dev libgbm-dev
```

### Arch Linux
> Tested with Arch Linux (x86_64, kernel version 6.9.3-arch1-1)

```bash
sudo pacman -S git make base-devel sdl2
```

## Building and Running
These steps have been tested on all the distributions listed above and work without modification.
```bash
git clone https://github.com/timoinutilis/lowres-nx.git
cd lowres-nx/platform/Linux/
make
./output/LowResNX
```
