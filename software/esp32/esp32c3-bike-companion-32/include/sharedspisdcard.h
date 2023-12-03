#ifndef _SHAREDSPISDCARD_H
#define _SHAREDSPISDCARD_H

#include <Arduino.h>
#include <FS.h>
#include <sharedspidevice.h>
#include <simpletile.h>
#include <gpxtrack.h>

/*

    Wrapper class for the SD-Card reader with application-specific functionality.

*/
class SharedSPISDCard : public SharedSPIDevice {

    enum FileType {Map, GPXTrackIn, GPXTrackOut, None};

private:
    uint8_t _PIN_CS;
    File file;
    FileType _currFileType;
    char* _mapPath;
    char* _gpxTrackInPath;
    char* _gpxTrackOutPath;
    
    bool openFile(const char* path);
    bool openFile(FileType fileType);
    void closeFile();

public:
    uint64_t read_bytes;

    SharedSPISDCard(uint8_t PIN_CS);
    void enableCS();
    void disableCS();

    void initialize();

    void setMapPath(const char* mapPath);
    void setGPXTrackInPath(const char* gpxTrackInPath);
    void setGPXTrackOutPath(const char* gpxTrackOutPath);

    bool exists(const char* path);

    // Map reading
    bool readTile(SimpleTile::Header& header, int16_t* tile_node_buffer, uint64_t& tileSize, int tile_id);
    bool readHeader(SimpleTile::Header& header);
    
    // Input GPX reading
    bool readGPX(SimpleTile::Header& header, GPXTrack& track);


};

#endif