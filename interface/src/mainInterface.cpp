#include <iostream>
#include <SFML/Graphics.hpp>
#include "Game.h"
#include "../../IA.cpp"
#include "../../include/chess.hpp"
#include <string>
using namespace IA;
using namespace chess;

int main()
{
label:
    sf::RenderWindow window(sf::VideoMode(1100, 800), "The Chess", sf::Style::Titlebar | sf::Style::Close);
    Game chess(sf::Color(0xf3bc7aff), sf::Color(0xae722bff));
    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))

        {
            if (e.type == sf::Event::Closed)
                window.close();
            if (e.type == sf::Event::MouseButtonPressed)
            {


                //Récupération du move en uci
                chess::Move move = IA::best_move(board);
                std::string moveUci =  uci::moveToUci(move);


                std::cout << "Meilleur coup : " << uci::moveToUci(move) << std::endl;


                //Récupération des coordonnées depuis le uci
                int xSelect = 8 -(moveUci[1] - '0');
                int ySelect = moveUci[0] - 'a';

                int xTarget = 8 - (moveUci[3] - '0');
                int yTarget = moveUci[2] - 'a';

                std::cout << "Normalement " << xSelect << ySelect << "to" <<xTarget<< yTarget<< std::endl;

                //Déplacemengt de la pièce
                chess.SelectPiece(chess.cells, xSelect,ySelect);
                chess.moveSelected(chess.cells, xTarget, yTarget);

                //Mise a jour du plateau
                board.makeMove(move);


                // if (e.mouseButton.button == sf::Mouse::Left)
                //  {
                //
                //
                //
                //      if (e.mouseButton.x >= 0 && e.mouseButton.x <= 800 && e.mouseButton.y >= 0 && e.mouseButton.y <= 800)
                //      {
                //          int x = e.mouseButton.y / 100, y = e.mouseButton.x / 100;
                //          if (!chess.getSelected() && chess.isOver == false)
                //          {
                //              //chess.SelectPiece(chess.cells, 1,1);
                //              chess.SelectPiece(chess.cells, x, y);
                //          }
                //          else
                //          {
                //              //chess.moveSelected(chess.cells,1,2);
                //              chess.moveSelected(chess.cells, e.mouseButton.y / 100, e.mouseButton.x / 100);
                //          }
                //      }
                //      if (e.mouseButton.x >= 850 && e.mouseButton.x <= 1024 && e.mouseButton.y >= 5 && e.mouseButton.y <= 55)
                //      {
                //          goto label;
                //      }
                //  }
            }
        }
        window.draw(chess);
        window.display();
    }
    return 0;
}
