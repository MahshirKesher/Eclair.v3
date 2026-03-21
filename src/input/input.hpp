#pragma once

#include "keys.hpp"

class Terminal;

class InputHandler
{
    public:
        InputHandler(Terminal& terminal);
    
        int define(int input);
        int defineSequence();
        
    private:
        Terminal& terminal_;
};
