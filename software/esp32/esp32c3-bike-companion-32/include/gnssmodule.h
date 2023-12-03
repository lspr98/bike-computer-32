#ifndef _GNSSMODULE_H
#define _GNSSMODULE_H

#include <Arduino.h>
#include <MicroNMEA.h>
#include <geopositionprovider.h>
#include <globalconfig.h>

/*
    IMPORTANT:  Baudrate for UART of GNSS needs to be adjusted to manually!
                Quectel L86 has a default baud of 9600, which is not enough for 10Hz refresh
*/

/*

    Wrapper class for the GNSS module as a position provider.

*/
class GNSSModule : public GeoPositionProvider {

public:

    MicroNMEA nmea;

    GNSSModule(uint8_t uartNumber);
    bool readGNSS();
    void initialize();

    bool step();
    bool isReady();
    bool getPosition(GeoPosition& geoPosition);
    uint16_t getHeading();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getDay();
    uint8_t getMonth();
    float getSpeed();
    uint8_t getSats();
    double getLatitude();
    double getLongitude();

private:

    HardwareSerial gps;
    char messageBuffer [128];
    long _lastUpdateTime;


};

#endif