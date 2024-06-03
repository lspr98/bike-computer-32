#ifndef CUSTOM_HANDLERS_H
#define CUSTOM_HANDLERS_H

#include <osmium/handler.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>

#include <BoundingBox.hpp>
#include <Tile.hpp>


/*

    Handler to collect statistics about the input OSM file. These statistics are needed
    to determine tile layout, map size, etc...

*/
struct StatHandler : public osmium::handler::Handler {

    // Total number of ways and ways with tag "highway"
    std::uint64_t ways      = 0;
    std::uint64_t highways  = 0;
    // ID-Statistics
    int min_way_id          = 1206467986;
    int max_way_id          = 0;
    // Minimum and maximum coordinates
    int64_t min_x          = 1e10;
    int64_t min_y          = 1e10;
    int64_t max_x          = -1e10;
    int64_t max_y          = -1e10;
    double min_lat          = 1e10;
    double max_lat          = -1e10;
    double min_lon          = 1e10;
    double max_lon          = -1e10;
    // Maximum number of nodes in a way, all nodes for all ways
    int max_way_node_count = 0;
    int all_way_node_count = 0;

    osmium::geom::Coordinates merc_coords;

    void way(const osmium::Way& way) noexcept {
        const char* highway = way.tags()["highway"];
        if(way.id() > max_way_id) {
            max_way_id = way.id();
        }
        if(way.id() < min_way_id) {
            min_way_id = way.id();
        }
        ways++;
        if (highway) {
            highways++;
            int n_nodes = way.nodes().size();
            if(n_nodes > max_way_node_count) {
                max_way_node_count = n_nodes;
            }
            all_way_node_count += n_nodes;
            for(auto &node : way.nodes()) {
                if(node.location().lat() < min_lat) {
                    min_lat = node.location().lat();
                }
                if(node.location().lat() > max_lat) {
                    max_lat = node.location().lat();
                }
                if(node.location().lon() < min_lon) {
                    min_lon = node.location().lon();
                }
                if(node.location().lon() > max_lon) {
                    max_lon = node.location().lon();
                }
                merc_coords = osmium::geom::lonlat_to_mercator(node.location());
                if(merc_coords.x < min_x) {
                    min_x = merc_coords.x;
                }
                if(merc_coords.x > max_x) {
                    max_x = merc_coords.x;
                }
                if(merc_coords.y < min_y) {
                    min_y = merc_coords.y;
                }
                if(merc_coords.y > max_y) {
                    max_y = merc_coords.y;
                }
            }
        }
    }

    void printStatistics() {
        float avg_way_node_count = (float) all_way_node_count / (float) highways;
        std::cout << "all_way_node_count: \t\t" << all_way_node_count << "\n";
        std::cout << "Ways: \t\t\t\t"   << ways << "\n";
        std::cout << "Highways: \t\t\t"   << highways << "\n";
        std::cout << "Max Way-ID: \t\t\t" << max_way_id << "\n";
        std::cout << "Min Way-ID: \t\t\t" << min_way_id << "\n";
        std::cout << "Avg. nodes per street: \t\t" << avg_way_node_count << "\n";
        std::cout << "Max nodes in street: \t\t" << max_way_node_count << "\n";
        std::cout << "Map Width: \t\t\t" << max_x-min_x << "\n";
        std::cout << "Map Height: \t\t\t" << max_y-min_y << "\n";
        std::cout << "Min (x, y): \t\t\t(" << std::fixed << min_x << ", " << min_y << ")\n";
        std::cout << "Max (x, y): \t\t\t(" << std::fixed << max_x << ", " << max_y << ")\n";
        std::cout << "Min (Lon, Lat): \t\t(" << min_lon << ", " << min_lat << ")\n";
        std::cout << "Max (Lon, Lat): \t\t(" << max_lon << ", " << max_lat << ")\n";
    }

};



/*

    Handler to assign bounding boxes for all highways and calculate the number of colliding tiles

*/
struct BoxHandler : public osmium::handler::Handler {
    WayBox* _wbox_arr;
    int _tile_size, _n_x_tiles;
    // Count number of ways
    int way_cnt = 0;
    // Count number of undefined boxes
    int n_undefined = 0;
    int n_collisions = 0;
    int _map_x, _map_y;

    BoxHandler(WayBox* wbox_arr, int tile_size, int n_x_tiles, int map_x, int map_y) : 
    _wbox_arr(wbox_arr), _tile_size(tile_size), _n_x_tiles(n_x_tiles), _map_x(map_x), _map_y(map_y) {}

    void way(const osmium::Way& way) noexcept {
        const char* highway = way.tags()["highway"];
        if (highway) {
            _wbox_arr[way_cnt] = WayBox(way);
            n_collisions += _wbox_arr[way_cnt].get_n_colliding_tiles(_tile_size, _n_x_tiles, _map_x, _map_y);

            if(!_wbox_arr[way_cnt].valid()) {
                n_undefined++;
            }
            way_cnt++;
        }
    }

};


/*

    Handler to determine which highways belong on which tiles.
    Also determines how many nodes of these highways are on the tile.

*/
struct TileAssigner : public osmium::handler::Handler {

