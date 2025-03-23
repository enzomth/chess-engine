import pygame
import sys

# Constantes pour la taille de la fenêtre et de la case
TILE_SIZE = 64
BOARD_SIZE = 8
WINDOW_SIZE = BOARD_SIZE * TILE_SIZE

# Dictionnaire pour associer les caractères de pièces FEN aux types de pièces
piece_map = {
    'p': 'bpawn', 'n': 'bknight', 'b': 'bbishop', 'r': 'brook', 'q': 'bqueen', 'k': 'bking',
    'P': 'wpawn', 'N': 'wknight', 'B': 'wbishop', 'R': 'wrook', 'Q': 'wqueen', 'K': 'wking'
}

# Fonction pour charger et redimensionner les images des pièces
def load_and_resize_piece(image_path):
    piece_image = pygame.image.load(image_path)
    piece_image = pygame.transform.scale(piece_image, (TILE_SIZE, TILE_SIZE))
    return piece_image

# Fonction pour charger l'échiquier à partir de la notation FEN
def load_board_from_fen(fen):
    fen = fen.split(' ')[0]
    board = [[None for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
    row, col = 0, 0
    for char in fen:
        if char == '/':
            row += 1
            col = 0
        elif char.isdigit():
            col += int(char)  # Saute les cases vides
        elif char in piece_map:
            board[row][col] = piece_map[char]
            col += 1
    return board

def main():
    if len(sys.argv) < 2:
        print("Erreur: Aucun FEN fourni.")
        sys.exit(1)

    # Récupérer le FEN depuis les arguments de la ligne de commande
    fen = sys.argv[1]

    pygame.init()

    # Créer la fenêtre
    window = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Chess Board")

    # Charger toutes les textures de pièces (en supposant que les images sont dans un dossier "images")
    piece_images = {
        'wpawn': load_and_resize_piece("images/white-pawn.png"),
        'wknight': load_and_resize_piece("images/white-knight.png"),
        'wbishop': load_and_resize_piece("images/white-bishop.png"),
        'wrook': load_and_resize_piece("images/white-rook.png"),
        'wqueen': load_and_resize_piece("images/white-queen.png"),
        'wking': load_and_resize_piece("images/white-king.png"),
        'bpawn': load_and_resize_piece("images/black-pawn.png"),
        'bknight': load_and_resize_piece("images/black-knight.png"),
        'bbishop': load_and_resize_piece("images/black-bishop.png"),
        'brook': load_and_resize_piece("images/black-rook.png"),
        'bqueen': load_and_resize_piece("images/black-queen.png"),
        'bking': load_and_resize_piece("images/black-king.png")
    }

    # Charger l'échiquier à partir de la notation FEN
    board = load_board_from_fen(fen)
    font = pygame.font.SysFont("Arial", 12)
    # Boucle principale du jeu
    while True:
        window.fill((255, 255, 255))  # Fond blanc

        # Dessiner l'échiquier (avec couleurs alternées)
        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                # Calculer la couleur de la case
                tile_color = (240, 217, 181) if (row + col) % 2 == 0 else (181, 136, 99)
                pygame.draw.rect(window, tile_color, (col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE))

                # Dessiner la pièce si présente
                piece = board[row][col]
                if piece:
                    piece_image = piece_images[piece]
                    window.blit(piece_image, (col * TILE_SIZE, row * TILE_SIZE))
        
        # Dessiner les annotations pour les colonnes (a, b, c, ...)
        for col in range(BOARD_SIZE):
            text = font.render(chr(ord('a') + col), True, (0, 0, 0))  # Colonnes a, b, c, ...
            window.blit(text, ((col) * TILE_SIZE + TILE_SIZE // 2 - text.get_width() // 2, WINDOW_SIZE -15))

        # Dessiner les annotations pour les rangées (1, 2, 3, ...)
        for row in range(BOARD_SIZE):
            text = font.render(str(BOARD_SIZE - row), True, (0, 0, 0))  # Rangées 1, 2, 3, ...
            window.blit(text, (5, (row) * TILE_SIZE + TILE_SIZE // 2 - text.get_height() // 2))
        
        # Gérer les événements
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        pygame.display.flip()

if __name__ == "__main__":
    main()
