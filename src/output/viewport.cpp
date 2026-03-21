#include "viewport.hpp"
#include <signal.h>

Viewport::Viewport(WinSize initSize)
{
    rows_ = initSize.rows;
    cols_ = initSize.cols;
    rowOffset_ = 0;
    
    viewStart_ = {0, 0};
}

void Viewport::setWindowSize(WinSize newWinsize)
{
    if(newWinsize.rows > 0) rows_ = newWinsize.rows;
    if(newWinsize.cols > 0) cols_ = newWinsize.cols;
}

void Viewport::setStart(Location newStart)
{
    if(newStart.pieceIndex >= 0 && newStart.inPieceOffset >= 0)
        viewStart_ = newStart;
}

void Viewport::setStart(int pieceIndex, int offset)
{
    if(pieceIndex >= 0 && offset >= 0)
    {
        viewStart_.pieceIndex = pieceIndex;
        viewStart_.inPieceOffset = offset;
    }
}

void Viewport::setRowOffset(int newOffset)
{
    if(newOffset >= 0) rowOffset_ = newOffset;
}

int Viewport::rows() const { return rows_; }
int Viewport::cols() const { return cols_; }
int Viewport::rowOffset() const { return rowOffset_; }
Location Viewport::viewStart() const { return viewStart_; }
