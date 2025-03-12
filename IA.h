//
// Created by isidore on 11/03/25.
//
#include "./include/chess.hpp"
#include "math.h"
#include "map"
#ifndef IA_H
#define IA_H



namespace IA {

    int evaluate_move(const chess::Board &board, const chess::Move &move);
    chess::Move best_move(chess::Board &board);

};



#endif //IA_H
