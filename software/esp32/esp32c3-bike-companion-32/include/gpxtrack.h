#ifndef _GPXTRACK_H
#define _GPXTRACK_H

#include <Arduino.h>


/*

    GPX-Track

*/
struct GPXTrack {
public:
    int16_t* xList;
    int16_t* yList;
    uint64_t* tileIdList;
    uint32_t numNodes;
    uint32_t nearestNodeId;
};

#endif