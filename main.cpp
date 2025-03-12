#include "./include/chess.hpp"
#include "math.h"
#include "map"
#include "IA.cpp"
using namespace chess;
using namespace IA;

int main () {
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for(int i=0; i<20; i++){
        chess::Move move = IA::best_move(board);
        std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;

        board.makeMove(move);
    }
    std::cout << "FEN  : " << board.getFen() << std::endl;

    return 0;
}