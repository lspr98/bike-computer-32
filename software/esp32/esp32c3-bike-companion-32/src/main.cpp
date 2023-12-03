#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <MicroNMEA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#include <gnssmodule.h>
#include <sharedspisdcard.h>
#include <sharedspidisplay.h>
#include <sharedspimutex.h>
#include <simpletile.h>
#include <constgeoposition.h>
#include <tileblockrenderer.h>
#include <uirenderer.h>
#include <serialutils.h>
#include <globalconfig.h>
#include <interppositionprovider.h>

/*

    IMPORTANT:  There is a bug in sd_diskio that causes a "Check status failed" if the SDCard-Reader
                and other SPI devices are hooked up to the same SPI bus. See the following github
                issue on how to fix this: https://github.com/espressif/arduino-esp32/issues/8534

                If you do not adapt the ff_sd_status method in sd_diskio accordingly, you will get an error.

*/


// Header of bin map file
SimpleTile::Header header;

uint16_t currHeading = 0;

// Fixed position provider for debugging
GeoPosition currPos(48.136223, 11.594579);
ConstGeoPosition mockPosProvider(currPos, currHeading);

SharedSPIDisplay display(DISPLAY_CS);
SharedSPISDCard sdcard(SDCARD_CS);
GNSSModule gnss(0);
GPXTrack track;

InterpPositionProvider ipos(&gnss, ((float) GNSS_MIN_UPDATE_TIME_MS) / ((float) TARGET_FRAME_TIME_MS));

// Path to map-file on SD-card
const char binary_path[] = "/map.bin";
// Path to gpx-track file on SD-card
const char gpx_path[] = "/track.gpx";

void setup() {
  sleep(2);
  SPI.setFrequency(SPI_FREQ);
  SPI.begin();
  Serial.begin(9600);

  sout <= "Bike-Companion-32";
  sout << "Build-date: " <= __DATE__;
  
  display.initialize();
  gnss.initialize();
  sdcard.initialize();
  sdcard.setMapPath(binary_path);
  sdcard.setGPXTrackInPath(gpx_path);
  while(!sdcard.readHeader(header)) {
    UIRENDERER.delay(100);
  }
  BOOTSCREEN.mapOK = true;
  UIRENDERER.step();
  if(sdcard.readGPX(header, track)) {
    UIRENDERER.setGPXTrackIn(&track);
    BOOTSCREEN.trackOK = true;
  } else {
    BOOTSCREEN.trackOK = -1;
    UIRENDERER.delay(500);
  }
  UIRENDERER.setHeader(&header);
  UIRENDERER.setGNSS(&gnss);
  UIRENDERER.initializeMap(&sdcard);
  UIRENDERER.setPositionProvider(&ipos);
  
  // Setup was successful. Render some more frames of the bootscreen to show it.
  UIRENDERER.delay(500);

  // Change to map
  UIRENDERER.setScreen(&MAPSCREEN);
}

void loop() {
  // TODO: Handle sudden SD-card removing.

  // double offset1 = 1e-5;
  // double offset2 = 0;
  // currPos.updatePosition(currPos.lat() + offset1, currPos.lon() + offset2);
  // currHeading += 1;
  // currHeading %= 360;
  // mockPosProvider.changePosition(currPos);
  // mockPosProvider.changeHeading(currHeading);

  UIRENDERER.step();

}