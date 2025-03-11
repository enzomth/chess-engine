#include "./include/chess.hpp"
#include "math.h"
#include "map"
using namespace chess;

std::map<chess::PieceType, int> piece_values = {
    {chess::PieceType::PAWN, 1},
    {chess::PieceType::KNIGHT, 3},
    {chess::PieceType::BISHOP, 3},
    {chess::PieceType::ROOK, 5},
    {chess::PieceType::QUEEN, 9},
    {chess::PieceType::KING, 0}
};

int evaluate_move(const chess::Board &board, const chess::Move &move) {
    // Vérifie si c'est une capture
    if (board.isCapture(move)) {
        chess::Piece captured_piece = board.at(move.to());
        return piece_values[captured_piece.type()];  // Retourne la valeur de la pièce capturée
    }
    return 0;  // Si ce n'est pas une capture, la valeur est 0
}

chess::Move best_move(chess::Board &board) {
    chess::Move best_move;
    Movelist moves;
    movegen::legalmoves(moves, board);
    int best_value = -1;
    for (const auto &move : moves) {
        
        int value = evaluate_move(board, move);
        if (value > best_value) {
            best_value = value;
            best_move = move;
        }  
    }
    return best_move;
}

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for(int i=0; i<20; i++){
    chess::Move move = best_move(board);
    std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;

    board.makeMove(move);
}
std::cout << "FEN  : " << board.getFen() << std::endl;

    return 0;
}