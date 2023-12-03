#ifndef CUSTOM_TILE_H
#define CUSTOM_TILE_H

#include <BoundingBox.hpp>

#define UINT16_T_MAX_VAL 65535

/*

    Class to represent a single tile of the map

*/

class Tile : public BoundingBox {

public:
    // Number of nodes in tile
    u_int16_t n_nodes;

    // Tile parameters. ID of tile, size of tile and total number of tiles along x direction
    int _tile_idx, _tile_size, _n_tiles_x;
    // Origin of map (lower left point)
    int _map_x, _map_y;

    Tile() : BoundingBox() {};

    Tile(int tile_idx, int tile_size, int n_tiles_x, int map_x, int map_y) : 
        _tile_idx(tile_idx), _tile_size(tile_size), _n_tiles_x(n_tiles_x), _map_x(map_x), _map_y(map_y) {
        
        lower_x = (tile_idx % n_tiles_x) * tile_size + map_x;
        lower_y = (tile_idx / n_tiles_x) * tile_size + map_y;
        upper_x = lower_x + tile_size;
        upper_y = lower_y + tile_size;
    };

    // Writes a global mercator coordinate in a uint16_t buffer with coordinates relative to tile origin.
    void write_global_coord(int x, int y, int16_t* buffer) {
        // Convert global coordinates to local coordinates
        int x_local = x-lower_x;
        int y_local = y-lower_y;
        // Edge case: local coordinate is exactly (0, 0) and thus its coordinate aligns with the global coordinate.
        // In this case, simply shift the coordinate by 1. This creates a small (but in most cases not visible) rendering error so its good enough for now
        if(!x_local && !y_local) x_local++;

        // At least one local coordinate must be positive (otherwise it is reserved for way metadata)
        assert(x_local > 0 || y_local > 0);

        // Must not be out-of-range before conversion
        assert(x_local < INT16_MAX && x_local > INT16_MIN);
        assert(y_local < INT16_MAX && y_local > INT16_MIN);
        buffer[0] = (int16_t) (x - lower_x);
        buffer[1] = (int16_t) (y - lower_y);
    }

    // Writes a separator (consisting of two consecutive zeros) into a buffer
    void write_way_separator(int16_t* buffer) {
        buffer[0] = (int16_t) 0;
        buffer[1] = (int16_t) 0;
    }

};


#endif