#pragma once

#include "terminal.hpp"
#include "cursor.hpp"
#include "input.hpp"
#include "text.hpp"
#include "render.hpp"
#include "viewport.hpp"
#include "file.hpp"

#include <string>

class EditorCore
{
    public:
        EditorCore(std::string filename);
    
        void run();
        
        Status processInput(int input);
        Status handleMovement(Movement input);
        
        Location findNextStart(Location initLoc);
        Location findPreviousStart(Location initLoc);
        
    private:
        Terminal terminal_;
        InputHandler input_;
        Viewport view_;
        Cursor cursor_;
        FileHandler file_;
        TextBuffer text_;
        Renderer render_;
        
        int cursorOffset;
        
        bool running;
};
