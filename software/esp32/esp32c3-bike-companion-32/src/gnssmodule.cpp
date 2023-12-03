#include <gnssmodule.h>
#include <screens.h>
#include <uirenderer.h>

GNSSModule::GNSSModule(uint8_t uartNumber)
    : gps(uartNumber), nmea(messageBuffer, sizeof(messageBuffer)), GeoPositionProvider() {};

void GNSSModule::initialize() {
    gps.begin(115200, SERIAL_8N1, -1, -1);

    while(!gps.available()) {
        UIRENDERER.step();
    }

    // Activate multi constellation
    if(GNSS_ENABLE_MULTI_CONSTELLATION) MicroNMEA::sendSentence(gps, "$PMTK353,1,1,1,0,0");
    UIRENDERER.delay(500);

    // Enable SBAS
    MicroNMEA::sendSentence(gps, "$PMTK313,1");
    UIRENDERER.delay(500);

    // Enable Low-Power operation
    if(GNSS_ENABLE_LOW_POWER) MicroNMEA::sendSentence(gps, "$PQGLP,W,1,0");
    UIRENDERER.delay(500);

    // Enable 10Hz refresh rate
    if(GNSS_ENABLE_HIGH_REFRESH) MicroNMEA::sendSentence(gps, "$PMTK220,100");
    
    BOOTSCREEN.gnssOK = true;

    _lastUpdateTime = 0;
    ready = true;
}

// Returns true if new data was read.
bool GNSSModule::readGNSS() {
    if((millis() - _lastUpdateTime) > GNSS_MIN_UPDATE_TIME_MS) {
        while(gps.available()) {
            nmea.process(gps.read());
        }
        _lastUpdateTime = millis();
        return true;
    } else {
        return false;
    }
}

bool GNSSModule::isReady() {
    return ready && nmea.isValid();
};

bool GNSSModule::getPosition(GeoPosition& geoPosition) {
    geoPosition.updatePosition(getLatitude(), getLongitude());
    return nmea.isValid();
};

uint16_t GNSSModule::getHeading() {
    // NMEA module outputs heading as "thousandths of a degree"
    int newHeading = nmea.getCourse() * 1e-3;
    // Safety checks to avoid out-of range headings
    if((newHeading %= 360) < 0) {
        return newHeading + 360;
    } else {
        return newHeading;
    }
};

uint8_t GNSSModule::getHour() {
    return nmea.getHour();
}

uint8_t GNSSModule::getMinute() {
    return nmea.getMinute();
}

uint8_t GNSSModule::getDay() {
    return nmea.getDay();
}

uint8_t GNSSModule::getMonth() {
    return nmea.getMonth();
}

float GNSSModule::getSpeed() {
    float tmp = (float) nmea.getSpeed();
    // Convert to knot
    tmp /= 1000.0;
    // Convert to kph
    tmp *= 1.852;
    return tmp;
}

uint8_t GNSSModule::getSats() {
    return nmea.getNumSatellites();
}

double GNSSModule::getLatitude() {
    return (double) nmea.getLatitude() * 1e-6;
}

double GNSSModule::getLongitude() {
    return (double) nmea.getLongitude() *1e-6;
}

bool GNSSModule::step() {
    return readGNSS();
}