#ifndef UTILS_GEO_H
#define UTILS_GEO_H

#include <Arduino.h>
// TODO: make this a proper coordinate class on which you can call conversion functions

#define R_EARTH 6378137.0

// Convert lon-lat coordinates to web mercator coordinates
// lonlat points to an array of 2 floats, lonlat[0] = lon, lonlat[1] = lat
// lonlat given in degrees
// -180° < LON < 180°
// -85° < LAT < 85°
void lonlat_to_mercator(float* mercator, float* lonlat) {
    // Convert lon to x
    mercator[0] = R_EARTH * DEG_TO_RAD*lonlat[0];

    // Convert lat to y using fast implementation
    mercator[1] = R_EARTH * 
        ((((((((((-3.1112583378460085319e-23  * lonlat[1] +
                    2.0465852743943268009e-19) * lonlat[1] +
                    6.4905282018672673884e-18) * lonlat[1] +
                    -1.9685447939983315591e-14) * lonlat[1] +
                    -2.2022588158115104182e-13) * lonlat[1] +
                    5.1617537365509453239e-10) * lonlat[1] +
                    2.5380136069803016519e-9)  * lonlat[1] +
                    -5.1448323697228488745e-6)  * lonlat[1] +
                    -9.4888671473357768301e-6)  * lonlat[1] +
                    1.7453292518154191887e-2)  * lonlat[1])
        /
        ((((((((((-1.9741136066814230637e-22  * lonlat[1] +
                    -1.258514031244679556e-20)  * lonlat[1] +
                    4.8141483273572351796e-17) * lonlat[1] +
                    8.6876090870176172185e-16) * lonlat[1] +
                    -2.3298743439377541768e-12) * lonlat[1] +
                    -1.9300094785736130185e-11) * lonlat[1] +
                    4.3251609106864178231e-8)  * lonlat[1] +
                    1.7301944508516974048e-7)  * lonlat[1] +
                    -3.4554675198786337842e-4)  * lonlat[1] +
                    -5.4367203601085991108e-4)  * lonlat[1] + 1.0);
}

#endif