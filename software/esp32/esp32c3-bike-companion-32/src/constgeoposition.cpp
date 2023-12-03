#include <constgeoposition.h>

ConstGeoPosition::ConstGeoPosition(GeoPosition& geoPosition, uint16_t heading) : _constPosition(geoPosition), _heading(heading), GeoPositionProvider() {
    ready = true;
};

bool ConstGeoPosition::isReady() {
    return true;
}

bool ConstGeoPosition::getPosition(GeoPosition& geoPosition) {
    geoPosition.updatePosition(_constPosition.lat(), _constPosition.lon());
    return true;
};

uint16_t ConstGeoPosition::getHeading() {
    return _heading;
};

double ConstGeoPosition::getLatitude() {
    return _constPosition.lat();
};

double ConstGeoPosition::getLongitude() {
    return _constPosition.lon();
}

void ConstGeoPosition::changePosition(GeoPosition& geoPosition) {
    _constPosition.updatePosition(geoPosition.lat(), geoPosition.lon());
};

void ConstGeoPosition::changeHeading(uint16_t newHeading) {
    _heading = newHeading;
};

bool ConstGeoPosition::step() {
    return true;
}


