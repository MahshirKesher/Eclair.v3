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
}

Status EditorCore::handleMovement(Movement input)
{ 
    int row = cursor_.row(), col = cursor_.col(), prefCol = cursor_.prefCol();
    int rowOffset = view_.rowOffset(), rows = view_.rows();
    
    switch(input)
    {
        case UP:
            if(row > 0) cursor_.setRow(--row);
            if(row <= rowOffset + 2)
            {
                view_.setStart({0, 0});
                render_.updateScreen();
                view_.setRowOffset(0);
            }
            break;
        case DOWN:
            if(row < file_.filerows()) cursor_.setRow(++row);
            if(row >= rowOffset + rows - 3)
            {
                view_.setStart(render_.rowStart(0));
                view_.setRowOffset(++rowOffset);
                render_.updateScreen();
                cursor_.setCol(std::min(prefCol, render_.rowSize(row)));
            }
            break;
        case LEFT:
            if(col > 0) cursor_.setCol(--col);
            else
            {
                cursor_.setRow(--row);
                cursor_.setCol(render_.rowSize(row));
            }
            cursor_.setPrefCol(cursor_.col());
            break;
        case RIGHT:
            if(col < render_.rowSize(row)) cursor_.setCol(++col);
            else
            {
                row++;
                cursor_.setCol(0);
                cursor_.setPrefCol(cursor_.col());
            }
            break;
        case PAGE_UP:
            
            break;
        case PAGE_DOWN:
            
            break;
        case HOME:
            
            break;
        case END:
            
            break;
    }
    render_.statusBar();
    render_.cursor();
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
            return Success;
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
