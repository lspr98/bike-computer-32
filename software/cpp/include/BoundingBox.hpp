#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <cstdint>
#include <osmium/osm/way.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/geom/mercator_projection.hpp>

/*

    Bounding box represented by lower left and upper right coordinate

*/

class BoundingBox {
    public:
        int32_t lower_x, lower_y, upper_x, upper_y;

        BoundingBox():
            lower_x(0), lower_y(0), upper_x(0), upper_y(0) {};

        BoundingBox(int32_t lower_x, int32_t lower_y, int32_t upper_x, int32_t upper_y): 
            lower_x(lower_x), lower_y(lower_y), upper_x(upper_x), upper_y(upper_y) {};

        // Check if two axis aligned 2D-boxes collide
        bool collidesWith(BoundingBox &b) {
            return (upper_x >= b.lower_x) && (b.upper_x >= lower_x) &&
                    (upper_y >= b.lower_y) && (b.upper_y >= lower_y) && valid();
        }

        // Check if a given point is inside the box
        bool contains(int32_t x, int32_t y) {
            return (lower_x <= x) && (x <= upper_x) && (lower_y <= y) && (y <= upper_y) && valid();
        }

        // Check if a given point is to the top and/or right of the box
        // From the perspective of the point, this means the tile is south and/or west of the point
        bool isSouthWestOf(int32_t x, int32_t y) {
            return ((upper_x < x) || (upper_y < y)) && valid();
        }

        // A bounding box is valid if the corners dont match.
        // Because of conversion to 64bit integer mercator coordinates,
        // some ways may become degenerate and are not valid anymore.
        bool valid() {
            return ((upper_x-lower_x > 0) || (upper_y-lower_y > 0));
        }
};


/*

    Bounding box containing a way, represented by lower left and upper right coordinate

*/
class WayBox : public BoundingBox {
    public:

        int way_id;

        // default constructor
        WayBox(): BoundingBox() {};

        // from explicit coordinates
        WayBox(int32_t lower_x, int32_t lower_y, int32_t upper_x, int32_t upper_y) :
            BoundingBox(lower_x, lower_y, upper_x, upper_y) {};

        // from way
        WayBox(const osmium::Way &w) {
            // Get the lat/lon lower left and upper right via osm envelope
            auto osmBox = w.nodes().envelope();
            
            // Convert to mercator
            auto bottom_left = osmium::geom::lonlat_to_mercator(osmBox.bottom_left());
            auto upper_right = osmium::geom::lonlat_to_mercator(osmBox.top_right());
            lower_x = bottom_left.x;
            lower_y = bottom_left.y;
            upper_x = upper_right.x;
            upper_y = upper_right.y;
            way_id = w.id();
        };

        // Calculate the number of tiles that collide with the bounding box
        int get_n_colliding_tiles(int tile_size, u_int64_t n_x_tiles, int map_x, int map_y) {
            int offset_ll_x = lower_x - map_x;
            int offset_ll_y = lower_y - map_y;
            int offset_ur_x = upper_x - map_x;
            int offset_ur_y = upper_y - map_y;
            int dx_tiles = offset_ur_x/tile_size - offset_ll_x/tile_size + 1;
            int dy_tiles = offset_ur_y/tile_size - offset_ll_y/tile_size + 1;
            return dx_tiles*dy_tiles;
        }

        // Get the indices of tiles that collide with the bounding box
        void get_colliding_tiles(int tile_size, int n_x_tiles, int map_x, int map_y, int* idx_buffer) {
            int offset_ll_x = lower_x - map_x;
            int offset_ll_y = lower_y - map_y;
            int offset_ur_x = upper_x - map_x;
            int offset_ur_y = upper_y - map_y;

            int tile_idx_ll = n_x_tiles*(offset_ll_y/tile_size) + (offset_ll_x/tile_size);
            int dx_tiles = offset_ur_x/tile_size - offset_ll_x/tile_size + 1;
            int dy_tiles = offset_ur_y/tile_size - offset_ll_y/tile_size + 1;
            int offset = 0;
            for(int i=0; i<dy_tiles; i++) {
                for(int j=0; j<dx_tiles; j++) {
                    idx_buffer[offset] = j + tile_idx_ll + n_x_tiles*i;
                    offset++;
                }
            }
        }
};


#endif