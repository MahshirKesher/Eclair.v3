#pragma once

#include "sharedTypes.hpp"
#include <fstream>
#include <string>

class FileHandler
{
    public:
        FileHandler();
        
        std::string open(std::string filename);
        
        void setFilerows(int filerows);
        
        void openForWrite();
        void writeChunk(const std::string& content);
        void close();
        
        int filerows();
        std::string filename();
        
    private:
        int filerows_;
        std::string filename_;
        
        std::fstream file_;
};
