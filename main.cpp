#include "./include/chess.hpp"
#include "piece_square.hpp"
#include "math.h"
#include "map"

using namespace chess;
#define DEPTH 1
#define PST_SIZE 8
int nb_pos = 0;
std::map<chess::PieceType,int, int> piece_values = {
    {chess::PieceType::PAWN, 136,208},
    {chess::PieceType::KNIGHT,781,854},
    {chess::PieceType::BISHOP, 825,915},
    {chess::PieceType::ROOK, 1276,1380},
    {chess::PieceType::QUEEN, 2538,2682}
};





int evaluate(const chess::Board &board) {
    
    
    int row;
    int col;
    std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();

    if (result.first == chess::GameResultReason::CHECKMATE) {
        return 10000;
    }

    int value = 0;
    chess::Color white = chess::Color::WHITE;
    chess::Color black = chess::Color::BLACK;



    // Ajouter la valeur des pièces du joueur actif
    for (auto pieceType : {chess::PieceType::PAWN, chess::PieceType::KNIGHT, chess::PieceType::BISHOP, 
                           chess::PieceType::ROOK, chess::PieceType::QUEEN, chess::PieceType::KING}) {
        Bitboard whitePieces = board.pieces(pieceType, white);
        Bitboard blackPieces = board.pieces(pieceType, black);
        
        for (int i = 0; i < 64; ++i) {
            if (whitePieces & (1ULL << i)) {
                row = i / PST_SIZE;  // L'index de la ligne est l'index de bit divisé par la taille de l'échiquier
                col = i % PST_SIZE;
                value += get_placement_value(,row,col,pieceType,);

            }
        for (int i = 0; i < 64; ++i) {
            if (whitePieces & (1ULL << i)) {
                    row = 7 - (i / PST_SIZE);  // L'index de la ligne est l'index de bit divisé par la taille de l'échiquier
                    col = (i % PST_SIZE);
                    value += get_placement_value(row,col,pieceType);
    
            }

        
            value += whitePieces.count() *piece_values[pieceType];
            value -= blackPieces.count() *piece_values[pieceType];
    }

    return value;
}
}}



int minmaxAlphaBeta(chess::Board board, int depth, bool isMaximizingPlayer,int alpha, int beta) {
    if (depth == 0) {
        return evaluate(board);  //on s'arrete ici
    }

    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.empty()) {
        return 0;  // Pas de coups possibles : Mat
    }

    if (isMaximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta)-depth;
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, maxEval);
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    }

    else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval = minmaxAlphaBeta(newBoard, depth - 1, true,alpha,beta);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, minEval);
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}


chess::Move best_move(chess::Board &board, int depth) {
    Movelist moves;
    movegen::legalmoves(moves, board);
    
    if (moves.empty()) {
        throw std::runtime_error("Aucun coup disponible !");
    }

    int best_eval = -100000; // Une valeur très basse au départ
    chess::Move bestMove; // Initialisation avec un premier coup
    //char test[]  = "b7h7";
    //chess::Move move = uci::uciToMove(board,test);
    for (const auto &move : moves) {
        
        chess::Board newBoard = board;
        newBoard.makeMove(move);
        
        int eval = minmaxAlphaBeta(newBoard, depth - 1, false,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()); // Appel MinMax pour évaluer ce coup
        std::cout << eval<< " : " << uci::moveToUci(move) <<std::endl;

        if (eval > best_eval) {
            best_eval = eval;
            bestMove = move; // Mise à jour du meilleur coup trouvé
        }
    }
    
    return bestMove; // Retourne le meilleur coup trouvé
}

int main () {
    //Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq – 0 1");
    Board board = Board("3r1r2/1k1q3p/p5p1/1pQ3P1/8/PBRn4/1P5b/K3R3 w - - 0 1");

    for(int i=0; i<5; i++){
        chess::Move move = best_move(board,DEPTH);
        std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;

        board.makeMove(move);
    }


    std::cout << "FEN  : " << board.getFen() << std::endl;

    return 0;
}