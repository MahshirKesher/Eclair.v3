#pragma once

#include <string>
#include <vector>
#include "sharedTypes.hpp"

enum Purpose
{
    INSERTION,
    DELETION
};

enum EditStatus
{
    StartOfPiece = 150,
    MidPiece,
    EndOfPiece,
    EndOfFile,
};

class TextBuffer
{
    public:
        TextBuffer(std::string init);
        
        Location globalToLoc(int offset);
        int locToGlobal(Location initLoc);
        
        Location findPreviousRowStart(Location initLoc);
        
        Piece newPiece();
        EditStatus dividePiece(Location insertLoc, Purpose usedFor);
        
        Status deletion(Location insertLoc);
        
        void edit(int input, Location insertionLoc);
        
        // SETTERS
        
        void setContInsert(bool state);
        
        // GETTERS
        
        int pieceCount() const;
        int totalSize() const;
        Piece piece(size_t index) const;
        
        const std::string original() const;
        const std::string appended() const;
        
        const std::string& giveBuffer(Piece& piece) const;
        
        bool contInsert() const;
        
    private:
        std::string original_;
        std::string appended_;
        
        bool contInsert_;
        
        int totalSize_;
        
        std::vector<Piece> pieces;
};
