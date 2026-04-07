#pragma once

#include "terminal.hpp"
#include "cursor.hpp"
#include "input.hpp"
#include "text.hpp"
#include "render.hpp"
#include "viewport.hpp"
#include "file.hpp"

#include <string>

#define TAB_SIZE 2

class EditorCore
{
    public:
        EditorCore(std::string filename);
    
        void run();
        
        Status processInput(int input);
        Status handleMovement(Movement input);
        Status handleEdit(int input);
        
        Status saveChanges();
        
        void updateCursorOffset();
        
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
