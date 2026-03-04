#include "Terminal.h"

#include "stdio.h"
#include "string.h"

void clearScreen()
{
    printf("\33[2J");    
}

void moveCursorUp(unsigned int lines)
{
    printf("\33[%dA", lines);
}

void moveCursorDown(unsigned int lines)
{
    printf("\33[%dB", lines);
}

void moveCursorLeft(unsigned int col)
{
    printf("\33[%dD", col);
}

void moveCursorRight(unsigned int col)
{
    printf("\33[%dC", col);
}

void setCursorPosition(unsigned int row, unsigned int col)
{
    printf("\33[%d;%dH", row, col);
}

void setColor(unsigned int color)
{
    printf("\33[%dm", color);
}

void setFGColorRGB(unsigned int r, unsigned int g, unsigned int b)
{
    printf("\33[38;2;%d;%d;%dm", r, g, b);
}

void setBGColorRGB(unsigned int r, unsigned int g, unsigned int b)
{
    printf("\33[48;2;%d;%d;%dm", r, g, b);
}

void setDecoration(unsigned int decor)
{
    printf("\33[%dm", decor);
}

void resetTextGraphics()
{
   printf("\33[0m");
}
