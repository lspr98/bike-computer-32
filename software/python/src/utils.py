import numpy as np
import math
import matplotlib.pyplot as plt
import os
from matplotlib.patches import Rectangle

r_earth = 6378137

'''
    Convert WGS84 (Lat/Lon) coordinates to mercator (X/Y) coordinates through projection
    Inputs:
        coords[:, 0] are the LON coordinates
        coords[:, 1] are the LAT coordinates

    Outputs:
        coords, where:
            coords[:, 0] are the X coordinates
            coords[:, 1] are the Y coordinates
'''
# Taken from libosmium.
def WGS84_to_Mercator(coords):
    assert coords.shape[1]==2, f"Incompatible dimension of coordinate array. Should be (N, 2) but is {coords.shape}"
    # Convext LON to X
    coords[:, 0] = r_earth * np.deg2rad(coords[:, 0])
    # Convert LAT to Y
    coords[:, 1] = r_earth * ((((((((((-3.1112583378460085319e-23  * coords[:, 1] +
                               2.0465852743943268009e-19) * coords[:, 1] +
                               6.4905282018672673884e-18) * coords[:, 1] +
                              -1.9685447939983315591e-14) * coords[:, 1] +
                              -2.2022588158115104182e-13) * coords[:, 1] +
                               5.1617537365509453239e-10) * coords[:, 1] +
                               2.5380136069803016519e-9)  * coords[:, 1] +
                              -5.1448323697228488745e-6)  * coords[:, 1] +
                              -9.4888671473357768301e-6)  * coords[:, 1] +
                               1.7453292518154191887e-2)  * coords[:, 1]) / ((((((((((-1.9741136066814230637e-22  * coords[:, 1] +
                              -1.258514031244679556e-20)  * coords[:, 1] +
                               4.8141483273572351796e-17) * coords[:, 1] +
                               8.6876090870176172185e-16) * coords[:, 1] +
                              -2.3298743439377541768e-12) * coords[:, 1] +
                              -1.9300094785736130185e-11) * coords[:, 1] +
                               4.3251609106864178231e-8)  * coords[:, 1] +
                               1.7301944508516974048e-7)  * coords[:, 1] +
                              -3.4554675198786337842e-4)  * coords[:, 1] +
                              -5.4367203601085991108e-4)  * coords[:, 1] + 1.0)

    # coords[:, 1] = ((map_size/180.0) * (90 + coords[:, 1]))

    return coords.astype(int)


'''

'''
# Extract all highways consisting of provided node_locations in Mercator coordinates
def create_highway_data(node_locations, highways):
    highways_plotdata = {}
    for i, hw_id in enumerate(highways.keys()):
        curr_hw_latlon = np.array([node_locations[node_id] for node_id in highways[hw_id]])
        highways_plotdata[hw_id] = WGS84_to_Mercator(curr_hw_latlon)

    return highways_plotdata

'''
    Get all tile indices that collide with a box defined by a lower-left point (ll)
    and an upper-right point (ur). All coordinates in mercator [x, y]
    Inputs:
        coords[:, 0] are the LON coordinates
        coords[:, 1] are the LAT coordinates

    Outputs:
        coords, where:
            coords[:, 0] are the X coordinates
            coords[:, 1] are the Y coordinates
'''
def get_colliding_tiles(ll, ur, tile_size, n_x_tiles, map_origin):
    colliding_tile_idx = []
    # Map origin is lower-left corner of the map.

    # Determin offsets from origin
    offset_ll = np.array(ll) - np.array(map_origin)
    offset_ur = np.array(ur) - np.array(map_origin)
    # Determine tile index for tile that includes ll point
    tile_idx_ll = n_x_tiles*math.floor(offset_ll[0, 1] / tile_size) + math.floor(offset_ll[0, 0] / tile_size)

    # Determine number of tiles in x and y direction that collide with box
    dx_tiles = math.floor(offset_ur[0, 0] / tile_size) - math.floor(offset_ll[0, 0] / tile_size) + 1
    dy_tiles = math.floor(offset_ur[0, 1] / tile_size) - math.floor(offset_ll[0, 1] / tile_size) + 1
    
    # Generate tile ids
    for i in range(dy_tiles):
        colliding_tile_idx.extend([j + tile_idx_ll + n_x_tiles*i for j in range(0, dx_tiles)])

    return colliding_tile_idx



