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

void EditorCore::updateCursorOffset()
{
    int cursorRowStart = text_.locToGlobal(render_.rowStart(cursor_.row() - view_.rowOffset()));
    cursorOffset = cursorRowStart + cursor_.col();
}

Status EditorCore::handleMovement(Movement input)
{ 
    if(text_.pieceCount() == 0) return Success;

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
                view_.setStart(render_.rowStart(1));
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
   
    std::string test;
    test += std::to_string(currentLoc.pieceIndex) + "." + std::to_string(currentLoc.inPieceOffset);
    test += " " + std::to_string(cursorOffset);
    test += " ";
    char currentChar = buffer.at(currentPiece.start + currentLoc.inPieceOffset);
    if (currentChar == '\n') test += "\\n";
    else test += currentChar;
    test += " " + std::to_string(text_.totalSize());
    render_.promptMessage(test);

    render_.cursor();
    text_.setContInsert(false);  
    return Success;
}

Status EditorCore::handleEdit(int input)
{
    if(input == DELETE_KEY) 
    {
        if(cursorOffset >= text_.totalSize()) return NoDeletionExecuted;
        Status edit = text_.deletion(text_.globalToLoc(cursorOffset));
        if(edit == NewlineDeleted) file_.setFilerows(file_.filerows() - 1);
    }
    else if(input == BACKSPACE_KEY) 
    {
        if(cursorOffset == 0) return NoDeletionExecuted;
        Status edit = text_.deletion(text_.globalToLoc(--cursorOffset));
        if(edit == NewlineDeleted) 
        {
            file_.setFilerows(file_.filerows() - 1);
            cursor_.setRow(cursor_.row() - 1);
            cursor_.setCol(render_.rowSize(cursor_.row() - view_.rowOffset()));
        }
        else cursor_.setCol(cursor_.col() - 1);
    }
    else if(input == '\t')
    {
        int charsToTab = TAB_SIZE - (cursor_.col() % TAB_SIZE);
        for(int i = 0; i < charsToTab; i++)
        {
            text_.edit(' ', text_.globalToLoc(cursorOffset)); 
            cursorOffset++;
            cursor_.setCol(cursor_.col() + 1);
        }
    }
    else
    {
        text_.edit(input, text_.globalToLoc(cursorOffset));
        if(input == '\n') 
        {
            file_.setFilerows(file_.filerows() + 1);
            cursor_.setRow(cursor_.row() + 1);
            cursor_.setCol(0);
        }
        else cursor_.setCol(cursor_.col() + 1);
        cursor_.setPrefCol(cursor_.col());
        cursorOffset++;
    }
    render_.updateScreen();
    return Success;
}

Status EditorCore::setFilename()
{
    std::string newFilename = "";
    Status status = Success;
    char currentChar = '0';

    while(currentChar != '\x1b')
    {
        status = terminal_.read(&currentChar);
        render_.promptMessage("Save as: " + newFilename);
        render_.cursor();
        if(status != Success) continue;
        currentChar = input_.define(currentChar);
        if(currentChar == '\n')
        {
            if(newFilename.empty()) 
            {
                render_.promptMessage("Provide a filename.");
                continue;
            }
            else
            {
                file_.setFilename(newFilename);
                return Success;
            }
        }
        else if(currentChar == BACKSPACE_KEY) newFilename.pop_back();
        newFilename += currentChar;
    }
    return Success;
}

Status EditorCore::saveChanges()
{
    if(!file_.hasName()) setFilename();
    file_.openForWrite();
    int pieceCount = text_.pieceCount();
    for(int i = 0; i < pieceCount; i++)
    {
        Piece currentPiece = text_.piece(i);
        const std::string& buffer = text_.giveBuffer(currentPiece);
        std::string chunk = buffer.substr(currentPiece.start, currentPiece.length);
        file_.writeChunk(chunk);
    }
    file_.close();
    std::string message = "Saved successfully. [" + std::to_string(text_.totalSize()) + " bytes]";
    render_.promptMessage(message);
    return Success;
}

Status EditorCore::processInput(int input)
{
    switch(input)
    {
        case QUIT:
            running = false;
            return Success;
        case SAVE:
            return saveChanges();
        case UP:
        case DOWN:
        case RIGHT:
        case LEFT:
        case PAGE_UP:
        case PAGE_DOWN:
        case HOME:
        case END:
            return handleMovement(static_cast<Movement>(input));
        case DELETE_KEY:
        case BACKSPACE_KEY:
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
