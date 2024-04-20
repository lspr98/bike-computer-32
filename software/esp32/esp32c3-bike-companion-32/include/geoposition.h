#ifndef _GEOPOSITION_H
#define _GEOPOSITION_H

#include <Arduino.h>
#include <simpletile.h>
#include <globalconfig.h>

/*

    Class to describe a location in both coordinate systems (WGS and Mercator).
    Includes conversion utilities between the two systems.

*/
class GeoPosition {

protected:
    int64_t _x, _y;
    double _lat, _lon;

public:
    // Default
    GeoPosition();
    // From Mercator
    GeoPosition(int64_t x, int64_t y);
    // From WGS
    GeoPosition(double lat, double lon);

    // Converter
    void mercatorToWGS(int64_t x, int64_t y, double* latBuf, double* lonBuf);
    void WGSToMercator(double lat, double lon, int64_t* xBuf, int64_t* yBuf);

    // Getter methods
    int64_t x();
    int64_t y();
    double lat();
    double lon();

    // Setter methods
    void updatePosition(int64_t xNew, int64_t yNew);
    void updatePosition(double lat, double lon);

};


/*

    Class to describe a location in local mercator coordinates (with respect to the tile origin)

*/
class LocalGeoPosition : public GeoPosition {

private:
    // Offset from tile origin to coordinate (position on tile)
    int16_t _xLocal, _yLocal;
    // Offset from tile origin to map origin
    int64_t _tileOriginX, _tileOriginY;
    // ID of tile on which coordinate is located
    uint64_t _tileId;
    // Header of mapdata.
    SimpleTile::Header* _header;

public:
    LocalGeoPosition(GeoPosition& globalPos, SimpleTile::Header* mapHeader);

    LocalGeoPosition(int64_t xGlobal, int64_t yGlobal, SimpleTile::Header* mapHeader);

    void getTileBlock(uint64_t* tilesIdBuf);
    uint64_t getTileID();
    // TODO: Should probably belong to the SimpleTile class
    static uint64_t getTileID(GeoPosition& globalPos, SimpleTile::Header* mapHeader);
    static uint64_t getTileID(int64_t xGlob, int64_t yGlob, int64_t xMap, int64_t yMap, uint64_t tileSize, uint64_t nXTiles);

    // Get lower left corner of tile (tile origin) in global coordinates
    // TODO: Should probably belong to the SimpleTile class
    static void getTileLL(uint64_t tileId, SimpleTile::Header* mapHeader, int64_t* tileOriginXBuf, int64_t* tileOriginYBuf);
    static void getTileLL(uint64_t tileId, uint64_t tileSize, uint64_t nXTiles, int64_t xMap, int64_t yMap, int64_t* tileOriginXBuf, int64_t* tileOriginYBuf);

    // Getter methods
    uint64_t tileId();

    int16_t xLocal();
    int16_t yLocal();
};

#endif