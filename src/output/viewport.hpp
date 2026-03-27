#pragma once

#include "sharedTypes.hpp"

class Viewport
{
    public:
        Viewport(WinSize initSize);
    
        void setWindowSize(WinSize newWinSize);
        void setStart(Location newStart);
        void setEnd(Location newStart);
        
        int rows() const;
        int cols() const;
        int rowOffset() const;
        
        void setRowOffset(int newOffset);
        
        Location viewStart() const;
        Location viewEnd() const;
        
    private:
        int rows_, cols_;
        int rowOffset_;
        
        Location viewStart_;
        Location viewEnd_;
};
