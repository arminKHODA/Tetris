# Tetris Game

This is a simple Tetris game built using SDL2 and SDL_ttf libraries. The objective of the game is to clear lines by filling them with Tetrominoes that fall from the top of the screen.

## Features

- Move and rotate Tetrominoes using keyboard inputs.
- Tetrominoes fall at a constant speed.
- Lines are cleared when they are completely filled.
- Score is kept based on the number of lines cleared.

## Requirements

- C++ compiler
- SDL2 library
- SDL_ttf library

## Installation

1. **Install SDL2**:
   - On Ubuntu:
     ```bash
     sudo apt-get install libsdl2-dev
     sudo apt-get install libsdl2-ttf-dev
     ```
   - On Windows:
     - Download the SDL2 development libraries from the [SDL website](https://www.libsdl.org/download-2.0.php) and follow the instructions for installation.
     - Download the SDL_ttf library from the [SDL_ttf website](https://www.libsdl.org/projects/SDL_ttf/) and follow the instructions for installation.

2. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/tetris-game.git
   cd tetris-game
