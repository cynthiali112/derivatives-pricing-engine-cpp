CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -I /opt/homebrew/opt/boost/include
TARGET := main
SRC := main.cpp

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

rebuild: clean all
