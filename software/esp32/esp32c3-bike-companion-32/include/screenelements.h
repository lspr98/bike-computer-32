#ifndef _SCREENELEMENTS_H
#define _SCREENELEMENTS_H

#include <Arduino.h>
#include <sharedspidisplay.h>

/*

    Defition of very basic UI-elements

*/

/*

    Parent class for all UI-elements

*/
class ScreenElement {

public:
    ScreenElement(uint16_t x, uint16_t y) : x(x), y(y) {};
    uint16_t x, y;
    virtual void draw(SharedSPIDisplay* display) = 0;
    virtual void step() = 0;
};


#define SCREENTEXT_MAX_LENGTH 64

/*

    Text UI-element

*/
class ScreenText : public ScreenElement {

private:
    uint8_t _textSize;
    char _text[SCREENTEXT_MAX_LENGTH];

public:
    ScreenText(uint16_t x, uint16_t y, uint8_t textSize, char* text) : ScreenElement(x, y), 
        _textSize(textSize) {
        setText(text);
    };

    ScreenText(uint16_t x, uint16_t y, uint8_t textSize, const char* text) : ScreenElement(x, y), 
        _textSize(textSize) {
        setText(text);

    };

    void draw(SharedSPIDisplay* display) {
        display->setCursor(x, y);
        display->setTextSize(_textSize);
        display->write(_text, 1);
    };

    void setText(char* text) {
        uint16_t newTextLength = strlen(text);
        if(newTextLength > SCREENTEXT_MAX_LENGTH) {
            strncpy(_text, text, SCREENTEXT_MAX_LENGTH);
        } else {
            strcpy(_text, text);
        }
    }

    void setText(const char* text) {
        uint16_t newTextLength = strlen(text);
        if(newTextLength > SCREENTEXT_MAX_LENGTH) {
            strncpy(_text, text, SCREENTEXT_MAX_LENGTH);
        } else {
            strcpy(_text, text);
        }
    }

    void step() {

    };
};


/*

    Spinner UI-element

*/
class ScreenCircleSpinner : public ScreenElement {

private:
    uint16_t _tick, _halfSize;
    bool _color, _isStatic, _centerVisible;
    uint8_t _thickness, _speed;
    ScreenElement* _centerElement;

public:
    ScreenCircleSpinner(uint16_t x, uint16_t y, uint16_t size, ScreenElement* centerElement, uint8_t thickness=2, uint8_t speed=1, u16_t tickoffset=0) : ScreenElement(x, y), 
        _halfSize(size/2), _tick(tickoffset), _color(BLACK), _centerElement(centerElement), _isStatic(false), _centerVisible(false), _thickness(thickness), _speed(speed) {};

    void draw(SharedSPIDisplay* display) {
        if(!_isStatic) {
            int16_t xc = x + _halfSize * (1.0 + sin(_tick*DEG_TO_RAD));
            int16_t yc = y + _halfSize * (1.0 + cos(_tick*DEG_TO_RAD));
            display->fillCircle(xc, yc, _thickness, _color);
        }
        if(_centerVisible) {
            _centerElement->step();
            _centerElement->draw(display);
        }
        
        return;
    };

    void step() {
        _tick += _speed;
        _tick %= 360;
    };

    void setCenterElement(ScreenElement* centerElement) {
        _centerElement = centerElement;
    }

    void setStatic(bool isStatic) {
        _isStatic = isStatic;
    }

    void setCenterVisiblilty(bool visible) {
        _centerVisible = visible;
    }
};


/*

    Double spinner UI-element

*/
class ScreenDoubleCircleSpinner : public ScreenElement {

    private:
    uint16_t _tick, _halfSize;
    bool _color, _isStatic, _centerVisible;
    uint8_t _thickness, _speed;
    ScreenCircleSpinner _s1, _s2;

public:
    ScreenDoubleCircleSpinner(uint16_t x, uint16_t y, uint16_t size, ScreenElement* centerElement, uint8_t thickness=2, uint8_t speed=1, uint16_t offset=180) : ScreenElement(x, y), 
        _halfSize(size/2), _tick(0), _color(BLACK), _isStatic(false), _centerVisible(false), _thickness(thickness), _speed(speed),
        _s1(x, y, size, centerElement, thickness, speed),
        _s2(x, y, size, &_s1, thickness, speed, offset) {        

            _s2.setCenterVisiblilty(true);

        };

    void draw(SharedSPIDisplay* display) {
        if(!_isStatic) {
            _s2.step();
            _s2.draw(display);
        }
        
        return;
    };

    void step() {
        _tick += _speed;
        _tick %= 360;
    };

    void setCenterElement(ScreenElement* centerElement) {
        _s1.setCenterElement(centerElement);
    }

    void setStatic(bool isStatic) {
        _s1.setStatic(isStatic);
        _s2.setStatic(isStatic);
    }

    void setCenterVisiblilty(bool visible) {
        _s1.setCenterVisiblilty(visible);
    }

};

#endif