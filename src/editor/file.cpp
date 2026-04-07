#include "file.hpp"
#include <iostream>
#include <vector>

FileHandler::FileHandler()
{
    filename_ = "blank";
    filerows_ = 0;
}

std::string FileHandler::open(std::string filename)
{
    if(filename == "") return "";
    filename_ = filename;

    file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    file_.seekg(0, file_.end);
    int filesize = file_.tellg();
    file_.seekg(0, file_.beg);

    std::string text(filesize, '*');
    file_.read(text.data(), filesize);
    file_.close();

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

void FileHandler::openForWrite()
{
   file_.open(filename_, std::ios::out | std::ios::trunc);
}

void FileHandler::writeChunk(const std::string& content)
{
    file_ << content;
}

void FileHandler::close()
{
    file_.close();
}

int FileHandler::filerows() { return filerows_; }
std::string FileHandler::filename() { return filename_; }
