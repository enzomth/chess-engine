#include "./include/chess.hpp"
#include "math.h"
#include "map"

using namespace chess;
#define DEPTH 5
int nb_pos = 0;
std::map<chess::PieceType, int> piece_values = {
    {chess::PieceType::PAWN, 10},
    {chess::PieceType::KNIGHT, 30},
    {chess::PieceType::BISHOP, 30},
    {chess::PieceType::ROOK, 50},
    {chess::PieceType::QUEEN, 90},
    {chess::PieceType::KING, 1000}
};

int nbPos = 0;

int evaluate_move(const chess::Board &board, const chess::Move &move) {
    nb_pos ++;
    chess::Board newBoard = board;
    newBoard.makeMove(move);
    std::pair<chess::GameResultReason, chess::GameResult> result = newBoard.isGameOver();

    if (result.first == chess::GameResultReason::CHECKMATE) {
        return 10000;
    }

    if (board.isCapture(move)) {
        chess::Piece captured_piece = board.at(move.to());
        int value = piece_values[captured_piece.type()];
        return value;
    }
    return 0;
}




int evaluate(const chess::Board &board) {
    
    
    
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
        
        
            value += whitePieces.count() *piece_values[pieceType];
            value -= blackPieces.count() *piece_values[pieceType];
    }

    return value;
}

int minmax(chess::Board board, int depth, bool isMaximizingPlayer){
    
    if (depth == 0) {
        return 0;  //on s'arrete ici
    }
    
    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.empty()) {
        return 0;  // Pas de coups possibles
    }

    if (isMaximizingPlayer) {
        int maxEval = -100000;
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval =  (evaluate_move(board, move) + minmax(newBoard, depth - 1, false))-depth;
            maxEval = std::max(maxEval, eval);
        }
        return maxEval;
    }
    
    else {
        int minEval = 100000;
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval =  - evaluate_move(board, move) + minmax(newBoard, depth - 1, true);
            minEval = std::min(minEval, eval);
        }
        return minEval;
    }

}

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
        
        int eval = evaluate_move(board,move) + minmaxAlphaBeta(newBoard, depth - 1, false,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()); // Appel MinMax pour évaluer ce coup
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

    std::cout << "nb pos : "<<nbPos << std::endl;
    std::cout << "FEN  : " << board.getFen() << std::endl;

    return 0;
}