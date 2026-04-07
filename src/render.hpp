#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "sharedTypes.hpp"

class Terminal;
class Viewport;
class TextBuffer;
class Cursor;
class FileHandler;

class Renderer
{
    public:
        Renderer(Terminal&, Viewport&, TextBuffer&, Cursor&, FileHandler&);
    
        void updateScreen();
        void clearScreen();
        
        void fillFrame();
        void statusBar();
        void blank();
        void cursor();
        
        void promptMessage(const std::string& message);
        
        std::string row(size_t index);
        int rowSize(size_t index);
        Location rowStart(size_t index);
        
    private:
        Terminal& terminal_;
        Viewport& view_;
        TextBuffer& text_;
        Cursor& cursor_;
        FileHandler& file_;
        
        std::vector<Location> rowStarts;
        std::vector<std::string> screenRows;
};
