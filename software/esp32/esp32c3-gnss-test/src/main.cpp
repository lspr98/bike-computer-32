#include <Arduino.h>
#include <MicroNMEA.h>

// To display free memory include the MemoryFree library, see
// https://github.com/maniacbug/MemoryFree and uncomment the line
// below
//#include <MemoryFree.h>

// Refer to serial devices by use
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
bool ledState = LOW;
volatile bool ppsTriggered = false;

HardwareSerial gps(0);

void setup(void)
{
  sleep(3);
	Serial.begin(9600); // console
  Serial.println("Hello!");
  gps.begin(115200, SERIAL_8N1, -1, -1);
  sleep(1);
  // Set baudrate
  // MicroNMEA::sendSentence(gps, "$PQBAUD,W,115200");
  // Enable GPS + GALILEO + GLONASS for Quectel L96/L86
  MicroNMEA::sendSentence(gps, "$PMTK353,1,1,1,0,0");
  sleep(1);
  // Enable Low-Power operation
  MicroNMEA::sendSentence(gps, "$PQGLP,W,1,0");
  // Enable 10Hz refresh rate
  MicroNMEA::sendSentence(gps, "$PMTK220,100");

}

void loop(void)
{
  while(gps.available()) {
    Serial.println(gps.readStringUntil('\n'));
  }
  Serial.println("Waiting for new data...");
  sleep(1);

}