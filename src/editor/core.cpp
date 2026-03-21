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
    
    cursorLoc.pieceIndex = 0;
    cursorLoc.inPieceOffset = 0;
}

Location EditorCore::findNextStart()
{
    Location initStart = view_.viewStart();
    Location currentStart = initStart;
    int pieceCount = text_.pieceCount();
    
    for(int i = currentStart.pieceIndex; i < pieceCount; i++)
    {
        Piece currentPiece = text_.piece(i);
        const std::string& buffer = text_.giveBuffer(currentPiece);
        
        for(int j = currentStart.inPieceOffset; j < currentPiece.length; j++)
        {
            if(buffer.at(j) == '\n')
            {
                bool endOfFile = (i == pieceCount - 1 && j == currentPiece.length - 1);
                if(endOfFile) return initStart;
                else if(j == currentPiece.length - 1) return {i + 1, 0}; 
                else return {i, j + 1};
            }
        }
        currentStart.inPieceOffset = 0;
    }
    return initStart;
}

bool EditorCore::enoughSpace(Location currentStart, int steps)
{
    int freespace = 0;
    for(int i = 0; i < currentStart.pieceIndex; i++)
    {
        freespace += text_.piece(i).length;
        if(freespace >= steps) return true;
    }
    freespace += currentStart.inPieceOffset;
    if(freespace >= steps) return true;
    else return false;
}

Location EditorCore::stepBack(Location currentStart, int steps)
{
    if(!enoughSpace(currentStart, steps)) return currentStart;

    if(currentStart.inPieceOffset >= steps)
        return {currentStart.pieceIndex, currentStart.inPieceOffset - steps};
    steps -= (currentStart.inPieceOffset + 1);
    if(currentStart.pieceIndex > 0) currentStart.pieceIndex--;
    currentStart.inPieceOffset = text_.piece(currentStart.pieceIndex).length - 1;
    while(steps > 0)
    {
        if(currentStart.inPieceOffset >= steps)
            return {currentStart.pieceIndex, currentStart.inPieceOffset - steps};
       
        steps -= (currentStart.inPieceOffset + 1);
        if(currentStart.pieceIndex > 0) currentStart.pieceIndex--;
        currentStart.inPieceOffset = text_.piece(currentStart.pieceIndex).length - 1;
    }
    return currentStart;
}

Location EditorCore::findPreviousStart()
{
    Location initStart = view_.viewStart();
    if(initStart.inPieceOffset < 2 && initStart.pieceIndex == 0) return {0, 0};
    
    Location currentStart = stepBack(initStart, 2);
    if(initStart.sameAs(currentStart)) return {0, 0};
    
    for(int i = currentStart.pieceIndex; i >= 0; i--)
    {
        Piece currentPiece = text_.piece(i);
        const std::string& buffer = text_.giveBuffer(currentPiece);
        
        for(int j = currentStart.inPieceOffset; j >= 0; j--)
        {
            if(buffer.at(j) == '\n')
            {
                if(j == currentPiece.length - 1) return {i + 1, 0}; 
                else return {i, j + 1};
            }
        }
        if(i > 0) currentStart.inPieceOffset = (text_.piece(i - 1).length - 1);
    }
    return {0, 0};
}

Location EditorCore::logicMoveCursor(int row, int col)
{
    int direction = 0;
    Location initLoc = cursorLoc;
    int currentRow = cursor_.row(); int currentCol = cursor_.col();

    if(row < currentRow || (row == currentRow && col < currentCol)) direction = -1;
    else if(row > currentRow || (row == currentRow && col > currentCol)) direction = 1;
    else return initLoc;

    
}

Status EditorCore::handleMovement(Movement input)
{ 
    int row = cursor_.row();
    int col = cursor_.col();
    int prefCol = cursor_.prefCol();
    int rowOffset = view_.rowOffset();
    switch(input)
    {
        case UP:
            if(row > 0) row--;
            if(row <= rowOffset + 3)
            {
                view_.setStart(findPreviousStart());
                rowOffset--;
                render_.updateScreen();
            }
            col = std::min(render_.rowSize(row - rowOffset), prefCol);
            break;
        case DOWN:
            if(row < file_.filerows()) row++;
            if(row >= (rowOffset + view_.rows()) - 3)
            {
                view_.setStart(findNextStart());
                render_.updateScreen();
                rowOffset++;
            }
            col = std::min(render_.rowSize(row - rowOffset), prefCol);
            break;
        case LEFT:
            if(col > 0) col--;
            else if(row > 0)
            {
                row--;
                col = render_.rowSize(row - rowOffset);
            }
            if(row <= rowOffset + 3)
            {
                view_.setStart(findPreviousStart());
                rowOffset--;
                render_.updateScreen();
            }
            prefCol = col;
            break;
        case RIGHT:
            if(col < render_.rowSize(row - rowOffset)) col++;
            else if(row < file_.filerows())
            {
                row++;
                col = 0;
            }
            if(row >= (rowOffset + view_.rows()) - 3)
            {
                view_.setStart(findNextStart());
                render_.updateScreen();
                rowOffset++;
            }
            prefCol = col;
            break;
        case PAGE_UP:
            for(int i = 0; i < view_.rows(); i++)
            {
                view_.setStart(findPreviousStart());
                if(view_.viewStart().sameAs({0, 0})) break;
            }
            rowOffset = std::max(0, rowOffset - view_.rows());
            row = std::max(0, row - view_.rows());
            render_.updateScreen();
            col = std::min(render_.rowSize(row - rowOffset), prefCol);
            break;
        case PAGE_DOWN:
            for(int i = 0; i < view_.rows(); i++)
            {
                Location initStart = view_.viewStart();
                view_.setStart(findNextStart());
                if(view_.viewStart().sameAs(initStart)) break;
            }
            rowOffset = std::min(file_.filerows(), view_.rowOffset() + view_.rows() - 2);
            row = std::min(file_.filerows(), row + view_.rows() - 2);
            render_.updateScreen();
            col = std::min(render_.rowSize(row - rowOffset), prefCol);
            break;
        case HOME:
            col = 0;
            prefCol = col;
            break;
        case END:
            col = render_.rowSize(row - rowOffset);
            prefCol = col;
            break;
    }
    cursor_.setRow(row);
    cursor_.setCol(col);
    cursor_.setPrefCol(prefCol);
    view_.setRowOffset(rowOffset);
    render_.statusBar();
    std::string buffer = "\x1b[" 
                       + std::to_string((cursor_.row() - view_.rowOffset()) + 1) 
                       + ";" 
                       + std::to_string(cursor_.col() + 1) 
                       + "H";
    terminal_.write(buffer);
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
