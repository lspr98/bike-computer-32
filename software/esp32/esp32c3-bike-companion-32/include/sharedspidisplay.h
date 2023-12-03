#ifndef _SHAREDSPIDISPLAY_H
#define _SHAREDSPIDISPLAY_H

#include <Arduino.h>
#include <sharedspidevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <globalconfig.h>

/*

    Wrapper class for the adafruit display.
    Includes extended functionality like drawing lines with a certain thickness.

*/

/*******************************************************************************
 * Sequence of offsets for line thickening.
 * 
 * Determines offsets of additional parallel lines with respect 
 * to the original line.
 ******************************************************************************/
const int thickness_offsets[6] = {1, -1, 2, -2, 3, -3};

class SharedSPIDisplay : public SharedSPIDevice {

private:
    Adafruit_SharpMem disp;
    float zoom;
    uint8_t _PIN_CS;

public:
    SharedSPIDisplay(uint8_t PIN_CS);
    void enableCS();
    void disableCS();

    void initialize();
    void newPage();

    void write(const char* str, bool hold=0);
    void clearDisplay();
    void clearDisplayBuffer();
    void setCursor(int16_t x, int16_t y);
    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawQuaterCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);

    void refresh();
    void drawCenterMarker();
    void drawStatusBar(const char* statusStr);

    /*******************************************************************************
     * Modified Bresenham algorithm based on the implementation from wikipeda 
     * (see https://de.wikipedia.org/wiki/Bresenham-Algorithmus)
     * 
     * Draws a line from p0=(x0, y0) to p1=(x1, y1) with a given thickness and color.
     * 
     * The modification draws parallel lines to increase line thickness.
     * The offset of the parallel line is determined by the thickness_offsets array.
     * For thickness = 1, the algorithm is equivalent to the original bresenham algorithm
     *
     * @param x0 x coordinate of first point (p0)
     * 
     * @param y0 y coordinate of first point (p0)
     * 
     * @param x1 x coordinate of second point (p1)
     * 
     * @param y1 y coordinate of second point (p1)
     * 
     * @param thickness thickness of the line that is drawn. Must take values in interval [1, 7].
     * 
     * @param color color of the line that is drawn.
     ******************************************************************************/
    void draw_line(int x0, int y0, int x1, int y1, uint8_t thickness, uint16_t color);

    void draw_dashedline(int x0, int y0, int x1, int y1, uint8_t thickness, uint8_t dashLength, uint16_t color);

};

#endif
