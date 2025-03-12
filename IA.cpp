#include "IA.h"
#include "./include/chess.hpp"
#include "math.h"
#include "map"
#include <random>
using namespace chess;

namespace IA{

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
    std::random_device rd;
    std::mt19937 gen(rd());

    chess::Move best_move;
    Movelist moves;
    Movelist best_moves;
    movegen::legalmoves(moves, board);


    int best_value = -1;
    for (const auto &move : moves) {

        int value = evaluate_move(board, move);
        if (value > best_value) {
            best_moves.clear();
            best_value = value;
            best_moves.add(move);
        }
        else if (value == best_value) {
            best_moves.add(move);
        }
    }

    std::uniform_int_distribution<> distrib(0, best_moves.size());
    int rdm = distrib(gen);

    return best_moves[rdm];
}

} //namesapce IA