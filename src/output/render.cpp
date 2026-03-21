#include "render.hpp"
#include "terminal.hpp"
#include "viewport.hpp"
#include "text.hpp"
#include "cursor.hpp"
#include "file.hpp"

#include <string>
#include <format>

Renderer::Renderer(Terminal& terminal, Viewport& view, TextBuffer& text, 
                   Cursor& cursor, FileHandler& file)
: terminal_(terminal),
  view_(view),
  text_(text),
  cursor_(cursor),
  file_(file)
{}

void Renderer::clearScreen()
{
    terminal_.write("\x1b[2J\x1b[H");
}

void Renderer::updateScreen()
{
    terminal_.write("\x1b[?25l");
    fillFrame();
    statusBar();
    std::string frame = "\x1b[" 
                      + std::to_string(cursor_.row() + 1) 
                      + ";" 
                      + std::to_string(cursor_.col() + 1) 
                      + "H";
    terminal_.write("\x1b[?25h");
}

void Renderer::blank()
{
    int screenRows = view_.rows();
    std::string frame;
    for(int i = 0; i <= screenRows / 3; i++)
    {
        frame += "\x1b[K";
        frame += "~\n\r";
    }
    std::string message = "Empty here. Start something with Eclair!\n\r";
    int padding = static_cast<int>((view_.cols() / 2) - (message.size() / 2));
    while(padding--) frame += " ";
    frame += message;
    
    for(int i = (screenRows / 3) + 1; i < screenRows; i++)
    {
        frame += "\x1b[K";
        frame += "~\n\r";
    }  
    terminal_.write(frame);
}

void Renderer::fillFrame()
{
    if(text_.original().empty() && text_.appended().empty())
    {
        blank();
        return;
    }

    Location start = view_.viewStart();

    displayBuffer_.clear();
    size_t pieceCount = text_.pieceCount();
    int rowCount = 0;
    int screenRows = view_.rows();
    Piece currentPiece;
    std::string currentRow;
    std::string frame;
    
    frame += "\x1b[H";
    frame += "\x1b[K";
    for(size_t pieceIndex = start.pieceIndex; pieceIndex < pieceCount; pieceIndex++)
    {
        currentPiece = text_.piece(pieceIndex);
        const std::string& buffer = text_.giveBuffer(currentPiece);
        for(int i = start.inPieceOffset; i < currentPiece.length; i++)
        {
            frame += buffer.at(currentPiece.start + i);
            currentRow += frame.back();
            if(frame.back() == '\n') 
            {
                displayBuffer_.push_back(currentRow);
                currentRow.clear();
                frame += '\r';
                frame += "\x1b[K";
                rowCount++;
            }
            if(rowCount == screenRows - 2) 
            {
                terminal_.write(frame);
                break;
            }
        }
    }
    while(rowCount < screenRows - 2)
    {
        frame += "\x1b[K";
        frame += "~\n\r";
        rowCount++;
    }
    terminal_.write(frame);
}

void Renderer::statusBar()
{
    size_t screenCols = static_cast<size_t>(view_.cols());
    std::string command = "\x1b[" + std::to_string(view_.rows() - 1) + ";1H";
    command += "\x1b[K";
    command += "\x1b[7m";
    std::string bar = std::format("{:.20} - {} lines", file_.filename(), file_.filerows());
    std::string rightBar = std::format("{}/{}", cursor_.row() + 1, file_.filerows());
    if(bar.size() >= screenCols - rightBar.size())
    {
        bar.resize(screenCols);
        terminal_.write(bar);
        return;
    }
    else
    {
        for(size_t i = bar.size(); i < screenCols - rightBar.size(); i++) bar += " ";
        bar += rightBar;
    }
    terminal_.write(command);
    terminal_.write(bar);
    terminal_.write("\x1b[m");
}

std::string Renderer::row(size_t index)
{
    if(index >= displayBuffer_.size()) return "";
    return displayBuffer_.at(index);
}

int Renderer::rowSize(size_t index)
{
    if(index >= displayBuffer_.size()) return 0;
    else return displayBuffer_.at(index).size() - 1;
}
