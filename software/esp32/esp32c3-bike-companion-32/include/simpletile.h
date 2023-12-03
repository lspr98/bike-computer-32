#ifndef _SIMPLETILE_H
#define _SIMPLETILE_H

#include <Arduino.h>
#include <FS.h>


/*

    Definition of map header.

*/
namespace SimpleTile {

    // Header struct for map meta-data
    struct Header {
        uint64_t map_x;
        uint64_t map_y;
        uint64_t map_width;
        uint64_t map_height;
        uint64_t n_x_tiles;
        uint64_t tile_size;
        uint64_t n_tiles;
        uint64_t max_nodes;
        uint64_t n_nodes;
        uint64_t n_ways;

        void print() {
            Serial.printf("map_X: %i\n", map_x);
            Serial.printf("map_Y: %i\n", map_y);
            Serial.printf("map_width: %i\n", map_width);
            Serial.printf("map_height: %i\n", map_height);
            Serial.printf("n_x_tiles: %i\n", n_x_tiles);
            Serial.printf("tile_size: %i\n", tile_size);
            Serial.printf("n_tiles: %i\n", n_tiles);
            Serial.printf("max_nodes: %i\n", max_nodes);
            Serial.printf("n_nodes: %i\n", n_nodes);
            Serial.printf("n_ways: %i\n", n_ways);
        }
    };

}


#endif