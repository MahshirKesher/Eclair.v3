#include "text.hpp"

TextBuffer::TextBuffer(std::string initContent)
{
    original_ = initContent;
    Piece initPiece = {0, static_cast<int>(original_.size()), ORIGINAL};
    pieces.push_back(initPiece);
}

const std::string& TextBuffer::giveBuffer(Piece& piece) const
{
    return (piece.source == ORIGINAL? original_ : appended_);
}

size_t TextBuffer::pieceCount() const { return pieces.size(); }
Piece TextBuffer::piece(size_t index) const 
{ 
    if(index < pieces.size()) return pieces.at(index);
    else return {0, 0, APPENDED};
}
const std::string TextBuffer::original() const { return original_; }
const std::string TextBuffer::appended() const { return appended_; }
