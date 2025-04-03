#include <utility>
#include <chrono>

#include "./include/chess.hpp"
#include "piece_square.hpp"
#include "math.h"
#include "map"

using namespace chess;
#define DEPTH 4


int nb_pos = 0;



int nb_coup_max_depth = 4;
int late_reduction = 2;

int test =0;
int current_depth = 0;

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
        return 1000000000000000;
    }
    else if (result.first != chess::GameResultReason::NONE) {
        return 0;
    }
    else{
        return eval(board);
    }

    
}

chess::Movelist sortMove(chess::Movelist& movelist,chess::Move bestMove,chess::Move killerMove1,chess::Move killerMove2) {
    chess::Movelist tmp;
    chess::Movelist movesSorted;

    chess::Move b = NULL;
    chess::Move k1 = NULL;
    chess::Move k2 = NULL;

    for (const auto &move : movelist) {
        if (move == bestMove){b=move;}
        else if (move == killerMove1){k1=move;}
        else if (move == killerMove2){k2=move;}
        else {
            tmp.add(move);
        }
    }

    if (b != NULL){movesSorted.add(b);}
    else if (k2 != NULL){movesSorted.add(k2);}
    else if (k1 != NULL){movesSorted.add(k1);}


    for (const auto &move : tmp) {
        movesSorted.add(move);
    }

    return movesSorted;
}

int minmaxAlphaBeta(chess::Board board, int depth, bool isMaximizingPlayer,int alpha, int beta,chess::Move killers1[],chess::Move killers2[]) {

    //Vérification dans la table de hashage
    int hashBoard = board.hash();
    auto calculated_board = hashTable.find(hashBoard);
    ValueMap value_stored;

    int killerDepth = board.plies_;

    if (calculated_board != hashTable.end()) {
        value_stored = calculated_board->second;
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


    if (depth <= 0) {

        //Peut etre essayer de store quand on arrive ici la première fois

        return evaluate(board);;  //on s'arrete ici
    }

    //Initialisation des valeurs a stocker
    ValueMap value_to_store;
    value_to_store.depth = depth;
    int type_exact;
    std::string uci_to_store;

    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.empty()) {
        return evaluate(board);
    }

    if (calculated_board != hashTable.end()) {
        moves = sortMove(moves,uci::uciToMove(board,value_stored.uci_move) ,killers1[killerDepth],killers2[killerDepth]);
    }



    if (isMaximizingPlayer) {

        type_exact = 1;

        int maxEval = std::numeric_limits<int>::min();

        int i =0;
        for (const auto &move : moves) {
            i++;
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup

            int eval;
            if (i >= nb_coup_max_depth) {
                eval = minmaxAlphaBeta(newBoard, depth - late_reduction, false, alpha, beta,killers1,killers2);
                if (beta <= eval) {
                    eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta,killers1,killers2);
                    test++;
                }
            }
            else {
                eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta,killers1,killers2);
            }

            //max
            if (eval > maxEval) {
                maxEval = eval;
                uci_to_store = uci::moveToUci(move);
            }


            //Alpha beta pruning
            if (beta <= maxEval) {
                type_exact = -1;

                killers1[killerDepth] = killers2[killerDepth];
                killers2[killerDepth] = move;

                break;
            }
            if (alpha < maxEval) {
                type_exact = 0;
                alpha = maxEval;
            }

            killers1[killerDepth] = killers2[killerDepth];
            killers2[killerDepth] = move;

            if (current_depth == depth) {
                std::cout << uci::moveToUci(move) << " : " << eval << std::endl;
            }

        }

        value_to_store.exact = type_exact;
        value_to_store.value = maxEval;
        value_to_store.uci_move = uci_to_store;

        hashTable[hashBoard] = value_to_store;


        return maxEval - current_depth + depth;
    }

    else {

        type_exact = -1;

        int minEval = std::numeric_limits<int>::max();

        int i =0;
        for (const auto &move : moves) {
            i++;
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup

            int eval;
            if (i >= nb_coup_max_depth) {
                eval = minmaxAlphaBeta(newBoard, depth - late_reduction, true, alpha, beta,killers1,killers2);
                if (eval <= alpha) {
                    eval = minmaxAlphaBeta(newBoard, depth - 1, true, alpha, beta,killers1,killers2);
                    test++;
                }
            }
            else {
                eval = minmaxAlphaBeta(newBoard, depth - 1, true, alpha, beta,killers1,killers2);
            }

            //min
            if (eval < minEval) {
                minEval = eval;
                uci_to_store = uci::moveToUci(move);
            }

            //Alpha beta pruning
            if (minEval <= alpha) {
                type_exact = 1;

                killers1[killerDepth] = killers2[killerDepth];
                killers2[killerDepth] = move;

                break;
            }
            if (minEval < beta) {
                type_exact = 0;
                beta = minEval;
            }

            killers1[killerDepth] = killers2[killerDepth];
            killers2[killerDepth] = move;

            if (current_depth == depth) {
                std::cout << uci::moveToUci(move) << " : " << eval << std::endl;
            }

        }

        value_to_store.exact = type_exact;
        value_to_store.value = minEval;
        value_to_store.uci_move = uci_to_store;
        hashTable[hashBoard] = value_to_store;

        return minEval + current_depth - depth;
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

    //killer moves
    chess::Move killers1[1000];
    chess::Move killers2[1000];

    for (int depth = 1 ; depth <= std::numeric_limits<int>::max(); depth++ ) {
    //for (int depth = 1 ; depth <=7 ; depth++ ) {

        current_depth = depth;

        //Arret par chrono
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (duration.count() > time) {
            break;
        }

        std::cout << "Current_depth: " << depth << std::endl;


        //Ancienne façon de faire, je garde por pouvoir vérifier si une erreur vient de la nouvelle version ou non
         // int best_eval = std::numeric_limits<int>::min(); // -inf au départ
         //
         // for (const auto &move : moves) {
         //
         //     chess::Board newBoard = board;
         //     newBoard.makeMove(move);
         //
         //     int eval = minmaxAlphaBeta(newBoard,depth,false,std::numeric_limits<int>::min(),std::numeric_limits<int>::max(),killers1,killers2);
         //
         //     std::cout << eval<< " : " << uci::moveToUci(move) <<std::endl;
         //
         //     if (eval > best_eval) {
         //         best_eval = eval;
         //         bestMove = move; // Mise à jour du meilleur coup trouvé
         //     }
         // }

        minmaxAlphaBeta(board,depth,true,std::numeric_limits<int>::min(),std::numeric_limits<int>::max(),killers1,killers2);
        bestMove = uci::uciToMove(board,hashTable[board.hash()].uci_move);
    }

    std::cout << hashTable[board.hash()].value<< std::endl;

    return bestMove;
}

