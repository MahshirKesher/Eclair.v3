CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23
INCLUDES = -I ./editor -I ./input -I ./output -I ./shared -I ./terminal

SRC = \
./main.cpp \
./editor/core.cpp \
./terminal/terminal.cpp \
./editor/cursor.cpp \
./input/input.cpp \
./output/viewport.cpp \
./output/render.cpp \
./editor/text.cpp \
./editor/file.cpp

TARGET = eclair

v3: $(SRC)
	$(CXX) $(SRC) $(CXXFLAGS) $(INCLUDES) -o $(TARGET)
