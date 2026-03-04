#ifndef TERMINAL_H
#define TERMINAL_H

/* Information for this project was provided by the following links:
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 * https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b
 * https://stackoverflow.com/questions/7414983/how-can-i-use-ansi-escape-codes-for-outputting-colored-text-in-c-and-c
 */

enum FG_Colors
{
    FG_BLACK=30,
    FG_RED,
    FG_GREEN,
    FG_YELLOW,
    FG_BLUE,
    FG_MAGENTA,
    FG_CYAN,
    FG_WHITE,
};

enum BG_Colors
{
    BG_BLACK=40,
    BG_RED,
    BG_GREEN,
    BG_YELLOW,
    BG_BLUE,
    BG_MAGENTA,
    BG_CYAN,
    BG_WHITE,
};

enum Decoration
{
    DECOR_BOLD = 1,
    DECOR_DIM,
    DECOR_ITALIC,
    DECOR_UNDERLINE,
    DECOR_BLINKING,
};

/**
 * @brief Clears the terminal of all text.
 */
void clearScreen();

/**
 * @brief Moves cursor up a given number of lines.
 * 
 * @param lines The number of lines to move up.
 */
void moveCursorUp(unsigned int);

/**
 * @brief Move cursor down a given number of lines.
 * 
 * @param lines The number of lines to move down.
 */
void moveCursorDown(unsigned int);

/**
 * @brief Move cursor left a given number of columns.
 * 
 * @param col The number of columns to move left.
 */
void moveCursorLeft(unsigned int);

/**
 * @brief Move cursor right a given number of columns.
 * 
 * @param col The number of columns to move right.
 */
void moveCursorRight(unsigned int);

/**
 * @brief Set cursor position by specifying the row and column.
 * 
 * @param row The y-axis position to set the cursor to.
 * @param col The x-axis position to set the cursor to.
 * 
 * @note Positions are not 0-indexed and use 1 as the base instead.
 */
void setCursorPosition(unsigned int, unsigned int);

/**
 * @brief Sets the terminal color.
 * 
 * @param color The color to set the terminal to.
 * 
 * @see FG_Colors & BG_Colors
 */
void setColor(unsigned int);

/**
 * @brief Set the terminal foreground color using 24-bit RGB.
 * 
 * @param r 8-bit value to set red channel to.
 * @param g 8-bit value to set green channel to.
 * @param b 8-bit values to set blue channel to.
 */
void setFGColorRGB(unsigned int, unsigned int, unsigned int);


/**
 * @brief Set the terminal background color using 24-bit RGB.
 * 
 * @param r 8-bit value to set red channel to.
 * @param g 8-bit value to set green channel to.
 * @param b 8-bit values to set blue channel to.
 */
void setBGColorRGB(unsigned int, unsigned int, unsigned int);

/**
 * @brief Sets the terminal decoration.
 * 
 * @param decor The decoration to apply to the terminal.
 */
void setDecoration(unsigned int);

/**
 * @brief Reset all modifications made to the text. 
 */
void resetTextGraphics();

#endif
