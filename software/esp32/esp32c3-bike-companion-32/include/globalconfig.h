#ifndef _GLOBALCONFIG_H
#define _GLOBALCONFIG_H

/**
 * 
 *  Configuration file for the companion.
 * 
**/

/**
 * 
 *      SPI Configuration
 * 
**/
#define SPI_FREQ 8000000
#define DISPLAY_CS  D3
#define SDCARD_CS   D2

/**
 * 
 *      GNSS module
 * 
**/
#define GNSS_BAUD   115200
#define GNSS_ENABLE_MULTI_CONSTELLATION true
#define GNSS_ENABLE_LOW_POWER false
#define GNSS_ENABLE_HIGH_REFRESH false
#define GNSS_MIN_UPDATE_TIME_MS 1000

/**
 * 
 *      Display module
 * 
**/
#define BLACK 0
#define WHITE 1

#define DISPLAY_WIDTH 144
#define DISPLAY_HEIGHT 168
#define POSITION_MARKER_SIZE 8


/**
 * 
 *      Geographic utilities
 * 
**/

// Radius of the earth.
#define R_EARTH 6378137.0

/**
 * 
 *      Map Rendering
 * 
**/

// Default map zoom level. 1.0 means that exactly one tile fits on the display.
#define DETAULT_ZOOM_LEVEL 1.25

// Number of tiles per dimension to keep in memory. Must be an uneven number > 1
#define RENDER_TILES_PER_DIM 3

// If the map should be rotated to be alinged with heading.
#define RENDER_HEADING true

// Minimum time between two consecutive tile data updates in milliseconds.
#define TILE_UPDATE_DEBOUNCE_MS 5000

// Minimum free heap memory required after tile buffer allocation
#define MIN_FREE_HEAP 10000


/**
 * 
 *      Settings for UI
 * 
**/
#define TARGET_FPS 60

#define BOOTSCREEN_SPINNER_SIZE 10
#define BOOTSCREEN_TEXT_SIZE 2

// Statusbar
// Number of characters per stat in the status bar, including terminator
#define N_CHAR_PER_STAT 6
// Status bar element codes:
// 0 = time
// 1 = date
// 2 = speed
// 3 = heading
// 4 = lat
// 5 = lon
// 6 = nsats
// Left element of status bar
#define LEFT_STAT 0
// Right element of status bar
#define RIGHT_STAT 3




/**
 * 
 * Derived globals (DO NOT CHANGE)
 * 
**/

#define TARGET_FRAME_TIME_MS 1000/TARGET_FPS

#define DISPLAY_MAX_DIM sqrt(DISPLAY_WIDTH*DISPLAY_WIDTH/2)
#define DISPLAY_WIDTH_HALF DISPLAY_WIDTH/2
#define DISPLAY_HEIGHT_HALF DISPLAY_HEIGHT/2

#define RENDER_TILES_PER_DIM_HALF RENDER_TILES_PER_DIM / 2
#define N_RENDER_TILES RENDER_TILES_PER_DIM*RENDER_TILES_PER_DIM

#endif