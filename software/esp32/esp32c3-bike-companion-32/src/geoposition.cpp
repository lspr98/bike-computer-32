#include <geoposition.h>
#include <globalconfig.h>
#include <serialutils.h>

GeoPosition::GeoPosition() : _x(0), _y(0) {
    mercatorToWGS(_x, _y, &_lat, &_lon);
}

GeoPosition::GeoPosition(int64_t x, int64_t y) : _x(x), _y(y) {
    mercatorToWGS(x, y, &_lat, &_lon);
}

GeoPosition::GeoPosition(double lat, double lon) : _lat(lat), _lon(lon) {
    WGSToMercator(lat, lon, &_x, &_y);
}

void GeoPosition::updatePosition(int64_t x, int64_t y) {
    _x = x;
    _y = y;
    mercatorToWGS(x, y, &_lat, &_lon);
}

void GeoPosition::updatePosition(double lat, double lon) {
    _lat = lat;
    _lon = lon;
    WGSToMercator(lat, lon, &_x, &_y);
}

// TODO: Include libosmium license
// This conversion is taken from libosimum
void GeoPosition::mercatorToWGS(int64_t x, int64_t y, double* latBuf, double* lonBuf) {
    *lonBuf = (double) (RAD_TO_DEG*((double) x) / R_EARTH);
    *latBuf = (double) (RAD_TO_DEG*(2* atan(exp(((double) y) / R_EARTH))));
};


// TODO: Include libosmium license
// This conversion is taken from libosimum
void GeoPosition::WGSToMercator(double lat, double lon, int64_t* xBuf, int64_t* yBuf) {
    *xBuf = (int64_t) (R_EARTH * DEG_TO_RAD * lon);
    *yBuf = (int64_t) R_EARTH *
                    ((((((((((-3.1112583378460085319e-23  * lat +
                               2.0465852743943268009e-19) * lat +
                               6.4905282018672673884e-18) * lat +
                              -1.9685447939983315591e-14) * lat +
                              -2.2022588158115104182e-13) * lat +
                               5.1617537365509453239e-10) * lat +
                               2.5380136069803016519e-9)  * lat +
                              -5.1448323697228488745e-6)  * lat +
                              -9.4888671473357768301e-6)  * lat +
                               1.7453292518154191887e-2)  * lat)
                    /
                    ((((((((((-1.9741136066814230637e-22  * lat +
                              -1.258514031244679556e-20)  * lat +
                               4.8141483273572351796e-17) * lat +
                               8.6876090870176172185e-16) * lat +
                              -2.3298743439377541768e-12) * lat +
                              -1.9300094785736130185e-11) * lat +
                               4.3251609106864178231e-8)  * lat +
                               1.7301944508516974048e-7)  * lat +
                              -3.4554675198786337842e-4)  * lat +
                              -5.4367203601085991108e-4)  * lat + 1.0);
};


int64_t GeoPosition::x() { return _x; };
int64_t GeoPosition::y() { return _y; };

double GeoPosition::lat() { return _lat; };
double GeoPosition::lon() { return _lon; };

LocalGeoPosition::LocalGeoPosition(GeoPosition& globalPos, SimpleTile::Header* mapHeader)
    : _header(mapHeader), GeoPosition(globalPos) {

    // calculate tileID
    _tileId = getTileID(globalPos, mapHeader);
    // get tile origin (lower left corner of tile in global coordinates)
    getTileLL(_tileId, mapHeader, &_tileOriginX, &_tileOriginY);
    // get local coordinates
    _xLocal = globalPos.x() - _tileOriginX;
    _yLocal = globalPos.y() - _tileOriginY;

};


LocalGeoPosition::LocalGeoPosition(int64_t xGlobal, int64_t yGlobal, SimpleTile::Header* mapHeader)
    : _header(mapHeader), GeoPosition(xGlobal, yGlobal) {

    // calculate tileID
    _tileId = getTileID(*this, mapHeader);
    getTileLL(_tileId, mapHeader, &_tileOriginX, &_tileOriginY);
    // get local coordinates
    _xLocal = this->x() - _tileOriginX;
    _yLocal = this->y() - _tileOriginY;

};


void LocalGeoPosition::getTileBlock(uint64_t* neighborTilesIdBuf) {
    // Calculate ID offset of lower left tile in block
    uint64_t id_offset = RENDER_TILES_PER_DIM_HALF * (1 + _header->n_x_tiles);

    // Get ID of lower left tile
    uint64_t ll_id = _tileId - id_offset;

    // Generate all other IDs
    for(uint16_t i=0; i<N_RENDER_TILES; i++) {
        neighborTilesIdBuf[i] = ll_id + (i / RENDER_TILES_PER_DIM)*(_header->n_x_tiles) + (i % RENDER_TILES_PER_DIM);
    }

    // Check if the lower left or upper right tile ID would be out of bounds.
    // This can happen if the current position is outside of the map or near the border.
    if(id_offset > _tileId || (_tileId + id_offset) > _header->n_tiles) {
        // Iterate through array again and clip values to 0 that are out of bounds
        for(uint16_t i=0; i<N_RENDER_TILES; i++) {
            if(neighborTilesIdBuf[i] > _header->n_tiles || neighborTilesIdBuf[i] < 0) {
                neighborTilesIdBuf[i] = 0;
            }
        }
    }

};


uint64_t LocalGeoPosition::getTileID() {
    return getTileID(*this, _header);
}

uint64_t LocalGeoPosition::getTileID(GeoPosition& globalPos, SimpleTile::Header* mapHeader) {

    return getTileID(globalPos.x(), globalPos.y(), mapHeader->map_x, mapHeader->map_y,
        mapHeader->tile_size, mapHeader->n_x_tiles);

};

uint64_t LocalGeoPosition::getTileID(int64_t xGlob, int64_t yGlob, int64_t xMap, int64_t yMap, uint64_t tileSize, uint64_t nXTiles) {

    // Note: Flooring operation is replaced by integer divisions as we deal with non-negative indices
    return nXTiles*((int64_t) ((yGlob - yMap) / tileSize)) + 
            (int64_t) ((xGlob - xMap) / tileSize);

};

uint64_t LocalGeoPosition::tileId() {
    return _tileId;
};

// Get global coordinates of lower left corner for a tile based on ID
void LocalGeoPosition::getTileLL(uint64_t tileId, SimpleTile::Header* mapHeader, int64_t* tileOriginXBuf, int64_t* tileOriginYBuf) {

    getTileLL(tileId, mapHeader->tile_size, mapHeader->n_x_tiles, mapHeader->map_x, mapHeader->map_y, tileOriginXBuf, tileOriginYBuf);

};

// Get global coordinates of lower left corner for a tile based on ID
void LocalGeoPosition::getTileLL(uint64_t tileId, uint64_t tileSize, uint64_t nXTiles, int64_t xMap, int64_t yMap, int64_t* tileOriginXBuf, int64_t* tileOriginYBuf) {
    
    // Note: Flooring operation is replaced by integer divisions as we deal with non-negative indices
    *tileOriginXBuf = (tileId % nXTiles) * tileSize + xMap;
    *tileOriginYBuf = (tileId / nXTiles) * tileSize + yMap;

};


int16_t LocalGeoPosition::xLocal() {return _xLocal;};
int16_t LocalGeoPosition::yLocal() {return _yLocal;};