# Compilateur et options
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -Isrc

# Fichiers sources et executable
SRC = main.cpp
EXEC = chess_ai

# Dépendances (ajout du header)
DEPS = piece_square.hpp

# Compilation
all: $(EXEC)

$(EXEC): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXEC)

# Nettoyage
clean:
	rm -f $(EXEC)
