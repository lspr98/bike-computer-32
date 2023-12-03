# Conversion tool for OSM data
Commandline tool to convert a given osm/pbf file to an optimized tile-based map that can be uploaded to the sd-card and is rendered by the ESP32.

## Limitations
The current code is not optimized for large maps and requires a substatial amount of RAM. If the tool runs out of memory, it simply stops without any error.

## Building
So far only tested on ubuntu 22.04.3 LTS.
Install dependencies:
```
sudo apt install make cmake build-essential libprotozero-dev libexpat1-dev zlib1g-dev libbz2-dev libboost-dev libgdal-dev libosmium2-dev
```
Run build:
```
make compile
```
The executable will be placed in the newly created **build** folder. Feel free to move it anywhere you like.

## Usage
The utility takes exactly two input arguments:
```
INPUT_PATH      The path to the original osm/pbf file
OUTPUT_PATH     The path for the optimized map
```
The following command would create **germany.bin** from **germany.osm.pbf**
```
./osm2simpletile /path/to/germany.osm.pbf /path/to/germany.bin
```

## Checking the exported map
Once the binary map is exported, the python notebook under **software/python/notebooks/test_plot_partial_map.ipynb** can be used to plot an arbitrary section of the map.

## A note on computation time
On a laptop with a i7-6600u (2 Cores @ 3.6GHz) and 16GB RAM, converting the complete DACH-region took about 14 Minutes and required 14GB of memory.

## Map file
The tool only keeps those points of the map, which make up roads/hiking tracks/sidewalks, etc... (anything with tag "highway").
The binary file that is created by the tool consists of three parts: metadata, pointers and tile-data, stored sequentially in memory.
```
+----------+----------+----------+
| METADATA | POINTERS | TILEDATA |
+----------+----------+----------+
```
- **Metadata**: Contains information about the map, for example how many tiles in total are present in the map
- **Pointers**: Byte offsets for each tile that is stored in the map. Acts as a lookup table for tile-data. Stores a memory offset pointer to the first byte of a tile for each tile.
- **Tiledata**: Stores the actual groups of points which make up segments of a road.


