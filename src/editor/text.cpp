#include "text.hpp"

TextBuffer::TextBuffer(std::string init)
{
    if(init != "") 
    {
        original_ = init;
        Piece initPiece = {0, static_cast<int>(original_.size()), ORIGINAL};
        pieces.push_back(initPiece);
    }
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

const std::string& TextBuffer::giveBuffer(Piece& piece) const
{
    return (piece.source == ORIGINAL? original_ : appended_);
}

int TextBuffer::pieceCount() const { return static_cast<int>(pieces.size()); }
Piece TextBuffer::piece(size_t index) const 
{ 
    if(index < pieces.size()) return pieces.at(index);
    else return {0, 0, APPENDED};
}
const std::string TextBuffer::original() const { return original_; }
const std::string TextBuffer::appended() const { return appended_; }
