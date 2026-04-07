#pragma once

#define CTRL_(x) ((x) & 0x1F)

enum Editing
{
    BACKSPACE_KEY = 127,
    DELETE_KEY = 400,
};

enum Movement
{
    UP = 650,
    DOWN,
    RIGHT,
    LEFT,
    
    PAGE_UP,
    PAGE_DOWN,
    
    HOME,
    END
};

enum Command
{
    QUIT = 300,
    SAVE
};
