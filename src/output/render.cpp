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
    cursor();
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
    frame += "~";
    std::string message = "Empty here. Start something with Eclair!\n\r";
    int padding = static_cast<int>((view_.cols() / 2) - (message.size() / 2));
    if(padding > 0) frame.append(padding, ' ');
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
    std::string row; std::string frame;
    int rowCount = view_.rowOffset();
    int pieceCount = text_.pieceCount();
    int frameEnd = view_.rowOffset() + view_.rows() - 2;
    frame += "\x1b[H\x1b[K";
    screenRows.clear();
    rowStarts.clear();
    for(int i = start.pieceIndex; i < pieceCount; i++)
    {
        Piece currentPiece = text_.piece(i);
        const std::string& buffer = text_.giveBuffer(currentPiece);
        for(int j = start.inPieceOffset; j < currentPiece.length; j++)
        {
            frame += buffer.at(currentPiece.start + j);
            row += frame.back();
            if(frame.back() == '\n') 
            {
                rowCount++;
                screenRows.push_back(row);
                row.clear();
                frame += '\r';
                frame += "\x1b[K";
            
                if(j < currentPiece.length - 1) rowStarts.push_back({i, j + 1});
                else if(i < pieceCount - 1) rowStarts.push_back({i + 1, 0});
                else rowStarts.push_back({i, j});
            }
            if(rowCount == frameEnd) 
            {
                if(j < currentPiece.length - 1) view_.setEnd({i, j + 1});
                else if(i < pieceCount - 1) view_.setEnd({i + 1, 0});
                else view_.setEnd({i, j});
                terminal_.write(frame);
                return;
            }
        }
        start.inPieceOffset = 0;
    }
    screenRows.push_back(row);
    while(rowCount++ < frameEnd) frame += "\x1b[K~\n\r";
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
        size_t space = screenCols - bar.size();
        bar.append(space, ' ');
        command += bar;
        command += "\x1b[m";
        terminal_.write(command);
        return;
    }
    else
    {
        size_t space = screenCols - rightBar.size() - bar.size();
        bar.append(space, ' ');
        bar += rightBar;
    }
    command += bar;
    command += "\x1b[m";
    terminal_.write(command);
}

void Renderer::cursor()
{
    std::string buffer = 
        "\x1b[" 
        + std::to_string((cursor_.row() - view_.rowOffset()) + 1) 
        + ";" 
        + std::to_string(cursor_.col() + 1) 
        + "H";
    terminal_.write(buffer);
}

std::string Renderer::row(size_t index)
{
    if(index >= screenRows.size()) return "";
    return screenRows.at(index);
}

int Renderer::rowSize(size_t index)
{
    if(index >= screenRows.size()) return 0;
    else return screenRows.at(index).size() - 1;
}

Location Renderer::rowStart(size_t index)
{
    if(index >= rowStarts.size()) return rowStarts.at(rowStarts.size() - 1);
    else return rowStarts.at(index);
}