'''
    Get bounding box of highway as dictionary containing lower-left and upper-right corner.
    highway_data is a (N, 2) numpy array of 2D-mercator coordinates making up the highway
'''
def get_highway_bounding_box(highway_data):
    # Lower left corner
    ll = np.array([np.min(highway_data[:, 0]), np.min(highway_data[:, 1])])
    # Upper right corner
    ur = np.array([np.max(highway_data[:, 0]), np.max(highway_data[:, 1])])
    return {"ll": ll, "ur": ur}



'''
    Convert a tile index to the lower left coordinate of the tile
'''
def get_tile_ll(tile_idx, tile_size, n_x_tiles, map_origin):
    x_offset = (tile_idx % n_x_tiles) * tile_size
    y_offset = math.floor(tile_idx / n_x_tiles) * tile_size
    return np.array(map_origin) + np.array([x_offset, y_offset])


'''
    Get tile metadata given all nodes in the map
'''
def get_tile_params(node_locations, tile_size):
    # Get all node locations in map
    all_latlon = np.array([loc for loc in node_locations.values()])
    # Convert to XY
    all_node_coords = WGS84_to_Mercator(all_latlon)

    # Determine min and max bounds for X and Y
    x_min = np.min(all_node_coords[:, 0])
    x_max = np.max(all_node_coords[:, 0])

    y_min = np.min(all_node_coords[:, 1])
    y_max = np.max(all_node_coords[:, 1])

    min_coords = np.array([x_min, y_min])
    max_coords = np.array([x_max, y_max])

    # Determine number of tiles in each dimension
    n_tile_x = math.ceil((x_max-x_min)/tile_size)
    n_tile_y = math.ceil((y_max-y_min)/tile_size)
    # Determine total number of tiles
    n_tiles = n_tile_x * n_tile_y

    return min_coords, max_coords, n_tile_x, n_tiles



def create_tile_for_idx(min_coords, n_x_tiles, tile_size, highways_plotdata, tile_idx):
    # Get lower left coordinate of current tile
    tile_ll = get_tile_ll(tile_idx, tile_size, n_x_tiles, min_coords)
    
    # Get upper right corner of tile
    tile_ur = tile_ll + tile_size
        
    # Get all ways for tile
    tile_ways = {}
    for hw_id in highways_plotdata.keys():
        hw_coords = highways_plotdata[hw_id]
        inidx = np.all(np.logical_and(tile_ll <= hw_coords, hw_coords <= tile_ur), axis=1)
        if np.any(inidx):
            tile_ways[hw_id] = hw_coords[inidx]
    
    
    print(f"Tile {tile_idx}")
    print(f"\t Number of highways: \t\t\t{len(tile_ways.keys())}")
    #print(f"\t Tile origin: \t\t\t {tile_ll}")
    #print(f"\t Tile UpperRight: \t\t {tile_ur}")
    
    tile = {}
    tile["id"] = tile_idx
    tile["ways"] = tile_ways
    tile["ur"] = tile_ur
    tile["ll"] = tile_ll
    
    return tile



'''
    Get the tile index of the tile that contains a given lat/lon coordinate
'''
def get_tileid_from_latlon(lat, lon, header):
    # Convert to mercator
    merc_coords = WGS84_to_Mercator(np.array(([[lon, lat]])))
    
    return get_tileid_from_merc(merc_coords, header)



'''
    Get the tile index of the tile that contains a given mercator xy coordinate
'''
def get_tileid_from_merc(merc_coords, header):
    # Shift by origin
    merc_coords -= np.array([[header["map_x"], header["map_y"]]])
    

    # Check if coordinate is outside of map
    # Case 1: coordinate is to south or west of map
    assert np.any(merc_coords > 0), f"The given coordinate is outside the map!"
    # Case 2: coordinate is to north or east of map
    assert np.any(merc_coords < np.array([[header["map_width"], header["map_height"]]])), f"The given coordinate is outside the map!"

    # Determine tile id
    tile_size = header["tile_size"]
    n_x_tiles = header["n_x_tiles"]
    return n_x_tiles*math.floor(merc_coords[0][1] / tile_size) + math.floor(merc_coords[0][0] / tile_size)


