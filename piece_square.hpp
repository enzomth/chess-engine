#include <iostream>
#include "./include/chess.hpp"

#define RANK_NB 8
#define FILE_NB 8
#define MAX_PHASE 18812
#define PST_SIZE 8




int piece_values[] = {136,781,825,1276,2538,0};


// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
// type on a given square a (middlegame, endgame) score pair is assigned. Table
// is defined for files A..D and white side: it is symmetric for black side and
// second half of the files.


int Bonus[6][RANK_NB][FILE_NB / 2][2] = {
  {},
  { // Knight
   { {-175, -96}, {-92,-65}, {-74,-49}, {-73,-21} },
   { { -77, -67}, {-41,-54}, {-27,-18}, {-15,  8} },
   { { -61, -40}, {-17,-27}, {  6, -8}, { 12, 29} },
   { { -35, -35}, {  8, -2}, { 40, 13}, { 49, 28} },
   { { -34, -45}, { 13,-16}, { 44,  9}, { 51, 39} },
   { {  -9, -51}, { 22,-44}, { 58,-16}, { 53, 17} },
   { { -67, -69}, {-27,-50}, {  4,-51}, { 37, 12} },
   { {-201,-100}, {-83,-88}, {-56,-56}, {-26,-17} }
  },
  { // Bishop
   { {-53,-57}, { -5,-30}, { -8,-37}, {-23,-12} },
   { {-15,-37}, {  8,-13}, { 19,-17}, {  4,  1} },
   { { -7,-16}, { 21, -1}, { -5, -2}, { 17, 10} },
   { { -5,-20}, { 11, -6}, { 25,  0}, { 39, 17} },
   { {-12,-17}, { 29, -1}, { 22,-14}, { 31, 15} },
   { {-16,-30}, {  6,  6}, {  1,  4}, { 11,  6} },
   { {-17,-31}, {-14,-20}, {  5, -1}, {  0,  1} },
   { {-48,-46}, {  1,-42}, {-14,-37}, {-23,-24} }
  },
  { // Rook
   { {-31, -9}, {-20,-13}, {-14,-10}, {-5, -9} },
   { {-21,-12}, {-13, -9}, { -8, -1}, { 6, -2} },
   { {-25,  6}, {-11, -8}, { -1, -2}, { 3, -6} },
   { {-13, -6}, { -5,  1}, { -4, -9}, {-6,  7} },
   { {-27, -5}, {-15,  8}, { -4,  7}, { 3, -6} },
   { {-22,  6}, { -2,  1}, {  6, -7}, {12, 10} },
   { { -2,  4}, { 12,  5}, { 16, 20}, {18, -5} },
   { {-17, 18}, {-19,  0}, { -1, 19}, { 9, 13} }
  },
  { // Queen
   { { 3,-69}, {-5,-57}, {-5,-47}, { 4,-26} },
   { {-3,-55}, { 5,-31}, { 8,-22}, {12, -4} },
   { {-3,-39}, { 6,-18}, {13, -9}, { 7,  3} },
   { { 4,-23}, { 5, -3}, { 9, 13}, { 8, 24} },
   { { 0,-29}, {14, -6}, {12,  9}, { 5, 21} },
   { {-4,-38}, {10,-18}, { 6,-12}, { 8,  1} },
   { {-5,-50}, { 6,-27}, {10,-24}, { 8, -8} },
   { {-2,-75}, {-2,-52}, { 1,-43}, {-2,-36} }
  },
  { // King
   { {271,  1}, {327, 45}, {271, 85}, {198, 76} },
   { {278, 53}, {303,100}, {234,133}, {179,135} },
   { {195, 88}, {258,130}, {169,169}, {120,175} },
   { {164,103}, {190,156}, {138,172}, { 98,172} },
   { {154, 96}, {179,166}, {105,199}, { 70,199} },
   { {123, 92}, {145,172}, { 81,184}, { 31,191} },
   { { 88, 47}, {120,121}, { 65,116}, { 33,131} },
   { { 59, 11}, { 89, 59}, { 45, 73}, { -1, 78} }
  }
};

  int PBonus[RANK_NB][FILE_NB][2] =
  { // Pawn {asymmetric distribution}
   { },
   { {  3,-10}, {  3, -6}, { 10, 10}, { 19,  0}, { 16, 14}, { 19,  7}, {  7, -5}, { -5,-19} },
   { { -9,-10}, {-15,-10}, { 11,-10}, { 15,  4}, { 32,  4}, { 22,  3}, {  5, -6}, {-22, -4} },
   { { -4,  6}, {-23, -2}, {  6, -8}, { 20, -4}, { 40,-13}, { 17,-12}, {  4,-10}, { -8, -9} },
   { { 13, 10}, {  0,  5}, {-13,  4}, {  1, -5}, { 11, -5}, { -2, -5}, {-13, 14}, {  5,  9} },
   { {  5, 28}, {-12, 20}, { -7, 21}, { 22, 28}, { -8, 30}, { -5,  7}, {-15,  6}, { -8, 13} },
   { { -7,  0}, {  7,-11}, { -3, 12}, {-13, 21}, {  5, 25}, {-16, 19}, { 10,  4}, { -8,  7} }
  };

