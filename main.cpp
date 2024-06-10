#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

// Screen dimensions
const int SCREEN_WIDTH = 300; // 10 * 30 (GRID_WIDTH * BLOCK_SIZE)
const int SCREEN_HEIGHT = 600; // 20 * 30 (GRID_HEIGHT * BLOCK_SIZE)

// Grid dimensions
const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 20;
const int BLOCK_SIZE = 30;

// Game speed (lower is faster)
const int GAME_SPEED = 500;

// Tetromino shapes
const std::vector<std::vector<std::vector<int>>> tetrominoes = {
    { {1, 1, 1, 1} }, // I
    { {1, 1}, {1, 1} }, // O
    { {0, 1, 0}, {1, 1, 1} }, // T
    { {0, 1, 1}, {1, 1, 0} }, // S
    { {1, 1, 0}, {0, 1, 1} }, // Z
    { {1, 0, 0}, {1, 1, 1} }, // L
    { {0, 0, 1}, {1, 1, 1} } // J
};

struct Tetromino {
    std::vector<std::vector<int>> shape;
    int x, y;
    int rotation;

    Tetromino(int type) : shape(tetrominoes[type]), x(GRID_WIDTH / 2 - 1), y(0), rotation(0) {}

    void rotate() {
        std::vector<std::vector<int>> rotated(shape[0].size(), std::vector<int>(shape.size()));
        for (int i = 0; i < shape.size(); ++i) {
            for (int j = 0; j < shape[i].size(); ++j) {
                rotated[j][shape.size() - 1 - i] = shape[i][j];
            }
        }
        shape = rotated;
    }

    void rotateBack() {
        std::vector<std::vector<int>> rotated(shape[0].size(), std::vector<int>(shape.size()));
        for (int i = 0; i < shape.size(); ++i) {
            for (int j = 0; j < shape[i].size(); ++j) {
                rotated[shape[0].size() - 1 - j][i] = shape[i][j];
            }
        }
        shape = rotated;
    }
};

void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool isCollision(const std::vector<std::vector<int>>& grid, const Tetromino& tetromino) {
    for (int i = 0; i < tetromino.shape.size(); ++i) {
        for (int j = 0; j < tetromino.shape[i].size(); ++j) {
            if (tetromino.shape[i][j] && (tetromino.y + i >= GRID_HEIGHT || tetromino.x + j < 0 || tetromino.x + j >= GRID_WIDTH || grid[tetromino.y + i][tetromino.x + j])) {
                return true;
            }
        }
    }
    return false;
}

void mergeTetromino(std::vector<std::vector<int>>& grid, const Tetromino& tetromino) {
    for (int i = 0; i < tetromino.shape.size(); ++i) {
        for (int j = 0; j < tetromino.shape[i].size(); ++j) {
            if (tetromino.shape[i][j]) {
                grid[tetromino.y + i][tetromino.x + j] = tetromino.shape[i][j] + 1; // +1 to differentiate between active (1) and locked (2)
            }
        }
    }
}

void clearLines(std::vector<std::vector<int>>& grid, int& score) {
    for (int i = GRID_HEIGHT - 1; i >= 0; --i) {
        bool fullLine = true;
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (grid[i][j] == 0) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            grid.erase(grid.begin() + i);
            grid.insert(grid.begin(), std::vector<int>(GRID_WIDTH, 0));
            score += 100;
            ++i;
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "Could not initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Tetris",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("font.ttf", 24); // Provide the correct path to a TTF font file
    if (!font) {
        std::cerr << "Could not load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<std::vector<int>> grid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
    int score = 0;
    bool running = true;
    bool gameStarted = false;
    SDL_Event event;
    srand(static_cast<unsigned int>(time(0)));

    Tetromino current(rand() % tetrominoes.size());
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime, elapsedTime;

    while (running) {
        currentTime = SDL_GetTicks();
        elapsedTime = currentTime - lastTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (!gameStarted && event.key.keysym.sym == SDLK_RETURN) {
                    gameStarted = true;
                }
                else if (gameStarted) {
                    if (event.key.keysym.sym == SDLK_LEFT) {
                        current.x--;
                        if (isCollision(grid, current)) {
                            current.x++;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_RIGHT) {
                        current.x++;
                        if (isCollision(grid, current)) {
                            current.x--;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN) {
                        current.y++;
                        if (isCollision(grid, current)) {
                            current.y--;
                            mergeTetromino(grid, current);
                            clearLines(grid, score);
                            current = Tetromino(rand() % tetrominoes.size());
                            if (isCollision(grid, current)) {
                                gameStarted = false; // Game over
                            }
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_q) {
                        current.rotate();
                        if (isCollision(grid, current)) {
                            current.rotateBack();
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_e) {
                        current.rotateBack();
                        if (isCollision(grid, current)) {
                            current.rotate();
                        }
                    }
                }
            }
        }

        if (!gameStarted) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            RenderText(renderer, font, "Press Enter to Start", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, { 255, 255, 255, 255 });
            SDL_RenderPresent(renderer);
            continue;
        }

        if (elapsedTime > GAME_SPEED) {
            current.y++;
            if (isCollision(grid, current)) {
                current.y--;
                mergeTetromino(grid, current);
                clearLines(grid, score);
                current = Tetromino(rand() % tetrominoes.size());
                if (isCollision(grid, current)) {
                    gameStarted = false; // Game over
                }
            }
            lastTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < GRID_HEIGHT; ++i) {
            for (int j = 0; j < GRID_WIDTH; ++j) {
                if (grid[i][j]) {
                    SDL_Rect block = { j * BLOCK_SIZE, i * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                    if (grid[i][j] == 1) {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Active tetromino color
                    }
                    else {
                        SDL_SetRenderDrawColor(renderer, 169, 169, 169, 255); // Locked tetromino color (grey)
                    }
                    SDL_RenderFillRect(renderer, &block);
                }
            }
        }

        for (int i = 0; i < current.shape.size(); ++i) {
            for (int j = 0; j < current.shape[i].size(); ++j) {
                if (current.shape[i][j]) {
                    SDL_Rect block = { (current.x + j) * BLOCK_SIZE, (current.y + i) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE };
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Active tetromino color
                    SDL_RenderFillRect(renderer, &block);
                }
            }
        }

        std::stringstream scoreText;
        scoreText << "Score: " << score;
        RenderText(renderer, font, scoreText.str(), 10, 10, { 255, 255, 255, 255 });

        SDL_RenderPresent(renderer);
    }

    // Game Over screen
    while (true) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        RenderText(renderer, font, "Game Over! Press R to Restart or ESC to Exit", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, { 255, 255, 255, 255 });
        RenderText(renderer, font, "Final Score: " + std::to_string(score), SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 + 50, { 255, 255, 255, 255 });
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_r) {
                    // Reset the game
                    grid.assign(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
                    score = 0;
                    gameStarted = false;
                    current = Tetromino(rand() % tetrominoes.size());
                    lastTime = SDL_GetTicks();
                    running = true;
                }
            }
        }

        if (!running) {
            break;
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
