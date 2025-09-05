CXX      := g++
CXXFLAGS := -std=c++17 -Wall -I./SFML-2.6.1-macOS-clang-arm64/include
LDFLAGS  := -L./SFML-2.6.1-macOS-clang-arm64/lib \
            -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio \
            -Wl,-rpath,@executable_path/SFML-2.6.1-macOS-clang-arm64/lib

SRC  := main.cpp
OBJ  := $(SRC:.cpp=.o)
BIN  := Battleship

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN)

.PHONY: all clean run
