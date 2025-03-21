#include <utility>
#include <chrono>

#include "./include/chess.hpp"
#include "piece_square.hpp"
#include "math.h"
#include "map"

using namespace chess;
#define DEPTH 4


int nb_pos = 0;


struct ValueMap {
    int value;
    int exact;
    int depth;
    std::string uci_move;
};

std::unordered_map<int, ValueMap> hashTable;

int evaluate(const chess::Board &board) {

    nb_pos ++;

    std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();

    if (result.first == chess::GameResultReason::CHECKMATE) {
        return 10000;
    }
    else{
        return eval(board);
    }

    
}



int minmaxAlphaBeta(chess::Board board, int depth, bool isMaximizingPlayer,int alpha, int beta) {

    //Vérification dans la table de hashage
    int hashBoard = board.hash();
    auto calculated_board = hashTable.find(hashBoard);

    if (calculated_board != hashTable.end()) {
        ValueMap value_stored = calculated_board->second;
        if (value_stored.depth > depth) {
            if (value_stored.exact == 0) {
                return value_stored.value;
            }
            if (value_stored.exact == -1 && value_stored.value >= beta) {
                return value_stored.value;
            }
            if (value_stored.exact == 1 && value_stored.value <= beta) {
                return value_stored.value;
            }
            if (value_stored.exact == 1) {
                beta = std::min(value_stored.value, beta);
            }
            else if (value_stored.exact == -1) {
                alpha = std::max(value_stored.value, alpha);
            }

        }
    }

    if (depth == 0) {
        return evaluate(board);  //on s'arrete ici
    }

    //Initialisation des valeurs a stocker
    ValueMap value_to_store;
    value_to_store.depth = depth;
    int type_exact;
    std::string uci_to_store;

    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.empty()) {
        return 0;  // Pas de coups possibles : Mat
    }


    //Tri le tableau des moves (marche super bien mais a optimiser si possible)
    std::stable_sort(moves.begin(),moves.end(),[board](chess::Move a,chess::Move b) -> bool {

        chess::Board board_a = board;
        chess::Board board_b = board;

        board_a.makeMove(a);
        board_b.makeMove(b);

        int hash_board_a = board_a.hash();
        int hash_board_b = board_b.hash();

        auto calculated_board_a = hashTable.find(hash_board_a);
        auto calculated_board_b = hashTable.find(hash_board_b);

        if (calculated_board_a != hashTable.end() && calculated_board_b != hashTable.end()) {
            return calculated_board_a->second.value < calculated_board_b->second.value;
        }

        return true;
    });




    if (isMaximizingPlayer) {

        type_exact = 1;

        int maxEval = std::numeric_limits<int>::min();
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta)-depth;

            //max
            if (eval > maxEval) {
                maxEval = eval;
                uci_to_store = uci::moveToUci(move);
            }


            //Alpha beta pruning
            if (beta <= maxEval) {
                type_exact = -1;
                break;
            }
            if (alpha < maxEval) {
                type_exact = 0;
                alpha = maxEval;
            }

        }

        value_to_store.exact = type_exact;
        value_to_store.value = maxEval;
        value_to_store.uci_move = uci_to_store;
        hashTable[hashBoard] = value_to_store;
        return maxEval;
    }

    else {

        type_exact = -1;

        int minEval = std::numeric_limits<int>::max();
        for (const auto &move : moves) {
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup
            int eval = minmaxAlphaBeta(newBoard, depth - 1, true,alpha,beta);

            //min
            if (eval < minEval) {
                minEval = eval;
                uci_to_store = uci::moveToUci(move);
            }

            //Alpha beta pruning
            if (minEval <= alpha) {
                type_exact = 1;
                break;
            }
            if (minEval < beta) {
                type_exact = 0;
                beta = minEval;
            }

        }

        value_to_store.exact = type_exact;
        value_to_store.value = minEval;
        value_to_store.uci_move = uci_to_store;
        hashTable[hashBoard] = value_to_store;
        return minEval;
    }
}

chess::Move best_move_iterative_deepening(chess::Board board,int time) {
    //Vérifie qu'il n'y a pas de mat
    Movelist moves;
    movegen::legalmoves(moves, board);
    chess::Move bestMove;

    if (moves.empty()) {
        throw std::runtime_error("Aucun coup disponible !");
    }
    hashTable.clear();

    //chrono pour arret
    auto start = std::chrono::high_resolution_clock::now();

    for (int depth = 1 ; depth <= std::numeric_limits<int>::max(); depth++ ) {
    //for (int depth = 1 ; depth <= 6; depth++ ) {

        //Arret par chrono
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (duration.count() > time) {
            break;
        }

        std::cout << "Current_depth: " << depth << std::endl;

        int best_eval = std::numeric_limits<int>::min(); // -inf au départ

        for (const auto &move : moves) {

            chess::Board newBoard = board;
            newBoard.makeMove(move);

            int eval = minmaxAlphaBeta(newBoard,depth,false,std::numeric_limits<int>::min(),std::numeric_limits<int>::max());

            std::cout << eval<< " : " << uci::moveToUci(move) <<std::endl;

            if (eval > best_eval) {
                best_eval = eval;
                bestMove = move; // Mise à jour du meilleur coup trouvé
            }
        }

    }

    return bestMove;
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
        
        int eval = minmaxAlphaBeta(newBoard,depth,false,std::numeric_limits<int>::min(),std::numeric_limits<int>::max());
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
    Board board = Board("8/P1Q1nk1p/5p2/4p3/4P2p/6P1/r4PKP/3q4 w - - 0 1");

    for(int i=0; i<1; i++){
        chess::Move move = best_move_iterative_deepening(board,5000);
        std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;

        board.makeMove(move);
    }

    std::cout << "nb pos : "<< nb_pos << std::endl;
    std::cout << "FEN  : " << board.getFen() << std::endl;


    return 0;
}