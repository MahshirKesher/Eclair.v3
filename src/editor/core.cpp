#include <algorithm>
#include <string>

#include "core.hpp"
#include "sharedTypes.hpp"
#include "keys.hpp"

EditorCore::EditorCore(std::string filename)
: terminal_(),
  input_(terminal_),
  view_(terminal_.getWindowSize()),
  cursor_(),
  file_(),
  text_(file_.open(filename)),
  render_(terminal_, view_, text_, cursor_, file_) 
{
    running = true;
    render_.updateScreen();
    
    cursorOffset = 0;
    
    std::string test = "\x1b[" + std::to_string(view_.rows()) + ";1H\x1b[K";
    test += std::to_string(cursorOffset);
    terminal_.write(test);
}

void EditorCore::updateCursorOffset()
{
    int cursorRowStart = text_.locToGlobal(render_.rowStart(cursor_.row() - view_.rowOffset()));
    cursorOffset = cursorRowStart + cursor_.col();
}

Status EditorCore::handleMovement(Movement input)
{ 
    int row = cursor_.row(), col = cursor_.col(), prefCol = cursor_.prefCol();
    int rowOffset = view_.rowOffset(), rows = view_.rows();
    
    switch(input)
    {
        case UP:
            if(row > 0) 
            {
                cursor_.setRow(--row);
                cursor_.setCol(std::min(prefCol, render_.rowSize(row - rowOffset)));
            }
            if(row <= rowOffset + 3 && row >= 3)
            {
                view_.setStart(text_.findPreviousRowStart(view_.viewStart()));
                view_.setRowOffset(--rowOffset);
                render_.updateScreen();
            }
            break;
        case DOWN:
            if(row < file_.filerows()) 
            {
                cursor_.setRow(++row);
                cursor_.setCol(std::min(prefCol, render_.rowSize(row - rowOffset)));
            }
            if(row >= rowOffset + rows - 3 && row <= file_.filerows())
            {
                view_.setStart(render_.rowStart(0));
                view_.setRowOffset(++rowOffset);
                render_.updateScreen();
            }
            break;
        case LEFT:
            if(col > 0) cursor_.setCol(--col);
            else if(row > 0)
            {
                cursor_.setRow(--row);
                cursor_.setCol(render_.rowSize(row - rowOffset));
                if(row <= rowOffset + 3 && row >= 3)
                {
                    view_.setStart(text_.findPreviousRowStart(view_.viewStart()));
                    view_.setRowOffset(--rowOffset);
                    render_.updateScreen();
                }
            }
            cursor_.setPrefCol(cursor_.col());
            break;
        case RIGHT:
            if(col < render_.rowSize(row - rowOffset)) cursor_.setCol(++col);
            else if(row < file_.filerows())
            {
                cursor_.setRow(++row);
                cursor_.setCol(0);
                if(row >= rowOffset + rows - 3 && row <= file_.filerows())
                {
                    view_.setStart(render_.rowStart(0));
                    view_.setRowOffset(++rowOffset);
                    render_.updateScreen();
                }
            }
            cursor_.setPrefCol(cursor_.col());
            break;
        case PAGE_UP:
            for(int i = 0; i < rows - 2; i++) 
            {
                view_.setStart(text_.findPreviousRowStart(view_.viewStart()));
                if(view_.viewStart().sameAs({0, 0})) break;
            }
            view_.setRowOffset(std::max(0, rowOffset - rows + 2));
            render_.updateScreen();
            cursor_.setRow(std::max(0, row - rows + 2));
            cursor_.setCol(std::min(prefCol, render_.rowSize(row - rowOffset)));
            break;
        case PAGE_DOWN:
            view_.setStart(view_.viewEnd());
            view_.setRowOffset(std::min(file_.filerows() - 2, rowOffset + rows - 2));
            render_.updateScreen();
            cursor_.setRow(std::min(file_.filerows(), row + rows - 2));
            cursor_.setCol(std::min(prefCol, render_.rowSize(row - rowOffset)));
            break;
        case HOME:
            cursor_.setCol(0);
            cursor_.setPrefCol(0);
            break;
        case END:
            cursor_.setCol(render_.rowSize(row - rowOffset));
            cursor_.setPrefCol(cursor_.col());
            break;
    }
    render_.statusBar();
    updateCursorOffset();
    
    Location currentLoc = text_.globalToLoc(cursorOffset);
    Piece currentPiece = text_.piece(currentLoc.pieceIndex);
    const std::string& buffer = text_.giveBuffer(currentPiece);
    
    std::string test = "\x1b[" + std::to_string(view_.rows()) + ";1H\x1b[K";
    test += std::to_string(currentLoc.pieceIndex) + "." + std::to_string(currentLoc.inPieceOffset);
    test += " " + std::to_string(cursorOffset);
    test += " ";
    char currentChar = buffer.at(currentPiece.start + currentLoc.inPieceOffset);
    if (currentChar == '\n') test += "\\n";
    else test += currentChar;
    terminal_.write(test);
    render_.cursor();
    text_.setContInsert(false);
    
    return Success;
}

Status EditorCore::handleEdit(int input)
{
    text_.edit(input, text_.globalToLoc(cursorOffset));
    if(input == '\n')
    {
        cursor_.setRow(cursor_.row() + 1);
        cursor_.setCol(0);
    }
    else if(input != '\b') cursor_.setCol(cursor_.col() + 1);
    cursorOffset++;
    render_.updateScreen();
    return Success;
}

Status EditorCore::processInput(int input)
{
    switch(input)
    {
        case QUIT:
            running = false;
            return Success;
        case UP:
        case DOWN:
        case RIGHT:
        case LEFT:
        case PAGE_UP:
        case PAGE_DOWN:
        case HOME:
        case END:
            return handleMovement(static_cast<Movement>(input));
        default:
            return handleEdit(input);
    }
}

void EditorCore::run()
{
    int input = 0;
    Status status = Success;

    while(running)
    {
        status = terminal_.read(&input); 
        
        if(status == Success) status = processInput(input_.define(input));
        else if(status == SignalInterrupt)
        {
            if(terminal_.checkFlag(WinResize))  
            {
                view_.setWindowSize(terminal_.getWindowSize());
                render_.updateScreen();
                terminal_.clearFlag();
            }
        }
    }
} 
