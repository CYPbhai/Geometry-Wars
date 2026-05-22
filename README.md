# Geometry Wars

A Geometry Wars-inspired arcade shooter built in C++ with SFML 3.0.2.

You fly around, shoot geometric enemies, and try to survive as long as possible. Enemies split into smaller pieces when killed. Pretty standard stuff for the genre, but it was a fun project to build from scratch.

---

## Features

- Entity Component System (ECS) architecture — entities are just IDs with attached components (transform, shape, collision, lifespan, score, input)
- Config file driven — window size, player stats, enemy settings, bullet properties are all read from `config.txt` so you can tweak things without recompiling
- Special weapon that charges up based on your score — fires a ring of bullets
- Enemies split into smaller versions on death, each with a lifespan before they fade out
- Fullscreen toggle at runtime (F11 key)
- Background music + sound effects (shoot, explosion, game over, etc.)
- ImGui debug panel for toggling systems on/off during play (movement, collision, spawning, lifespan)
- Camera shake on kills
- Score display and a special weapon charge bar on the HUD

---

## Controls

| Key / Button | Action |
|---|---|
| W A S D | Move |
| Left click (hold) | Shoot |
| Right click | Special weapon |
| P | Pause/Resume |
| M | Toggle music |
| F11 | Toggle fullscreen |
| Escape | Quit |

---

## Dependencies

- [SFML 3.0.2](https://www.sfml-dev.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [imgui-sfml](https://github.com/SFML/imgui-sfml)

The Libraries folder already contains SFML 3.0.2 and ImGui — you shouldn't need to install anything separately if you're using the Visual Studio solution.

---

## Building

Tested on Windows with Visual Studio 2022 (x64). Just open `GeometryWars.sln` and build. The project links against the bundled SFML libs and expects assets to be in an `assets/` folder next to the executable.

Asset folder structure expected:

```
assets/
  audio/
    background.mp3
    shoot.wav
    explosion.wav
    explosionBig.wav
    tap.wav
    gameover.wav
  fonts/
    (your font file, path set in config.txt)
```

---

## Config File

`config.txt` controls most of the game parameters. Example entries:

```
Window 1280 720 60 0
Font assets/fonts/myfont.ttf 24 255 255 255
Player 32 32 10 10 10 255 255 255 2 8 5.0
Enemy 32 32 255 255 255 2 3 8 90 120 3.0 6.0
Bullet 10 10 255 255 255 255 255 255 2 16 90 10.0
```

The fields for each line are documented in `Game.h`.

---

## Project Structure

```
src/
  main.cpp
  Game.h / Game.cpp      -- main game loop and all systems
  Entity.hpp             -- entity class with component access helpers
  EntityManager.hpp      -- handles entity creation and cleanup
  Components.hpp         -- all component definitions
  Vec2.hpp               -- simple 2D vector math
Libraries/
  SFML-3.0.2/
  imgui/
```

---

## Notes

This started as a follow-along project with Dave Churchill's game dev series on YouTube, then I kept adding things on top - audio, special weapon, camera shake, fullscreen, the debug panel. The core ECS design comes from his videos, the rest is mine.

Not trying to ship this anywhere, it's just something I built to get more comfortable with C++ and game architecture patterns.
