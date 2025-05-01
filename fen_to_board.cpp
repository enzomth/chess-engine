#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <map>

const int TILE_SIZE = 64;
const int BOARD_SIZE = 8;
const int WINDOW_SIZE = BOARD_SIZE * TILE_SIZE;

// Dictionnaire pour associer les caractères de pièces FEN aux types de pièces
std::map<char, std::string> piece_map = {
    {'p', "bpawn"}, {'n', "bknight"}, {'b', "bbishop"}, {'r', "brook"},
    {'q', "bqueen"}, {'k', "bking"}, {'P', "wpawn"}, {'N', "wknight"},
    {'B', "wbishop"}, {'R', "wrook"}, {'Q', "wqueen"}, {'K', "wking"}
};

// Fonction pour charger et redimensionner les images des pièces
SDL_Texture* load_and_resize_piece(SDL_Renderer* renderer, const std::string& image_path) {
    SDL_Surface* surface = IMG_Load(image_path.c_str());
    if (!surface) {
        std::cerr << "Erreur de chargement de l'image : " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Fonction pour charger l'échiquier à partir de la notation FEN
std::vector<std::vector<std::string>> load_board_from_fen(const std::string& fen) {
    std::vector<std::vector<std::string>> board(BOARD_SIZE, std::vector<std::string>(BOARD_SIZE, ""));
    int row = 0, col = 0;
    for (char c : fen) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (isdigit(c)) {
            col += c - '0';  // Saute les cases vides
        } else if (piece_map.find(c) != piece_map.end()) {
            board[row][col] = piece_map[c];
            col++;
        }
    }
    return board;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Erreur : Aucun FEN fourni." << std::endl;
        return 1;
    }

    std::string fen = argv[1];

    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur d'initialisation de SDL : " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Chess Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Erreur de création de la fenêtre : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Erreur de création du renderer : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Charger les textures des pièces
    std::map<std::string, SDL_Texture*> piece_images = {
        {"wpawn", load_and_resize_piece(renderer, "images/white-pawn.png")},
        {"wknight", load_and_resize_piece(renderer, "images/white-knight.png")},
        {"wbishop", load_and_resize_piece(renderer, "images/white-bishop.png")},
        {"wrook", load_and_resize_piece(renderer, "images/white-rook.png")},
        {"wqueen", load_and_resize_piece(renderer, "images/white-queen.png")},
        {"wking", load_and_resize_piece(renderer, "images/white-king.png")},
        {"bpawn", load_and_resize_piece(renderer, "images/black-pawn.png")},
        {"bknight", load_and_resize_piece(renderer, "images/black-knight.png")},
        {"bbishop", load_and_resize_piece(renderer, "images/black-bishop.png")},
        {"brook", load_and_resize_piece(renderer, "images/black-rook.png")},
        {"bqueen", load_and_resize_piece(renderer, "images/black-queen.png")},
        {"bking", load_and_resize_piece(renderer, "images/black-king.png")}
    };

    // Charger l'échiquier à partir de la notation FEN
    std::vector<std::vector<std::string>> board = load_board_from_fen(fen);

    // Boucle principale du jeu
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_RenderClear(renderer);

        // Dessiner l'échiquier
        for (int row = 0; row < BOARD_SIZE; row++) {
            for (int col = 0; col < BOARD_SIZE; col++) {
                SDL_Rect tile = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_SetRenderDrawColor(renderer, (row + col) % 2 == 0 ? 240 : 181, (row + col) % 2 == 0 ? 217 : 136, (row + col) % 2 == 0 ? 181 : 99, 255);
                SDL_RenderFillRect(renderer, &tile);

                // Dessiner la pièce
                std::string piece = board[row][col];
                if (!piece.empty()) {
                    SDL_Rect dest = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                    SDL_RenderCopy(renderer, piece_images[piece], NULL, &dest);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Libérer les ressources et fermer SDL
    for (auto& pair : piece_images) {
        SDL_DestroyTexture(pair.second);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
