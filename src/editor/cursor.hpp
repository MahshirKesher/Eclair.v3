#pragma once

#include "keys.hpp"

class Cursor
{
    public:
        Cursor();
    
        int row() const;
        int col() const;
        int prefCol() const;
        
        void setRow(int row);
        void setCol(int col);
        void setPrefCol(int col);
        
    private:
        int row_;
        int col_;
        
        int preferredColumn_;
};