'''
    Read header of map which includes statistics like how many tiles are present in the file.
'''
def read_header(binary_path):
    # Parse header
    # buffer_header[0] = (uint64_t) stats.min_x;
    # buffer_header[1] = (uint64_t) stats.min_y;
    # buffer_header[2] = (uint64_t) map_width;
    # buffer_header[3] = (uint64_t) map_height;
    # buffer_header[4] = (uint64_t) n_x_tiles;
    # buffer_header[5] = (uint64_t) tile_size;
    # buffer_header[6] = (uint64_t) n_tiles;
    # buffer_header[7] = (uint64_t) max_tile_nodes;
    # buffer_header[8] = (uint64_t) total_tile_nodes;
    # buffer_header[9] = (uint64_t) stats.ways;
    header = {}
    header_keys = ["map_x", "map_y", "map_width", "map_height", "n_x_tiles", "tile_size", "n_tiles",
                "max_tile_nodes", "total_tile_nodes", "ways"]
    with open(binary_path, "rb") as f:
        # Parse header
        for i in range(10):
            bytes_read = f.read(8)
            is_signed = header_keys[i] in ["map_x", "ma_y"]
            header[header_keys[i]] = int.from_bytes(bytes_read, byteorder='little', signed=is_signed)

    return header


'''
    Read data for tile with index tile_idx from a binary file
'''
def read_tile(tile_idx, header, binary_path):
    # Get file size
    f_size = os.path.getsize(binary_path)
    # Offset at which the ptr to the tile can be read.
    tile_ptr_offset = 10*8 + tile_idx*8
    # Start of tile data section in file
    offset = 10*8 + header["n_tiles"]*8
    
    tile = {}
    
    with open(binary_path, "rb") as f:
        # Read tile pointer
        f.seek(tile_ptr_offset)
        # Get pointer to start of tile data for current tile
        tile_start = int.from_bytes(f.read(8), byteorder='little', signed=False) + offset
        # Check if it is the last tile.
        if tile_idx == header["n_tiles"]-1:
            # Tile is the last tile. End of tile data is end of file.
            tile_end = f_size
        else:
            # Get pointer to start of tile data for next tile
            tile_end = int.from_bytes(f.read(8), byteorder='little', signed=False) + offset
        # Calculate tile size and number of coordinates in tile (including separators)
        tile_size = tile_end - tile_start
        n_coords = tile_size // 4

        # Return if tile is empty
        if not n_coords:
            return tile
        # Read tile otherwise
        f.seek(tile_start)
        curr_way_id = 0
        curr_way = []
        for i in range(0, n_coords):
            bytes_read = f.read(4)
            # Parse coordinate
            curr_coord = [
                int.from_bytes(bytes_read[0:2], byteorder='little', signed=True),
                int.from_bytes(bytes_read[2:4], byteorder='little', signed=True)
            ]

            if curr_coord[0]==0 and curr_coord[1]==0:
                # End of way.
                # Append way
                if len(curr_way) > 0:
                    tile[curr_way_id] = np.array(curr_way)
                # Increase way counter
                # Start new way
                curr_way_id += 1
                curr_way = []

            else:
                # Append coordinate to current way
                curr_way.append(curr_coord)
    return tile



'''
    Plot tiles given by a tile index list in a given plot object
'''
def plot_tiles(ax, tile_idx_list, header, bin_file_path):

    ll_plot = np.array([1e10, 1e10])
    ur_plot = np.array([0, 0])
    # 1. Get the tiles.
    tile_list = [read_tile(idx, header, bin_file_path) for idx in tile_idx_list]

    map_origin = np.array([[header["map_x"], header["map_y"]]])

    # 2. Plot ways of each tile.
    for tile, tile_idx in zip(tile_list, tile_idx_list):
        # Plot tile outline
        ll = get_tile_ll(tile_idx, header["tile_size"], header["n_x_tiles"], map_origin)
        for hw_id in tile.keys():
            tile[hw_id] += ll
        ur = ll + np.array([[header["tile_size"], header["tile_size"]]])
        if ll_plot[0] > ll[0, 0]:
            ll_plot[0] = ll[0, 0]
        if ll_plot[1] > ll[0, 1]:
            ll_plot[1] = ll[0, 1]
        if ur_plot[0] < ur[0, 0]:
            ur_plot[0] = ur[0, 0]
        if ur_plot[1] < ur[0, 1]:
            ur_plot[1] = ur[0, 1]
        ax.add_patch(
            Rectangle((ll[0, 0], ll[0, 1]), header["tile_size"], header["tile_size"], 
                edgecolor = 'grey',
                fill=False)
        )

        for hw in tile.values():
            ax.plot(hw[:, 0], hw[:, 1], color='red')
    
    ax.set_aspect('equal', adjustable='box')
    ax.set_xlim(ll_plot[0] - 100, ur_plot[0] + 100)
    ax.set_ylim(ll_plot[1] - 100, ur_plot[1] + 100)