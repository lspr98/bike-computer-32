#include <interppositionprovider.h>
#include <globalconfig.h>
#include <serialutils.h>

InterpPositionProvider::InterpPositionProvider(GeoPositionProvider* posProvider, uint32_t nInterpolation) : _posProvider(posProvider), _nInterpolation(nInterpolation), _initTrigger(true) {
    // Require at least one interpolation point
    if(!_nInterpolation) _nInterpolation++;
}

bool InterpPositionProvider::step() {
    if(!isReady()) return false;

    _posProvider->getPosition(_currPos);
    
    if(_initTrigger) {
        _posProvider->getPosition(_lastPos);
        _nextPos.updatePosition(_lastPos.lat(), _lastPos.lon());
        _tLastUpdate = millis();
        _lastHeading = _posProvider->getHeading();
        _nextHeading = _lastHeading;
        _initTrigger = false;
    } else {
        // Check if we need to read a new position
        if((_nextPos.x() != _currPos.x()) || (_nextPos.y() != _currPos.y())) {
            _lastPos.updatePosition(_nextPos.lat(), _nextPos.lon());
            _lastHeading = _nextHeading;
            _initTrigger = !(_posProvider->getPosition(_nextPos));
            _nextHeading = _posProvider->getHeading();
            _tLastUpdate = millis();
            _cnt = 0;
        }
    }

    // Interpolate
    float perc = ((float) _cnt) / ((float) _nInterpolation);

    uint64_t x_new = _lastPos.x() + _nextPos.x() * perc - _lastPos.x() * perc;
    uint64_t y_new = _lastPos.y() + _nextPos.y() * perc - _lastPos.y() * perc;
    
    _currHeading = _lastHeading + (_nextHeading - _lastHeading) * perc;
    _currPos.updatePosition(x_new, y_new);
    
    _cnt++;

    return true;
}


bool InterpPositionProvider::isReady() {
    return _posProvider->isReady();
}


bool InterpPositionProvider::getPosition(GeoPosition& geoPosition) {
    // Return interpolated position
    geoPosition.updatePosition(_currPos.x(), _currPos.y());
    return true;
}


uint16_t InterpPositionProvider::getHeading() {
    // Return interpolated heading
    return _currHeading;
}


double InterpPositionProvider::getLatitude() {
    return _currPos.lat();
}


double InterpPositionProvider::getLongitude() {
    return _currPos.lon();
}