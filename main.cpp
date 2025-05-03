
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

std::unordered_map<unsigned long, ValueMap> hashTable;

int evaluate(const chess::Board &board) {


    std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();

    if (result.first == chess::GameResultReason::CHECKMATE) {
        if (board.sideToMove() == Color::WHITE) {
            return -10000;
        }
        return 10000;
    }
    if (result.first != chess::GameResultReason::NONE) {
        return 0;
    }
    return eval(board);
    
}

chess::Movelist sortMove(chess::Movelist& movelist,chess::Move bestMove,chess::Move killerMove1,chess::Move killerMove2) {
    chess::Movelist tmp;// Liste temporaire pour stocker les autres coups.
    chess::Movelist movesSorted; // Liste finale triée à retourner.

    chess::Move b = NULL;
    chess::Move k1 = NULL;
    chess::Move k2 = NULL;

    for (const auto &move : movelist) {
        if (move == bestMove){b=move;}
        else if (move == killerMove1){k1=move;}
        else if (move == killerMove2){k2=move;}//On récupère les killer et le best
        else {
            tmp.add(move); // Ce n’est ni un  bestMove, ni un killer → on le garde pour plus tard.
        }
    }

    if (b != NULL){movesSorted.add(b);}
    if (k1 != NULL){movesSorted.add(k1);}
    if (k2 != NULL){movesSorted.add(k2);}//on les ajoutes en premiers


    for (const auto &move : tmp) {
        movesSorted.add(move);//on ajoute le reste
    }

    return movesSorted;
}

