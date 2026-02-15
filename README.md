# Fumbo Engine

Fumbo Engine is 2D video game engine using raylib/opengl as graphical backend

![last-commit](https://img.shields.io/github/last-commit/Fierys0/FumboEngine)
![repo-size](https://img.shields.io/github/repo-size/Fierys0/FumboEngine)
![license](https://img.shields.io/github/license/Fierys0/FumboEngine)

---

## DISCLAMER

This engine is still in development, engine only support 2d game for now. No engine editor as for now, its still being work on.

## Features

- raylib/opengl rendering
- 2d physics engine

---

## Build Requirements

### All Platforms

- **CMake ≥ 3.16**
- **C++17 compatible compiler**
  - GCC / Clang (GCC Recommended)
  - MinGW-w64 or MSVC

### Linux

Install raylib:

**Pacman**

```bash
sudo pacman -S raylib
```

**APT**

```bash
sudo apt install libraylib-dev
```

Install mpv

**Pacman**

```bash
sudo pacman -S mpv
```

**APT**

```bash
sudo apt install mpv
```

## Building the Project

```bash
mkdir build
cmake -G "Ninja" -S engine -B build
cmake --build build
```

---
