#include <uirenderer.h>
#include <serialutils.h>
#include <globalconfig.h>
#include <Arduino.h>

UIRenderer::UIRenderer() :
    _currentScreen(&BOOTSCREEN), _hasGNSS(false), _hasHeader(false), _hasPositionProvider(false) {

    _leftStat = (StatusBarElement) LEFT_STAT;
    _rightStat = (StatusBarElement) RIGHT_STAT;
    _textBuffer = new char[(N_CHAR_PER_STAT + 1)*2];

    _tLastRender = millis();

}

bool UIRenderer::initializeMap(SharedSPISDCard* sd) {
    if(_hasHeader) {
        if(!_mapRenderer.initialize(_header, sd, _disp)) {
            return false;
        }
        if(_hasPositionProvider) {
            _mapRenderer.setPositionProvider(_posProvider);
        }
        return true;
    } else {
        return false;
    }
};

void UIRenderer::setDisplay(SharedSPIDisplay* display) {
    _disp = display;
    _hasDisplay = true;
}

void UIRenderer::setGNSS(GNSSModule* gnss) {
    _gnss = gnss;
    _hasGNSS = true;
}

void UIRenderer::setPositionProvider(GeoPositionProvider* positionProvider) {
    _posProvider = positionProvider;
    _hasPositionProvider = true;
    _mapRenderer.setPositionProvider(_posProvider);
}

void UIRenderer::setHeader(SimpleTile::Header* header) {
    _header = header;
    _hasHeader = true;
}

void UIRenderer::setGPXTrackIn(GPXTrack* track) {
    _track = track;
    _hasTrackIn = true;
    _mapRenderer.setGPXTrackIn(_track);
};

void UIRenderer::setScreen(Screen* newScreen) {
    _currentScreen = newScreen;
}


// Note: This function always returns true such that it can be used to keep a while loop alive.
bool UIRenderer::step() {
    if(!_hasDisplay) return true;

    _disp->clearDisplayBuffer();

    _currentScreen->render(_disp);

    if(_hasGNSS) {
        _gnss->step();
    }

    if(_currentScreen->hasMap && _hasPositionProvider) {
        _posProvider->step();
        // Try to render map. If it is not ready, render the waiting screen instead
        if(!_mapRenderer.step(1)) {
            FIXWAITINGSCREEN.setCenterText(_textBuffer);
            FIXWAITINGSCREEN.render(_disp);
        }
    }

    // Note: Statusbar must be rendered after screen!
    if(_currentScreen->hasStatusBar) {
        renderStatusBar();
    }


    _disp->refresh();

    // Check if we need to wait before next render loop to achieve desired FPS.
    _tLoopRender = millis() - _tLastRender;
    if(_tLoopRender < TARGET_FRAME_TIME_MS) {
        delayMicroseconds((TARGET_FRAME_TIME_MS - _tLoopRender)*1000);
    }
    _tLastRender = millis();

    return true;
}

void UIRenderer::renderStatusBar() {

    // Render left status
    if(_leftStat == speed || _leftStat == heading || _leftStat == lat || _leftStat == lon) {
        // Requires position provider
        if(!_hasPositionProvider) {
            renderStat(err, _textBuffer, true);
        } else {
            renderStat(_leftStat, _textBuffer, true);
        }
    } else {
        // Requires GNSS
        if(!_hasGNSS) {
            renderStat(err, _textBuffer, true);
        } else {
            renderStat(_leftStat, _textBuffer, true);
        }
    }

    // Render right status
    if(_rightStat == speed || _rightStat == heading || _rightStat == lat || _rightStat == lon) {
        // Requires position provider
        if(!_hasPositionProvider) {
            renderStat(err, _textBuffer + N_CHAR_PER_STAT);
        } else {
            renderStat(_rightStat, _textBuffer + N_CHAR_PER_STAT);
        }
    } else {
        // Requires GNSS
        if(!_hasGNSS) {
            renderStat(err, _textBuffer + N_CHAR_PER_STAT);
        } else {
            renderStat(_rightStat, _textBuffer + N_CHAR_PER_STAT);
        }
    }
    _disp->drawStatusBar(_textBuffer);
}

void UIRenderer::renderStat(StatusBarElement ele, char* textBuff, bool removeTerminator) {
    // TODO: sNprintf for every format!
    uint8_t n_print_chars;
    
    switch (ele)
    {
    case time:
        // sout <= "Printing time";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "%02i:%02i", _gnss->getHour(), _gnss->getMinute());
        break;

    case date:
        // sout <= "Printing date";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "%02i.%02i", _gnss->getDay(), _gnss->getMonth());
        break;

    case speed:
        // sout <= "Printing speed";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "%f", _gnss->getSpeed());
        // sprintf(textBuff, "%04.0f", (float) 541.653);
        break;

    case heading:
        // sout <= "Printing heading";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "HE%03i", _posProvider->getHeading());
        break;

    case lat:
        // sout <= "Printing lat";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "%2.2f", _posProvider->getLatitude());
        break;

    case lon:
        // sout <= "Printing lon";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "%2.2f", _posProvider->getLongitude());
        break;

    case nsats:
        // sout <= "Printing nsats";
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, "SAT%02i", _gnss->getSats());
        break;
    
    default:
        // Also includes StatusBarElement::err case
        n_print_chars = snprintf(textBuff, N_CHAR_PER_STAT, " ERR ");
        break;
    }

    if(removeTerminator) {
        if(n_print_chars > 0 && n_print_chars < N_CHAR_PER_STAT) {
            memset(textBuff + n_print_chars, ' ', N_CHAR_PER_STAT - n_print_chars);
            // textBuff[n_print_chars] = ' ';
        } else {
            textBuff[N_CHAR_PER_STAT-1] = ' ';
        }
        
    }
}

// Perform a delay/sleep operation while keeping UI alive.
void UIRenderer::delay(uint64_t milliseconds) {
    long t_start = millis();
    while((millis() - t_start) < milliseconds) {
        step();
    }
}

UIRenderer UIRENDERER;