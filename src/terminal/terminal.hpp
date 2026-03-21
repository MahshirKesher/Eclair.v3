#pragma once

#include "sharedTypes.hpp"
#include <cstdint>
#include <signal.h>
#include <string_view>
#include <termios.h>

class Terminal
{
    public:
        Terminal();
        ~Terminal();
    
        Status enableRawMode();
        Status disableRawMode();
        
        Status read(void* storage);
        Status write(std::string_view input);
        
        bool checkFlag(Interrupt flag);
        void clearFlag();
        
        void setupSignalHandler();
        static void handleInterrupt(int signal);
        
        WinSize getWindowSize();
        
    private:
        struct termios original_;
        volatile static sig_atomic_t signalFlag;
};
