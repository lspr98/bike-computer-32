#ifndef _SHAREDSPIDEVICE_H
#define _SHAREDSPIDEVICE_H

#include <globalconfig.h>

/*

    Parent class for devices accessed via a shared SPI-bus.

*/

class SharedSPIDevice {

protected:
    bool _hasSPI;

public:
    int device_id;
    
    SharedSPIDevice() {
        _hasSPI = false;
        device_id = _glob_device_id++;
    };
    
    virtual void enableCS() = 0;
    
    virtual void disableCS() = 0;

    bool hasSpi() {
        return _hasSPI;
    };

private:

    static int _glob_device_id;

};

#endif
