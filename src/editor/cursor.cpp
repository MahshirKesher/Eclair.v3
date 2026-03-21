#include "cursor.hpp"

Cursor::Cursor()
{
    row_ = 0;
    col_ = 0;
    preferredColumn_ = 0;
}

void Cursor::moveTo(int row, int col)
{
    row_ = row;
    col_ = col;
}

void Cursor::setRow(int row)
{
    row_ = row;
}

void Cursor::setCol(int col)
{
    col_ = col;
}

void Cursor::setPrefCol(int col)
{
    preferredColumn_ = col;
}

int Cursor::row() const { return row_; }
int Cursor::col() const { return col_; }
int Cursor::prefCol() const { return preferredColumn_; }
