#include <Arduino.h>

#include <SPI.h>
#include <sharedspidisplay.h>
#include <sharedspimutex.h>
#include <sharedspidevice.h>
#include <screens.h>
#include <uirenderer.h>
#include <serialutils.h>

SharedSPIDisplay::SharedSPIDisplay(uint8_t PIN_CS) : 
    _PIN_CS(PIN_CS), disp(&SPI, PIN_CS, DISPLAY_WIDTH, DISPLAY_HEIGHT, SPI_FREQ), SharedSPIDevice() {

    // Setup chip selector pin
    pinMode(PIN_CS, OUTPUT);
    // Default to disabled (for the sharp display, this means LOW for whatever reason)
    disableCS();

    SMUTEX.registerDevice(this);

};

void SharedSPIDisplay::enableCS() {
    // Serial.print("Device with ID=");
    // Serial.print(device_id);
    // Serial.println(" enabling CS");
    digitalWrite(_PIN_CS, HIGH);
    _hasSPI = true;
};

void SharedSPIDisplay::disableCS() {
    // Serial.print("Device with ID=");
    // Serial.print(device_id);
    // Serial.println(" disabling CS");
    digitalWrite(_PIN_CS, LOW);
    _hasSPI = false;
};

void SharedSPIDisplay::initialize() {
    SMUTEX.aquireSPI(this);
    while(!disp.begin()) {
        delay(100);
    }
    sout <= "Found display.";
    // Update Bootscreen status
    BOOTSCREEN.displayOK = true;
    // Set display and screen for UI renderer
    UIRENDERER.setDisplay(this);
    UIRENDERER.setScreen(&BOOTSCREEN);
}

void SharedSPIDisplay::newPage() {
    SMUTEX.aquireSPI(this);
    clearDisplay();
    disp.setTextSize(1);
    disp.setTextColor(BLACK);
    disp.setCursor(0, 5);
    disp.cp437(true);
}

void SharedSPIDisplay::clearDisplay() { 
    SMUTEX.aquireSPI(this);
    disp.clearDisplay();
}

// Just clear the buffer without rendering. Prevents "flashing"
void SharedSPIDisplay::clearDisplayBuffer() {
    // Does not write anything to the display so
    // we dont need SPI bus access
    disp.clearDisplayBuffer();
}

void SharedSPIDisplay::drawCenterMarker() {
    SMUTEX.aquireSPI(this);
    int16_t x0, y0, x1, y1, x2, y2;
    x0 = DISPLAY_WIDTH_HALF - POSITION_MARKER_SIZE;
    y0 = DISPLAY_WIDTH_HALF + POSITION_MARKER_SIZE;
    x1 = DISPLAY_WIDTH_HALF;
    y1 = DISPLAY_WIDTH_HALF - 2*POSITION_MARKER_SIZE;
    x2 = DISPLAY_WIDTH_HALF + POSITION_MARKER_SIZE;
    y2 = DISPLAY_WIDTH_HALF + POSITION_MARKER_SIZE;
    disp.fillTriangle(x0, y0, x1, y1, x2, y2, BLACK);
};

void SharedSPIDisplay::drawStatusBar(const char* statusStr) {
    SMUTEX.aquireSPI(this);
    disp.fillRect(0, DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_HEIGHT-DISPLAY_WIDTH, WHITE);
    draw_line(0, DISPLAY_WIDTH, DISPLAY_WIDTH, DISPLAY_WIDTH, 4, BLACK);
    disp.setTextSize(2);
    disp.setTextColor(BLACK);
    disp.setCursor(7, DISPLAY_WIDTH+6);
    disp.cp437(true);
    disp.write(statusStr);
};

void SharedSPIDisplay::refresh() {
    SMUTEX.aquireSPI(this);
    disp.refresh();
}

void SharedSPIDisplay::setCursor(int16_t x, int16_t y) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    disp.setCursor(x, y);
}

void SharedSPIDisplay::setTextSize(uint8_t s) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    disp.setTextSize(s);
}

void SharedSPIDisplay::setTextColor(uint16_t c) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    disp.setTextColor(c);
}

void SharedSPIDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    disp.drawPixel(x, y, color);
}

void SharedSPIDisplay::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    SMUTEX.aquireSPI(this);
    disp.fillCircle(x0, y0, r, color);
}

void SharedSPIDisplay::drawQuaterCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    disp.drawCircleHelper(x0, y0, r, cornername, color);
};

void SharedSPIDisplay::write(const char* str, bool hold) {
    disp.setTextColor(BLACK);
    disp.write(str);
    if(!hold) {
        SMUTEX.aquireSPI(this);
        disp.refresh();
    }
}

void SharedSPIDisplay::draw_line(int x0, int y0, int x1, int y1, uint8_t thickness, uint16_t color) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    if(dx < -dy) {
        while (1) {
        disp.drawPixel(x0, y0, color);
        for(int i=0; i<thickness-1; i++) {
            disp.drawPixel(x0+thickness_offsets[i], y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
        }
    } else {
        while (1) {
        disp.drawPixel(x0, y0, color);
        for(int i=0; i<thickness-1; i++) {
            disp.drawPixel(x0, y0+thickness_offsets[i], color);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
        }
    }

}


void SharedSPIDisplay::draw_dashedline(int x0, int y0, int x1, int y1, uint8_t thickness, uint8_t dashLength, uint16_t color) {
    // Does not write anything to the display so
    // we dont need SPI bus access
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    uint8_t cnt = 0;

    if(dx < -dy) {
        while (1) {
        ++cnt %= dashLength;
        if(!cnt) color = !color;

        disp.drawPixel(x0, y0, color);
        for(int i=0; i<thickness-1; i++) {
            disp.drawPixel(x0+thickness_offsets[i], y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
        }
    } else {
        while (1) {
        ++cnt %= dashLength;
        if(!cnt) color = !color;

        disp.drawPixel(x0, y0, color);
        for(int i=0; i<thickness-1; i++) {
            disp.drawPixel(x0, y0+thickness_offsets[i], color);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
        }
    }

}