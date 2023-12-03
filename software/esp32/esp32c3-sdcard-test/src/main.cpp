#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

struct Header
{
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


// Selector pin for SPI of SD-Card reader
const int SS_PIN = D2;
// SD-Card detection pin of SD-Card reader (currently unused)
const int DET_PIN = D5;

// How often should we randomly choose N_TILES_READ tiles and read them
const int N_EXPERIMENTS = 100;

// Number of random tiles to read to evaluate performance
// When the map moves to in both directions, we need to read 5 new tiles in the worst case.
const int N_TILES_READ = 5;

// Array to store tile ids that should be read
int tile_ids[N_TILES_READ];

// Pointer to array where tile node data will be put
uint16_t* tile_node_buffer;

const char binary_path[] = "/bayern.bin";

// Header of bin map file
Header header;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readHeader(fs::File &file, Header& header) {
  
  if(file.available()) {
    file.readBytes((char*) &(header.map_x), 8);
    file.readBytes((char*) &(header.map_y), 8);
    file.readBytes((char*) &(header.map_width), 8);
    file.readBytes((char*) &(header.map_height), 8);
    file.readBytes((char*) &(header.n_x_tiles), 8);
    file.readBytes((char*) &(header.tile_size), 8);
    file.readBytes((char*) &(header.n_tiles), 8);
    file.readBytes((char*) &(header.max_nodes), 8);
    file.readBytes((char*) &(header.n_nodes), 8);
    file.readBytes((char*) &(header.n_ways), 8);
  }

}

void readTile(fs::File &file, Header& header, uint16_t* tile_node_buffer, int tile_id) {
  // Get pointer and size of tile
  uint64_t ptr_tile, ptr_next_tile, tile_size;

  // Move reader to tile pointer
  file.seek(sizeof(uint64_t)*(10 + tile_id));
  // Read tile pointer
  file.readBytes((char *) &ptr_tile, sizeof(uint64_t));
  // Read next tile pointer (if it is not the last tile)
  if(tile_id < header.n_tiles) {
    file.readBytes((char *) &ptr_next_tile, sizeof(uint64_t));
  } else {
    ptr_next_tile = file.size();
  }
  tile_size = ptr_next_tile - ptr_tile;

  // Move reader to start of tile
  file.seek(sizeof(uint64_t)*(10 + header.n_tiles) + ptr_tile);
  // Read tile
  file.readBytes((char *) tile_node_buffer, tile_size);

}

void setup(){
  Serial.begin(115200);
  sleep(3);
  while(!SD.begin(SS_PIN)) {
    Serial.println("Card Mount Failed");
    sleep(5);
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);
  sleep(3);

  // Open file
  File file = SD.open(binary_path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  // Read map header and print map statistics
  readHeader(file, header);
  header.print();

  Serial.println("Allocating memory for tile node buffer...");
  tile_node_buffer = new uint16_t[header.max_nodes*2];

  for(int j=0; j<N_EXPERIMENTS; j++) {
    Serial.println("Generating random tile ids...");
    for(int i=0; i<N_TILES_READ; i++) {
      tile_ids[i] = rand() % header.n_tiles;
    }
    long t_start = millis();
    for(int i=0; i<N_TILES_READ; i++) {
      readTile(file, header, tile_node_buffer, tile_ids[i]);
    }
    int t_end = millis() - t_start;
    float avg_read = (float) t_end / (float) N_TILES_READ;
    Serial.print("Avg time per tile: ");
    Serial.print(avg_read);
    Serial.println("ms");
  }


  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  file.close();

  free(tile_node_buffer);
}

void loop(){

}