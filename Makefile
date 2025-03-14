# Compilateur et options
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -Isrc

# Fichiers sources et executable
SRC = main.cpp
EXEC = chess_ai

# Compilation
all: $(EXEC)

$(EXEC): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Nettoyage
clean:
	rm -f $(EXEC)
