#include "text.hpp"

TextBuffer::TextBuffer(std::string init)
{
    if(init != "") 
    {
        original_ = init;
        Piece initPiece = {0, static_cast<int>(original_.size()), ORIGINAL};
        pieces.push_back(initPiece);
        totalSize_ = pieces.at(0).length;
    }
    else totalSize_ = 0;
}

Location TextBuffer::globalToLoc(int offset)
{
    int pieceCount_ = pieceCount();
    for(int i = 0; i < pieceCount_; i++)
    {
        Piece currentPiece = piece(i);
        if(offset < currentPiece.length) return {i, offset};
        else offset -= currentPiece.length;
    } //Leaving the loop without returning means we hit the end of file.
    Location eof = {pieceCount_ - 1, piece(pieceCount_ - 1).length};
    return eof;
}

int TextBuffer::locToGlobal(Location initLoc)
{
    int offset = 0;
    for(int i = 0; i < initLoc.pieceIndex; i++)
    {
        Piece currentPiece = piece(i);
        offset += currentPiece.length;
    }
    return offset + initLoc.inPieceOffset; 
}

Location TextBuffer::findPreviousRowStart(Location initLoc)
{
    bool currentNewlinePassed = false;
    bool initialPiecePassed = false;
    Location currentStart = initLoc;
    for(int i = currentStart.pieceIndex; i >= 0; i--)
    {
        Piece currentPiece = piece(i);
        const std::string& buffer = giveBuffer(currentPiece);
        if(initialPiecePassed) currentStart.inPieceOffset = currentPiece.length - 1;
        for(int j = currentStart.inPieceOffset; j >= 0; j--)
        {
            if(buffer.at(currentPiece.start + j) == '\n')
            {
                if(currentNewlinePassed)
                {
                    if(j < currentPiece.length - 1) return {i, j + 1};
                    else if(i < pieceCount() - 1) return {i + 1, 0};
                }
                if(!(initLoc.sameAs({i, j}))) currentNewlinePassed = true;
            }
        }
        initialPiecePassed = true;
    }
    return {0, 0};
}

Piece TextBuffer::newPiece()
{
    int newStart;
    if(appended_.size() == 0) newStart = 0;
    else newStart = appended_.size() - 1;
    Piece middlePart = {newStart, 1, APPENDED};
    return middlePart;
}

EditStatus TextBuffer::dividePiece(Location insertLoc, Purpose usedFor)
{
    Piece currentPiece = piece(insertLoc.pieceIndex);
    if(insertLoc.inPieceOffset == 0) return StartOfPiece;
    if(usedFor == DELETION && insertLoc.inPieceOffset == currentPiece.length - 1) return EndOfPiece;
    if((insertLoc.pieceIndex == pieceCount() - 1 && insertLoc.inPieceOffset == currentPiece.length) || pieces.size() == 0) return EndOfFile;
    
    Piece leftPart = {currentPiece.start, insertLoc.inPieceOffset, currentPiece.source};
    Piece rightPart = {currentPiece.start + insertLoc.inPieceOffset, currentPiece.length - insertLoc.inPieceOffset, currentPiece.source};
    
    pieces.at(insertLoc.pieceIndex) = leftPart;
    pieces.insert(pieces.begin() + insertLoc.pieceIndex + 1, rightPart);
    return MidPiece;
}

Status TextBuffer::deletion(Location insertLoc)
{
    Piece currentPiece = piece(insertLoc.pieceIndex);
    const std::string& buffer = giveBuffer(currentPiece);
    char deletedChar = buffer.at(currentPiece.start + insertLoc.inPieceOffset);
    
    EditStatus status = dividePiece(insertLoc, DELETION);
    switch(status)
    {
        case StartOfPiece:
            currentPiece.start++;
            currentPiece.length--;
            break;
        case MidPiece:
            currentPiece = piece(++insertLoc.pieceIndex);
            currentPiece.start++;
            currentPiece.length--;
            break;
        case EndOfPiece:
        case EndOfFile:
            currentPiece.length--;
            break;
    }
    totalSize_--;
    setContInsert(false);
    if(currentPiece.length == 0) 
        pieces.erase(pieces.begin() + insertLoc.pieceIndex);
    else pieces.at(insertLoc.pieceIndex) = currentPiece;
    if(deletedChar == '\n') return NewlineDeleted;
    return Success;
}

void TextBuffer::edit(int input, Location insertLoc)
{
    appended_ += input;
    
    if(contInsert())
    {
        pieces.at(std::max(0, insertLoc.pieceIndex - 1)).length++;
        totalSize_++;
        return;
    }
    
    EditStatus editStatus = dividePiece(insertLoc, INSERTION);
    if(editStatus == StartOfPiece) pieces.insert(pieces.begin() + insertLoc.pieceIndex, newPiece());
    else if(editStatus == EndOfFile) pieces.push_back(newPiece());
    else pieces.insert(pieces.begin() + insertLoc.pieceIndex + 1, newPiece());
    setContInsert(true);
    totalSize_++;
}

void TextBuffer::setContInsert(bool state)
{
    contInsert_ = state;
}

const std::string& TextBuffer::giveBuffer(Piece& piece) const
{
    return (piece.source == ORIGINAL? original_ : appended_);
}

int TextBuffer::pieceCount() const { return static_cast<int>(pieces.size()); }
int TextBuffer::totalSize() const { return totalSize_; }
Piece TextBuffer::piece(size_t index) const 
{ 
    if(index < pieces.size()) return pieces.at(index);
    else return {0, 0, APPENDED};
}
const std::string TextBuffer::original() const { return original_; }
const std::string TextBuffer::appended() const { return appended_; }
bool TextBuffer::contInsert() const { return contInsert_; }