int minmaxAlphaBeta(chess::Board board, int depth, bool isMaximizingPlayer,int alpha, int beta,chess::Move killers1[],chess::Move killers2[],Color color) {
    /*board: position actuelle de l’échiquier

depth: profondeur restante à explorer

isMaximizingPlayer: booléen indiquant si le joueur courant maximise (true) ou minimise (false)

alpha, beta: bornes pour l’élagage alpha-bêta

killers1, killers2: tableaux contenant les coups killers pour chaque profondeur

color: couleur du joueur IA (le joueur qu'on cherche à évaluer de manière cohérente, ex: IA joue blanc, donc on veut un score positif si les blancs sont mieux)*/

    //Vérification dans la table de hashage
    unsigned long hashBoard = board.hash();
    auto calculated_board = hashTable.find(hashBoard);//on cherche dna sla table et sinon ça renvoie end
    ValueMap value_stored;

    int killerDepth = board.plies_;//nombre de coups joués depuis le début

    if (calculated_board != hashTable.end()){//si pas trouvé
        value_stored = calculated_board->second;//on extrait value stored qui contient exact,depth et value
        if (value_stored.depth > depth) {//on update si la depth est plus profonde
            //exact peut etre 0 = val exacte, ou -1 borne inférieure ou 1 borne superieure
            if (value_stored.exact == 0) {
                return value_stored.value;
            }
            if (value_stored.exact == -1 && value_stored.value >= beta) {//coupe beta
                return value_stored.value;
            }
            if (value_stored.exact == 1 && value_stored.value <= alpha) {//coupe alpha
                return value_stored.value;
            }
            if (value_stored.exact == 1) {//on continue en updatant
                beta = std::min(value_stored.value, beta);
            }
            else if (value_stored.exact == -1) {
                alpha = std::max(value_stored.value, alpha);
            }

        }
    }


    if (depth <= 0) {

        return  color == Color::WHITE? evaluate(board): -evaluate(board);//on s'arrete ici qunad on a atteint la depth max
    }

    //Initialisation des valeurs a stocker
    ValueMap value_to_store;
    value_to_store.depth = depth;
    int type_exact;
    std::string uci_to_store;

    Movelist moves;
    movegen::legalmoves(moves, board);
    if (moves.empty()) {//si plus de coup disponibles on évalue
        return evaluate(board);
    }

    if (calculated_board != hashTable.end()) {//si on le connait on met en premier les meilleurs et les 2 killers
        moves = sortMove(moves,uci::uciToMove(board,value_stored.uci_move) ,killers1[killerDepth],killers2[killerDepth]);
    }



    if (isMaximizingPlayer) {// si c'est un joueur maximisant

        type_exact = 1;//bornes sup (on est par défaut pessimiste

        int maxEval = std::numeric_limits<int>::min();

        int i =0;
        for (const auto &move : moves) {//pour chaque move possible
            i++;
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup

            int eval;
            if (i >= nb_coup_max_depth) {
                eval = minmaxAlphaBeta(newBoard, depth - late_reduction, false, alpha, beta,killers1,killers2,color);
                value_to_store.depth = depth/2;
                if (beta <= eval) {
                    eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta,killers1,killers2,color);
                    value_to_store.depth = depth;
                }
            }
            else {
                eval = minmaxAlphaBeta(newBoard, depth - 1, false, alpha, beta,killers1,killers2,color);
            }

            //max
            if (eval > maxEval) {//mise à jour du max
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


            if (current_depth == depth) {
                std::cout << uci::moveToUci(move) << " : " << eval << std::endl;
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

        int i =0;
        for (const auto &move : moves) {
            i++;
            chess::Board newBoard = board;
            newBoard.makeMove(move);  // Joue le coup

            int eval;
            if (i >= nb_coup_max_depth) {//late move reduction si le i est plus grand que les premiers coups qu'on juge
                eval = minmaxAlphaBeta(newBoard, depth - late_reduction, true, alpha, beta,killers1,killers2,color);
                value_to_store.depth = depth/2;
                if (eval <= alpha) {//si le coup est mieux que prévu on l'évalue complètement
                    eval = minmaxAlphaBeta(newBoard, depth - 1, true, alpha, beta,killers1,killers2,color);
                    value_to_store.depth = depth;
                }
            }
            else {//si le i est faible on late move reduct pas
                eval = minmaxAlphaBeta(newBoard, depth - 1, true, alpha, beta,killers1,killers2,color);
            }

            //min
            if (eval < minEval) {//update du min si coup bien
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



            if (current_depth == depth) {
                std::cout << uci::moveToUci(move) << " : " << eval << std::endl;
            }

        }

        value_to_store.exact = type_exact;
        value_to_store.value = minEval;
        value_to_store.uci_move = uci_to_store;
        hashTable[hashBoard] = value_to_store;

return minEval;
    }
}

    chess::Move best_move_iterative_deepening(chess::Board board,int time,Color color) {
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
        chess::Move killers1[1000];// il faut les initialiser pour éviter les erreurs
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

            minmaxAlphaBeta(board,depth,true,std::numeric_limits<int>::min(),std::numeric_limits<int>::max(),killers1,killers2,color);
            bestMove = uci::uciToMove(board, hashTable[board.hash()].uci_move);
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
        Color color = Color::BLACK;
        std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();
        while(result.first == chess::GameResultReason::NONE){
            chess::Move move = best_move_iterative_deepening(board,15000,color);
            board.makeMove(move);
            command = "python3 fen_to_board.cpp \"" + board.getFen() + "\"";
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
            command = "python3 fen_to_board.cpp \"" + board.getFen() + "\"";
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
        Color color = Color::WHITE;
        std::pair<chess::GameResultReason, chess::GameResult> result = board.isGameOver();
        while(result.first == chess::GameResultReason::NONE){
            
            std::cout << "Entrez votre coup en UCI : ";
            // Pour ignorer le caractère de nouvelle ligne laissé dans le buffer par le précédent cin
            std::getline(std::cin, input);
            chess::Move move = uci::uciToMove(board,input);
            board.makeMove(move);
            command = "python3 fen_to_board.cpp \"" + board.getFen() + "\" &";
            res= std::system(command.c_str());

            if (res != 0) {
                std::cerr << "Erreur lors de l'exécution du script Python." << std::endl;
            }
            //afficher tableau
            result = board.isGameOver();
            if(result.first == chess::GameResultReason::NONE){
                break;
            }
            move = best_move_iterative_deepening(board,15000,color);
            board.makeMove(move);
            command = "python3 fen_to_board.cpp \"" + board.getFen() + "\" &";
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
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq – 0 1");

    //2b2b1r/ppN1kppp/5q2/2p5/3n4/1PPQ2P1/PK2P2P/R7 w - - 0 1
    //1rbqr3/p4pkp/2p2nP1/2p1p3/2P1P3/3P3P/P5PN/RN1Q1RK1 b - - 0 1
    //1rb5/8/p5p1/1p1N1kNp/5P1P/4KP2/Pbn3P1/6R1 w - - 0 1
    //r4rk1/1b4b1/ppn1N1p1/2pB3p/P3P2q/3P4/1PP3PP/R1B2RK1 w - - 0 1
    //K3Q2R/PP4PP/3q1N2/1R3P2/3nPp2/4p3/Bpp3pp/1kr2b1r w - - 0 1    souci, ça crash...

    Color color = Color::WHITE;
    for (int i = 0; i < 100; i++) {
        // L'ordinateur joue
        color = (i % 2 == 1) ? Color::BLACK : Color::WHITE;
        chess::Move move = best_move_iterative_deepening(board, 15000, board.sideToMove());
        std::cout << "Meilleur coup (ordinateur) : " << uci::moveToUci(move) << std::endl;

        board.makeMove(move);

        // Affichage facultatif de l’échiquier (si tu as une fonction pour)
        // displayBoard(board);

        // Vérifie que la partie n’est pas finie
        if (board.isGameOver().first != chess::GameResultReason::NONE) {
            std::cout << "Fin de la partie." << std::endl;
            break;
        }

        // L'utilisateur joue
        std::string input;
        chess::Move userMove;

        while (true) {
            std::cout << "Entrez votre coup (format UCI, ex: e2e4) : ";
            std::cin >> input;

            try {
                userMove = uci::uciToMove(board, input);
                Movelist moves;
                movegen::legalmoves(moves, board);
                if (moves.find(userMove)!=-1) {
                    board.makeMove(userMove);
                    break;
                } else {
                    std::cout << "Coup illégal. Réessayez." << std::endl;
                }

            } catch (const std::exception &e) {
                std::cout << "Format invalide ou coup incorrect. Réessayez." << std::endl;
            }
        }

        if (board.isGameOver().first != chess::GameResultReason::NONE) {
            std::cout << "Fin de la partie." << std::endl;
            break;
        }
    }

    std::cout << "nb pos : "<< nb_pos << " " << test << std::endl;
    std::cout << "FEN  : " << board.getFen() << std::endl;


    return 0;
}
