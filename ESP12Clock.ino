/*
 Name:		ESP12Clock.ino
 Created:	10.10.2020 16:52:00
 Author:	Alexey Bogdan aka Sr.FatCat
*/
//#define _DEBUG 1

#define PROJECT "ESP12Clock"
#define VERSION "1.1"

#define NTP_SERVER "192.168.0.60"
#include <MyArduino.h>
#include <MyWiFi.h>
String deviceID = "ESP";

#include <TM1637Display.h>
#define CLK 5
#define DIO 4
TM1637Display display(CLK, DIO);
uint8_t dataClock[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t textOtA[] = {   0, 0x3f, SEG_E | SEG_D | SEG_F | SEG_G, 0x7f & ~SEG_D };

#include <UniversalKeyboardDriver.h>

enum EKeyCodes { KEY_UP = 13, KEY_DOWN = 2, KEY_SET = 0};
const int16_t keyCodeArray_d[] = { KEY_UP,  KEY_DOWN, KEY_SET };
CKeyboardDriver keyDriver(1);

class CBrigtness {
    const uint8_t brightnessVal[3] = {0, 2, 7};
    const uint8_t maxVal = ARRAY_SIZE(brightnessVal)-1;
    uint8_t curVal;
public:
    enum EBrigtness {MIN = 0, MID = 1, MAX = ARRAY_SIZE(brightnessVal)-1};
    CBrigtness(EBrigtness val = EBrigtness::MAX) {curVal = val;}
    EBrigtness get() {return static_cast<EBrigtness>(curVal);}
    uint8_t operator++() {if (curVal<maxVal) curVal++; return brightnessVal[curVal]; }
    uint8_t operator--() {if (curVal>0) curVal--; return brightnessVal[curVal]; }
    uint8_t operator[](const EBrigtness idx){return brightnessVal[static_cast<uint8_t>(idx)]; }
    CBrigtness& operator= (const EBrigtness idx) {curVal = idx; return *this; }
    operator uint8_t() { return brightnessVal[curVal]; }
} brightness;

class CSenseStep {
    uint8_t _hit = 0;
    const uint8_t _hitMax;
    uint16_t _midValMin = 990;
    uint16_t _midValMax = 1023;
    CBrigtness::EBrigtness _curBrtns;  
    CBrigtness::EBrigtness _newBrtns;  
    uint32_t _delayMs;
    CBrigtness::EBrigtness getVal(uint16_t );
    CBrigtness& _brightness;
public:
    CSenseStep(CBrigtness& brightness, const uint32_t delayMs, const uint8_t hitMax = 12) : _brightness(brightness), _hitMax(hitMax) { _delayMs = delayMs;  _newBrtns = _curBrtns = _brightness.get(); }
    void run(TM1637Display&);
} lightSense(brightness, 500);

CBrigtness::EBrigtness CSenseStep::getVal(uint16_t val){
    if (val >= _midValMax) return CBrigtness::EBrigtness::MIN;
    if (val <= _midValMin) return CBrigtness::EBrigtness::MAX;
    return CBrigtness::EBrigtness::MID;
}

void CSenseStep::run(TM1637Display& display){
    DEF_TMENEGMENT;
    IF_TMENEGMENT(_delayMs){
        uint16_t lightSens = analogRead(A0);

        if ( getVal(lightSens) != _curBrtns ){
            if (getVal(lightSens) == _newBrtns){
                if (++_hit == _hitMax){
                    _hit = 0;
                    _brightness = _curBrtns = _newBrtns;                   
                    display.setBrightness(_brightness);
                    DEBUG_PRINT("new brightness = %i\n", static_cast<uint8_t>(brightness));
                }
            }
            else {
                _hit = 0;
                _newBrtns = getVal(lightSens);
            }
        }
        PASS_TMENEGMENT;
    }
}

#include "TM1637Effects.h"

void setup() {
    Serial.begin(115200);
    DEBUG_PRINT("\n\n******************** %s %s ********************\n\n", PROJECT, VERSION);
    keyDriver.addDevice((CKeybdDevice*)new CKeybdDigital((int16_t*)keyCodeArray_d, ARRAY_SIZE(keyCodeArray_d)));
    setupWiFi();  
    display.setBrightness(brightness);
    fillSegments((const uint8_t[4]){SEG_G|SEG_DP, SEG_G, SEG_G, SEG_G});   
    display.setSegments(dataClock);     
    upWiFi();
}

void loop() {
    static bool isRefreshDisplay = true;
    static uint32_t timeToInvertDelimit = 0;
    static bool isDelimit = false;
    static bool isNoWiFi = true;
 
    const bool isNewTime = !IS_REAL_TIME(now());

    if (WiFi.status() == WL_CONNECTED) {
        syncTime();
        if (isNoWiFi) showNoWiFi(isNoWiFi = false);
    }
    else {
        if (!isNoWiFi) showNoWiFi(isNoWiFi = true);
    }

    if (IS_REAL_TIME(now())){
        const uint32_t t = millis();
        if (t > timeToInvertDelimit){            
            isDelimit = !isDelimit;
            timeToInvertDelimit = t + 1000;
            showDelimit(isDelimit);
        }
        if (isNewTime || (isRefreshDisplay && second() == 0)){
            showClockData(isDelimit, isNoWiFi);
            if (WiFi.status() != WL_CONNECTED) upWiFi();
            isRefreshDisplay = false;
        }
        if (second()==59) isRefreshDisplay = true;
    }
    else if (WiFi.status() != WL_CONNECTED) upWiFi();

    //lightSense.run(display);

    keyDriver.listenKeys(); 
	if (keyDriver.isKeyPressed()) {
		bool lpress;
		int currKey = keyDriver.getKeyCode(&lpress);
		DEBUG_PRINT("KeyCode = %i LP=%i\n", currKey, lpress);
        if (currKey == KEY_UP || currKey == KEY_DOWN) {
            display.setBrightness( currKey == KEY_UP ? ++brightness : --brightness );
            DEBUG_PRINT("......brightness = %i\n", static_cast<uint8_t>(brightness));
        } 
        else if (currKey == KEY_SET){
            DEBUG_PRINT("_________SHOW_________\n");
            if (lpress){
                display.setSegments(textOtA);
                delay(1500);
                runOTA("http://newsam.h1n.ru/esp8266/ESP12Clock.ino.bin");
                // int res;
                // if (HTTP_UPDATE_OK != (res = runOTA("http://newsam.h1n.ru/esp8266/ESP12Clock.ino.bin"))) {
                //     display.setSegments(textOtA);
                //     delay(1500);
                //     display.showNumberDecEx(res);
                //     delay(3000);
                // }
            }
            else {
                display.clear();
                for (int i=0; i<6; i++){
                    display.showNumberDecEx(analogRead(A0));
                    delay(500);
                }
                showClockData(isDelimit, isNoWiFi);
            }
        }
    }
}
