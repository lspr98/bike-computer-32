#ifndef UTILS_DRAWING_H
#define UTILS_DRAWING_H

#include <Arduino.h>
#include <Adafruit_SharpMem.h>

/*******************************************************************************
 * Sequence of offsets for line thickening.
 * 
 * Determines offsets of additional parallel lines with respect 
 * to the original line.
 ******************************************************************************/
const int thickness_offsets[6] = {1, -1, 2, -2, 3, -3};


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
 * @param display Display object of adafruit library on which drawPixel is called
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
void draw_line(Adafruit_SharpMem display, int x0, int y0, int x1, int y1, uint8_t thickness, uint16_t color) {

  int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  if(dx < -dy) {
    while (1) {
      display.drawPixel(x0, y0, color);
      for(int i=0; i<thickness-1; i++) {
        display.drawPixel(x0+thickness_offsets[i], y0, color);
      }
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      if (e2 > dy) { err += dy; x0 += sx; }
      if (e2 < dx) { err += dx; y0 += sy; }
    }
  } else {
    while (1) {
      display.drawPixel(x0, y0, color);
      for(int i=0; i<thickness-1; i++) {
        display.drawPixel(x0, y0+thickness_offsets[i], color);
      }
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      if (e2 > dy) { err += dy; x0 += sx; }
      if (e2 < dx) { err += dx; y0 += sy; }
    }
  }

}

#endif