    int _n_collisions, _tile_size, _n_x_tiles, _map_x, _map_y, _max_way_node_count, _way_cnt;
    WayBox* _wBoxes;
    Tile _currTile;
    int n_idx_curr;
    int* idx_arr;
    uint16_t* _nodes_per_tile;
    int total_number_of_tile_nodes = 0;
    int n_negative_way_ids = 0;
    int n_negative_tile_ids = 0;

    TileAssigner(int n_collisions, int tile_size, int n_x_tiles, int map_x, int map_y, 
    int max_way_node_count, WayBox* wBoxes, uint16_t* nodes_per_tile) : 
        _n_collisions(n_collisions), _tile_size(tile_size), _n_x_tiles(n_x_tiles), 
        _map_x(map_x), _map_y(map_y), _max_way_node_count(max_way_node_count), 
        _way_cnt(0), _wBoxes(wBoxes), _nodes_per_tile(nodes_per_tile) {
    }
    
    // Iterate through all ways
    void way(const osmium::Way& way) noexcept {
        const char* highway = way.tags()["highway"];
        if (highway) {
            if(way.id() < 0) {
                n_negative_way_ids++;
            }
            // Get bounding box of current way
            auto curr_wBox = _wBoxes[_way_cnt];
            // Check that way ID matches bounding box ID
            assert(curr_wBox.way_id == way.id());

            // Get number of colliding tiles
            n_idx_curr = curr_wBox.get_n_colliding_tiles(_tile_size, _n_x_tiles, _map_x, _map_y);
            // Allocate memory for tile indices
            idx_arr = new int[n_idx_curr];
            
            // Get colliding tile indices
            curr_wBox.get_colliding_tiles(_tile_size, _n_x_tiles, _map_x, _map_y, idx_arr);
            // idx_arr now contains the indices for all colliding tiles.

            // Find containing nodes for each colliding tile
            // i iterates over all potentially colliding tiles.
            for(int i=0; i<n_idx_curr; i++) {
                bool prev_node_in_tile = false;
                // Create tile
                _currTile = Tile(idx_arr[i], _tile_size, _n_x_tiles, _map_x, _map_y);
                uint j=0;
                auto it = way.nodes().begin();
                auto end = way.nodes().end();
                double prev_x = 0;
                double prev_y = 0;
                for(; it != end; ++it) {
                    double curr_x = osmium::geom::detail::lon_to_x(it->lon());
                    double curr_y = osmium::geom::detail::lat_to_y(it->lat());
                    // TODO: Separate counting of nodes and separators.
                    if(_currTile.contains(curr_x, curr_y)) {
                        // Check if we need to add the previous node too.
                        if(!prev_node_in_tile && j > 0) {
                            if(_currTile.isSouthWestOf(prev_x, prev_y)) {
                                // Add previous node.
                                _nodes_per_tile[idx_arr[i]]++;
                                total_number_of_tile_nodes++;
                            }
                        }
                        // Add current node
                        _nodes_per_tile[idx_arr[i]]++;
                        total_number_of_tile_nodes++;
                        
                        // Write separator if way ends.
                        if(j==(way.nodes().size()-1)) {
                            _nodes_per_tile[idx_arr[i]]++;
                        }
                        prev_node_in_tile = true;
                    } else {
                        // Current node is not in tile, but we need to add it if
                        // the previous one was added AND it is to the top right
                        // of the tile.
                        if(prev_node_in_tile && _currTile.isSouthWestOf(curr_x, curr_y)) {
                            // Add the one node that was to the top right of tile
                            _nodes_per_tile[idx_arr[i]]++;
                        }
                        // Write separator if previous node was in tile
                        // OR the way ends.
                        if(prev_node_in_tile || j==(way.nodes().size()-1)) {
                            _nodes_per_tile[idx_arr[i]]++;
                        }
                        // Signal that previous node was not in tile for
                        // next iteration.
                        prev_node_in_tile = false;
                    }
                    j++;
                    prev_x = curr_x;
                    prev_y = curr_y;
                }
            }

            // Avoid memory leak
            delete idx_arr;
            _way_cnt++;
        }
    }

};


/*

    Handler to convert all relevant node coordinates to mercator coordinates

*/
struct MercatorConverter : public osmium::handler::Handler {

    int32_t* _node_x_coords;
    int32_t* _node_y_coords;
    uint64_t* _highway_indices;
    int _way_cnt;
    uint64_t _coord_ptr;

    MercatorConverter(int32_t* node_x_coords, int32_t* node_y_coords, uint64_t* highway_indices) :
        _node_x_coords(node_x_coords), _node_y_coords(node_y_coords), _highway_indices(highway_indices), _way_cnt(0), _coord_ptr(0) {};

    void way(const osmium::Way& way) noexcept {
        const char* highway = way.tags()["highway"];
        if (highway) {
            // Mark start for current highway by writing coord pointer to highway indices array.
            // Points to the first node of the current highway.
            _highway_indices[_way_cnt] = _coord_ptr;

            // Iterate over all remaining nodes in the highway
            auto it = way.nodes().begin();
            for(uint64_t i=0; i<way.nodes().size(); i++) {
                _node_x_coords[_coord_ptr] = osmium::geom::detail::lon_to_x(it->lon());
                _node_y_coords[_coord_ptr] = osmium::geom::detail::lat_to_y(it->lat());

                _coord_ptr++;
                it++;
            }

            _way_cnt++;
        }
    }


};

#endif