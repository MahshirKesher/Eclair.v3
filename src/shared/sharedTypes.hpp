#pragma once

#include <string>
#include <vector>

enum Status
{
    Success = 1,
    AttributeGettingError = 10,
    AttributeSettingError = 11,
    ReadingError = 20,
    NoInput = 21,
    SignalInterrupt = 22,
    WritingError = 30,
    TextBufferOverflow = 40,
    NewlineDeleted = 50,
    NoDeletionExecuted
};

enum Interrupt
{
    StandBy = 255,
    WinResize = 256
};

enum SOURCE
{
    ORIGINAL = 100,
    APPENDED
};

struct WinSize
{
    int rows;
    int cols;
};

struct Piece
{
    int start;
    int length;
    SOURCE source;
};

struct Location
{
    int pieceIndex;
    int inPieceOffset;
    
    bool sameAs(const Location otherLoc)
    {
        return (pieceIndex == otherLoc.pieceIndex) 
              && (inPieceOffset == otherLoc.inPieceOffset);
    }
};