int eval(chess::Board board){
    int row, col;
    int partie = 0;
    int phase = 0;
    int middlegame_score = 0;
    int endgame_score = 0;

    chess::Color white = chess::Color::WHITE;
    chess::Color black = chess::Color::BLACK;

    // Ajouter la valeur des pièces du joueur actif
    for (auto pieceType : {chess::PieceType::underlying::PAWN, chess::PieceType::underlying::KNIGHT, chess::PieceType::underlying::BISHOP,
      chess::PieceType::underlying::ROOK, chess::PieceType::underlying::QUEEN, chess::PieceType::underlying::KING}) {


        chess::Bitboard whitePieces = board.pieces(pieceType, white);
        chess::Bitboard blackPieces = board.pieces(pieceType, black);

        int pieceTypeIndex = static_cast<int>(pieceType);

        if(pieceType == chess::PieceType::PAWN){
          for (int i = 0; i < 64; ++i) {
            if (whitePieces & (1ULL << i)) {
                row = i / PST_SIZE;  // L'index de la ligne
                col = i % PST_SIZE;  // L'index de la colonne
                middlegame_score += Bonus[pieceTypeIndex][row][col][0];  // Ajouter la valeur d'ouverture
                endgame_score += Bonus[pieceTypeIndex][row][col][1];
            }
        }
        for (int i = 0; i < 64; ++i) {
          if (blackPieces & (1ULL << i)) {
              row = 7 - (i / PST_SIZE);  // L'index de la ligne pour les pièces noires (symétrie)
              col = i % PST_SIZE;
              middlegame_score -= Bonus[pieceTypeIndex][row][col][0];  // Ajouter la valeur d'ouverture
              endgame_score -= Bonus[pieceTypeIndex][row][col][1];
          }
      }
      }
      else{

        
        // Pour les pièces blanches
        for (int i = 0; i < 64; ++i) {
            if (whitePieces & (1ULL << i)) {
                row = i / PST_SIZE;  // L'index de la ligne
                col = i % PST_SIZE;  // L'index de la colonne
                middlegame_score += Bonus[pieceTypeIndex][row][col%4][0];  // Ajouter la valeur d'ouverture
                endgame_score += Bonus[pieceTypeIndex][row][col%4][1];
            }
        }

        // Pour les pièces noires
        for (int i = 0; i < 64; ++i) {
            if (blackPieces & (1ULL << i)) {
                row = 7 - (i / PST_SIZE);  // L'index de la ligne pour les pièces noires (symétrie)
                col = i % PST_SIZE;
                middlegame_score -= Bonus[pieceTypeIndex][row][col%4][0];  // Ajouter la valeur d'ouverture
                endgame_score -= Bonus[pieceTypeIndex][row][col%4][1];
            }
        }
      }
    

        // Ajouter la valeur de la pièce elle-même (dépend de la phase du jeu)
        phase += whitePieces.count() * piece_values[pieceTypeIndex]
                 + blackPieces.count() * piece_values[pieceTypeIndex];


        

        partie += whitePieces.count() * piece_values[pieceTypeIndex]
                 - blackPieces.count() * piece_values[pieceTypeIndex];
    }

    // Calcul de l'évaluation en fonction de la phase du jeu (calculer les scores d'ouverture et de fin de partie)

    int eval = partie + (int)((phase * middlegame_score + (MAX_PHASE - phase) * endgame_score) / MAX_PHASE);

    return eval;
}