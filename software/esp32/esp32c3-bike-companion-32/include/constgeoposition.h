#ifndef _CONSTGEOPOSITION_H
#define _CONSTGEOPOSITION_H

#include <geopositionprovider.h>
#include <geoposition.h>
#include <Arduino.h>

/*

    Position Provider that outputs a constant heading and position.

*/

class ConstGeoPosition : public GeoPositionProvider {
private:
    GeoPosition _constPosition;
    uint16_t _heading;
public:
    ConstGeoPosition(GeoPosition& geoPosition, uint16_t heading);

    bool step();
    bool isReady();
    bool getPosition(GeoPosition& geoPosition);
    void changePosition(GeoPosition& geoPosition);
    void changeHeading(uint16_t newHeading);
    uint16_t getHeading();
    double getLatitude();
    double getLongitude();
};

#endif