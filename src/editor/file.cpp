#include "file.hpp"
#include <fstream>
#include <vector>

FileHandler::FileHandler()
{
    filename_ = "blank";
    filerows_ = 0;
}

std::string FileHandler::open(std::string filename)
{
    if(filename == "") return " ";
    filename_ = filename;

    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0, file.end);
    int filesize = file.tellg();
    file.seekg(0, file.beg);

    std::string text(filesize, '*');
    file.read(text.data(), filesize);

    std::vector<int> rowStarts; 

    for(int i = 0; i < filesize; i++)
    {
        if(text.at(i) == '\n') 
        {
            filerows_++;
            rowStarts.push_back(i + 1);
        }
    }
    return text;
}

void FileHandler::setFilerows(int filerows)
{
    filerows_ = filerows;
}

int FileHandler::filerows() { return filerows_; }
std::string FileHandler::filename() { return filename_; }