void play(){
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq – 0 1");
    std::string input;
    std::string command = "";
    int res = 0;
    std::cout << "Entrez la couleur que vous souhaitez (w ou b) : ";
      // Pour ignorer le caractère de nouvelle ligne laissé dans le buffer par le précédent cin
    std::getline(std::cin, input);
    if (input == "b" || input == "B") {
        std::cout << "Vous avez choisi noir (black)." << std::endl;
        std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();
        while(result.first == chess::GameResultReason::NONE){
            chess::Move move = best_move_iterative_deepening(board,5000);
            board.makeMove(move);
            command = "python3 fen_to_board.py \"" + board.getFen() + "\"";
            res = std::system(command.c_str());

            if (res != 0) {
                std::cerr << "Erreur lors de l'exécution du script Python." << std::endl;
            }
            result = board.isGameOver();
            if(result.first == chess::GameResultReason::NONE){
                break;
            }
            //afficher le move 
            std::cin.ignore();
            std::cout << "Entrez votre coup en UCI : ";
            // Pour ignorer le caractère de nouvelle ligne laissé dans le buffer par le précédent cin
            std::getline(std::cin, input);
            move = uci::uciToMove(board,input);
            board.makeMove(move);
            command = "python3 fen_to_board.py \"" + board.getFen() + "\"";
            //afficher tableau
            res = std::system(command.c_str());

            if (res != 0) {
                std::cerr << "Erreur lors de l'exécution du script Python." << std::endl;
            }
            result = board.isGameOver();

        }
    }
    else if (input == "w" || input == "W") {
        std::cout << "Vous avez choisi blanc (white)." << std::endl;
        std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();
        while(result.first == chess::GameResultReason::NONE){
            
            std::cout << "Entrez votre coup en UCI : ";
            // Pour ignorer le caractère de nouvelle ligne laissé dans le buffer par le précédent cin
            std::getline(std::cin, input);
            chess::Move move = uci::uciToMove(board,input);
            board.makeMove(move);
            command = "python3 fen_to_board.py \"" + board.getFen() + "\" &";
            res= std::system(command.c_str());

            if (res != 0) {
                std::cerr << "Erreur lors de l'exécution du script Python." << std::endl;
            }
            //afficher tableau
            result = board.isGameOver();
            if(result.first == chess::GameResultReason::NONE){
                break;
            }
            move = best_move_iterative_deepening(board,5000);
            board.makeMove(move);
            command = "python3 fen_to_board.py \"" + board.getFen() + "\" &";
            res = std::system(command.c_str());

            if (res != 0) {
                std::cerr << "Erreur lors de l'exécution du script Python." << std::endl;
            }
            result = board.isGameOver();
            //afficher le move 


        }
    }
    else {
        std::cout << "Entrée invalide. Veuillez entrer 'b' ou 'w'." << std::endl;
    }
}

int main () {
    //Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq – 0 1");
    Board board = Board("1rb5/8/p5p1/1p1N1kNp/5P1P/4KP2/Pbn3P1/6R1 w - - 0 1");
    //K3Q2R/PP4PP/3q1N2/1R3P2/3nPp2/4p3/Bpp3pp/1kr2b1r w - - 0 1    souci, ça crash...


    for(int i=0; i<1; i++){
        chess::Move move = best_move_iterative_deepening(board,15000);
        std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;

        board.makeMove(move);
    }

    std::cout << "nb pos : "<< nb_pos << " " << test << std::endl;
    std::cout << "FEN  : " << board.getFen() << std::endl;

    return 0;
}
