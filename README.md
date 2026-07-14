# Pong — Built from Scratch in C with raylib

https://github.com/user-attachments/assets/75947a8b-f3af-4295-b73d-84cefd364a01

A complete, playable Pong game written in pure C using [raylib](https://www.raylib.com/) as a thin rendering/audio layer — **no game engine**. I built this to understand what a game engine actually does for you by doing it all by hand: the game loop, timing, rendering, input, collision, and audio.

Rather than a finished product, this project is a deliberate exercise in **game development fundamentals** and low-level systems thinking.

## What it demonstrates

- **Custom game loop** with delta-time–based movement, so gameplay speed is decoupled from frame rate
- **Finite state machine** driving the game flow (Menu → Countdown → Playing → Game Over)
- **Physics & collision** from first principles — AABB collision, wall reflection, and paddle bounce angles that vary with where the ball strikes the paddle
- **A basic AI opponent** that tracks the ball and returns to center when idle
- **Resource & audio management** — loading, playing, and unloading sound effects with proper cleanup
- **Data-oriented design** using structs to model game objects (ball, paddles) and their state

## Tech

C · raylib · vector math · fixed-timestep game loop

## Controls

| Action | Key |
| --- | --- |
| Move left paddle | `W` / `S` |
| Right paddle | AI-controlled |
| Start game | `Space` |
| Toggle slow-motion | `Q` |

First to 10 points wins.
