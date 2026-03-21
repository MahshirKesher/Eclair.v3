#pragma once

#include <string>

class FileHandler
{
    public:
        FileHandler();
        
        std::string open(std::string filename);
        void setFilerows(int filerows);
        
        int filerows();
        std::string filename();
        
    private:
        int filerows_;
        std::string filename_;
};
