#ifndef _TILEBLOCKRENDERER_H
#define _TILEBLOCKRENDERER_H

#include <sharedspidisplay.h>
#include <sharedspisdcard.h>
#include <geoposition.h>
#include <geopositionprovider.h>

/*

    Map renderer. Renders a block of tiles to the display buffer.

*/

class TileBlockRenderer {

private:

    bool _hasPositionProvider, _hasHeader, _hasTrackIn;
    float _zoomLevel, _zoomScale;
    int _heading;
    float* _rotMtxBuf;
    uint64_t* _renderTileIds;
    int16_t* _renderTileData;
    uint64_t* _renderTileSizes;
    uint64_t _perTileBufferSize;    
    uint64_t _centerTileId, _prevCenterTileId;
    long long _prevCenterChangeTime, _prevTileUpdateTime;
    int* _offsetDirectionMap;

    SimpleTile::Header* _header;
    SharedSPISDCard* _sd;
    SharedSPIDisplay* _display;
    GPXTrack* _track;
    GeoPositionProvider* _positionProvider;

    void updateTileBuffer(LocalGeoPosition& center);
    void render(LocalGeoPosition& center);
    void renderGPX(LocalGeoPosition& center);

    bool isOnDisplay(int disp_LL_x, int disp_LL_y, int disp_UR_x, int disp_UR_y, int16_t x0, int16_t y0);

public:
    TileBlockRenderer();

    void initialize(SimpleTile::Header* mapHeader, SharedSPISDCard* sd, SharedSPIDisplay* display);
    void setPositionProvider(GeoPositionProvider* newPositionProvider);
    void setGPXTrackIn(GPXTrack* track);
    void setZoom(float newZoomLevel);
    bool step(bool holdOn=false);

};

#endif