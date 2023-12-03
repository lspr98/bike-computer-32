#ifndef _GEOPOSITIONPROVIDER_H
#define _GEOPOSITIONPROVIDER_H

#include <geoposition.h>

/*

    Abstract class for any algorithm that can provide a sequence of positions.

*/
class GeoPositionProvider {

protected:
    bool ready;

public:
    GeoPositionProvider() { ready = false; };
    
    // Step method to perform updates that need to be performed on every frame render.
    virtual bool step() = 0;
    virtual bool isReady() = 0;
    virtual bool getPosition(GeoPosition& geoPosition) = 0;
    virtual uint16_t getHeading() = 0;
    virtual double getLatitude() = 0;
    virtual double getLongitude() = 0;

};


#endif