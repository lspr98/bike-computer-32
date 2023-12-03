#ifndef _INTERPPOSITIONPROVIDER_H
#define _INTERPPOSITIONPROVIDER_H

#include <geopositionprovider.h>
#include <gnssmodule.h>

/*

    Wrapper around a GeoPositionProvider module to provide a smooth motion. Causes a delayed position.

*/

class InterpPositionProvider : public GeoPositionProvider {

private:
    GeoPositionProvider* _posProvider;
    uint32_t _nInterpolation, _cnt;
    GeoPosition _lastPos, _currPos, _nextPos;
    uint16_t _lastHeading, _currHeading, _nextHeading;
    unsigned long _tLastUpdate;
    bool _initTrigger;

public:

    InterpPositionProvider(GeoPositionProvider* posProvider, uint32_t nInterpolation);

    bool step();
    bool isReady();
    bool getPosition(GeoPosition& geoPosition);
    uint16_t getHeading();
    double getLatitude();
    double getLongitude();

};


#endif