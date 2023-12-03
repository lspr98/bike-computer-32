#ifndef _SHAREDSPIMUTEX_H
#define _SHAREDSPIMUTEX_H

#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <sharedspidevice.h>
/*

    Mutex-like lock to prevent multiple active SPI devices on same bus.
    Ensures that only one chip select is active at a time.

*/

class SharedSPIMutex {

private:

    std::vector<SharedSPIDevice*> spi_devices;

public:
    
    void registerDevice(SharedSPIDevice *newDev) {
        spi_devices.push_back(newDev);
    };

    void aquireSPI(SharedSPIDevice* currDev) {
        // If device that requested bus access already has access we dont need to do anything
        if((*currDev).hasSpi()) return;

        // Otherwise, deactivate all SPI devices
        for(auto dev : spi_devices) {
            dev->disableCS();
        }
        // Activate device that requested access
        currDev->enableCS();
    };

    int getNumberDevices() {
        return spi_devices.size();
    }
};

extern SharedSPIMutex SMUTEX;

#endif
