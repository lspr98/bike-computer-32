#include <iostream>
#include <fstream>
#include <ostream>
#include <osmium/visitor.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/util/memory.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/tile.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include <BoundingBox.hpp>
#include <Tile.hpp>
#include <CustomHandlers.hpp>

using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;



int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cout << "Usage: osm2simpletile PATH_TO_INPUT_FILE PATH_TO_OUTPUT_FILE \n";
        return 1;
    }

    // Tile size (in mercator coordinates)
    int tile_size = 512;

    // Map statistics
    double map_height, map_width;
    int n_x_tiles, n_y_tiles, n_tiles, n_collisions, map_x, map_y, max_way_node_count, n_ways, n_undefined, max_tile_nodes, all_way_node_count;
    long total_tile_nodes, total_filesize;
    uint64_t highways;

    // Buffers for calculating sizes
    int byte_header, byte_ptr, byte_tiles;
    uint16_t* nodes_per_tile;
    uint16_t* byte_per_tile;
    uint64_t* ptr_per_tile;

    // Buffers to store coordinates and IDs
    uint32_t* node_x_coords;
    uint32_t* node_y_coords;
    uint64_t* highway_indices;

    // Read OSM input file
    const osmium::io::File input_file{argv[1]};

    // Create node location index and apply box handler to get bounding boxes for all ways
    index_type index;
    location_handler_type location_handler{index};

    // First pass. Generate statistics.
    std::cout << "------------------------- 1/6 Gathering map statistics -------------------------\n";
    osmium::io::Reader reader{input_file, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
    StatHandler stats;
    osmium::apply(reader, location_handler, stats);
    reader.close();

    map_height = stats.max_y-stats.min_y;
    map_width = stats.max_x-stats.min_x;
    n_x_tiles = ceil(map_width/tile_size);
    n_y_tiles = ceil(map_height/tile_size);
    n_tiles = n_x_tiles*n_y_tiles;
    all_way_node_count = stats.all_way_node_count;
    map_x = stats.min_x;
    map_y = stats.min_y;
    highways = stats.highways;
    max_way_node_count = stats.max_way_node_count;
    n_ways = stats.ways;

    stats.printStatistics();
    location_handler.clear();
    std::cout << "X-tiles: \t\t\t" << n_x_tiles << "\n";
    std::cout << "Y-tiles: \t\t\t" << n_y_tiles << "\n";
    std::cout << "Total tiles: \t\t\t" << n_tiles << "\n";

    // Create array to hold bounding boxes for all ways
    WayBox* wBoxes = new WayBox[highways];

    // Second pass. Get collisions between tiles and highways
    std::cout << "---------------------------- 2/6 Finding collisions ----------------------------\n";
    BoxHandler bHandler(wBoxes, tile_size, n_x_tiles, map_x, map_y);
    osmium::io::Reader reader2{input_file, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
    osmium::apply(reader2, location_handler, bHandler);
    reader2.close();

    n_collisions = bHandler.n_collisions;
    n_undefined = bHandler.n_undefined;

    location_handler.clear();

    std::cout << "all_way_node_count: \t\t" << all_way_node_count << "\n";
    std::cout << "Collisions: \t\t\t" << n_collisions << "\n";
    std::cout << "Undefined bboxes: \t\t" << n_undefined << "\n";


    // Third pass. Generate mapping between tiles and highways
    std::cout << "------------------------- 3/6 Mapping highways to tiles ------------------------\n";
    nodes_per_tile = new uint16_t[n_tiles] {0};
    osmium::io::Reader reader3{input_file, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
    TileAssigner tHandler(n_collisions, tile_size, n_x_tiles, map_x, map_y, max_way_node_count, wBoxes, nodes_per_tile);
    osmium::apply(reader3, location_handler, tHandler);
    reader3.close();
    total_tile_nodes = tHandler.total_number_of_tile_nodes;
    location_handler.clear();

    // Fourth pass. Get all mercator coordinates of all highways
    std::cout << "-------------------------- 4/6 Converting coordinates --------------------------\n";
    node_x_coords = new uint32_t[all_way_node_count];
    node_y_coords = new uint32_t[all_way_node_count];
    highway_indices = new uint64_t[highways];
    osmium::io::Reader reader4{input_file, osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};
    MercatorConverter mercConv(node_x_coords, node_y_coords, highway_indices);
    osmium::apply(reader4, location_handler, mercConv);
    reader4.close();
    location_handler.clear();


    std::cout << "--------------------- 5/6 Calculating storage requirements ---------------------\n";
    byte_per_tile = new uint16_t[n_tiles];
    ptr_per_tile = new uint64_t[n_tiles];

    // The header stores the following metadata:
    //  map_x       uint64  x-coordinate of lower left corner (mercator-web)
    //  map_y       uint64  y-coordinate of lower left corner (mercator-web)
    //  map_width   uint64
    //  map_height  uint64
    //  n_x_tiles   uint64  number of tiles in x direction
    //  tile_size   uint64  size of tile
    //  n_tiles     uint64  number of tiles
    //  max_nodes   uint64  largest number of nodes on single tile
    //  n_nodes     uint64  number of nodes
    //  n_ways      uint64  number of ways

    byte_header = 10*8;
    byte_ptr = 0;
    byte_tiles = 0;
    max_tile_nodes = -1;

    for(int i=0; i<n_tiles; i++) {
        if(nodes_per_tile[i] > max_tile_nodes) {
            max_tile_nodes = nodes_per_tile[i];
        }
        if(i==0) {
            ptr_per_tile[i] = 0;
        } else {
            ptr_per_tile[i] = ptr_per_tile[i-1] + byte_per_tile[i-1];
        }
        // Store size of current tile and allocate memory
        byte_per_tile[i] = 2*sizeof(uint16_t)*nodes_per_tile[i];

        // Add space needed for node locations and delimiters
        byte_tiles += byte_per_tile[i];
        // Add space needed for tile-pointer (64-bit pointer -> 8 byte)
        byte_ptr += 8;
    }

    delete byte_per_tile;
    delete nodes_per_tile;

    total_filesize = byte_header + byte_ptr + byte_tiles;

    std::cout << "Filesize total: \t\t" 
        << (total_filesize/(1000)) << "KB ~= " 
        << (total_filesize/(1000*1000)) << "MB ~= " 
        << (total_filesize/(1000*1000*1000)) << "GB\n";

    std::cout << "File composition: \t\t"
        << (int)(((float)byte_header/total_filesize)*100) << "% Header, "
        << (int)(((float)byte_ptr/total_filesize)*100) << "% Pointer, "
        << (int)(((float)byte_tiles/total_filesize)*100) << "% Tiles \n";


    
    std::cout << "------------------------------- 6/6 Writing map --------------------------------\n";
    // Finally, create the output file!
    // Create buffer for nodes
    uint64_t* buffer_header = (uint64_t*) new char[byte_header];
    // buffer_pointer has tile offsets in BYTE count
    uint64_t* buffer_pointer = ptr_per_tile;
    // buffer_tiles has the buffer data
    int16_t* buffer_tiles = (int16_t*) calloc(byte_tiles, sizeof(char));
    // Write header buffer
    //  map_x       uint64  x-coordinate of lower left corner (mercator-web)
    //  map_y       uint64  y-coordinate of lower left corner (mercator-web)
    //  map_width   uint64
    //  map_height  uint64
    //  n_x_tiles   uint64  number of tiles in x direction
    //  tile_size   uint64  size of tile
    //  n_tiles     uint64  number of tiles
    //  max_nodes   uint64  largest number of nodes on single tile
    //  n_nodes     uint64  number of nodes, including separators
    //  n_ways      uint64  number of ways
    buffer_header[0] = (uint64_t) map_x;
    buffer_header[1] = (uint64_t) map_y;
    buffer_header[2] = (uint64_t) map_width;
    buffer_header[3] = (uint64_t) map_height;
    buffer_header[4] = (uint64_t) n_x_tiles;
    buffer_header[5] = (uint64_t) tile_size;
    buffer_header[6] = (uint64_t) n_tiles;
    buffer_header[7] = (uint64_t) max_tile_nodes;
    buffer_header[8] = (uint64_t) total_tile_nodes;
    buffer_header[9] = (uint64_t) n_ways;

    int* idx_arr;
    Tile currTile;

    // Write tile buffer
    uint64_t* _local_pointer_offsets = (uint64_t*) calloc(n_tiles, sizeof(uint64_t));

    for(uint64_t hw_id=0; hw_id < highways; hw_id++) {
        // Get bounding box of current way
        auto curr_wBox = wBoxes[hw_id];
        // Get number of colliding tiles
        int n_idx_curr = curr_wBox.get_n_colliding_tiles(tile_size, n_x_tiles, map_x, map_y);
        // Allocate memory for tile indices
        idx_arr = new int[n_idx_curr];
        // Get colliding tile indices
        curr_wBox.get_colliding_tiles(tile_size, n_x_tiles, map_x, map_y, idx_arr);
        // idx_arr now contains the indices for all colliding tiles.

        // Find containing nodes for each colliding tile

        // ANY CHANGE TO THIS LOGIC HAS TO BE REPLICATED IN THE TILEASSINGER!
        // TODO: Make this better, it is horrible
        for(int i=0; i<n_idx_curr; i++) {
            // Create tile
            int curr_tile_id = idx_arr[i];
            currTile = Tile(curr_tile_id, tile_size, n_x_tiles, map_x, map_y);
            // J goes over all nodes for current hw!
            uint64_t j_end = 0;
            // Last highway.
            if(hw_id == highways-1) {
                j_end = all_way_node_count-1;
            } else {
                j_end = highway_indices[hw_id+1];
            }

            bool prev_node_in_tile = false;

            for(uint64_t j=highway_indices[hw_id]; j<j_end; j++) {
                if(currTile.contains(node_x_coords[j], node_y_coords[j])) {
                    // I=tile_idx
                    // J=node_idx
                    // Check if we need to add the previous node too
                    if(!prev_node_in_tile && j > highway_indices[hw_id]) {
                        if(currTile.isSouthWestOf(node_x_coords[j-1], node_y_coords[j-1])) {
                            // Add previous node.
                            currTile.write_global_coord(
                                node_x_coords[j-1],
                                node_y_coords[j-1],
                                buffer_tiles + (buffer_pointer[curr_tile_id] + _local_pointer_offsets[curr_tile_id])/2
                            );
                            // Increase local pointer offset
                            _local_pointer_offsets[curr_tile_id] += 2*sizeof(int16_t);
                        }
                    }
                    // Add current node in buffer for I-th tile
                    currTile.write_global_coord(
                        node_x_coords[j],
                        node_y_coords[j],
                        buffer_tiles + (buffer_pointer[curr_tile_id] + _local_pointer_offsets[curr_tile_id])/2
                    );
                    // This operation always writes two coordinates, so we increase the pointer offset by 2*sizeof(int16_t)
                    _local_pointer_offsets[curr_tile_id] += 2*sizeof(int16_t);
                    // Write separator if way ends.
                    if(j==j_end-1) {
                        currTile.write_way_separator(buffer_tiles + (buffer_pointer[curr_tile_id] + _local_pointer_offsets[curr_tile_id])/2);
                        _local_pointer_offsets[curr_tile_id] += 2*sizeof(int16_t);
                    }
                    // Signal that tile was added
                    prev_node_in_tile = true;
                } else {
                    // Current node is not in tile.
                    if(prev_node_in_tile && currTile.isSouthWestOf(node_x_coords[j], node_y_coords[j])) {
                        // Add the one node that was to the top right of tile
                        currTile.write_global_coord(
                            node_x_coords[j],
                            node_y_coords[j],
                            buffer_tiles + (buffer_pointer[curr_tile_id] + _local_pointer_offsets[curr_tile_id])/2
                        );
                        // This operation always writes two coordinates, so we increase the pointer offset by 2*sizeof(int16_t)
                        _local_pointer_offsets[curr_tile_id] += 2*sizeof(int16_t);
                    }
                    if(prev_node_in_tile || j==j_end-1) {
                        // Write separator if end of way.
                        currTile.write_way_separator(buffer_tiles + (buffer_pointer[curr_tile_id] + _local_pointer_offsets[curr_tile_id])/2);
                        _local_pointer_offsets[curr_tile_id] += 2*sizeof(int16_t);
                    }
                    prev_node_in_tile = false;
                }
            }
        }

        // Avoid memory leak
        delete idx_arr;
    }

    FILE* file = fopen(argv[2], "wb");
    fwrite(buffer_header, sizeof(buffer_header[0]), 10, file);
    fwrite(buffer_pointer, sizeof(buffer_pointer[0]), n_tiles, file);
    fwrite(buffer_tiles, sizeof(buffer_tiles[0]), byte_tiles/sizeof(buffer_tiles[0]), file);
    fclose(file);

    std::cout << "Map created successfully at: " << argv[2] << "\n";

}