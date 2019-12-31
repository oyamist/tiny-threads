#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "src/tinycircuits/VL53L0X.h"    // Time-of-Flight Distance sensor
#include "src/omilli/Thread.h"
#include <Wireling.h>   // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "OLED042Thread.h"
#include "LEDThread.h"
#include "LraThread.h"
#include "RangeThread.h"

using namespace om;

RangeThread rangeThread;
VL53L0X distanceSensor; 

RangeThread::RangeThread() {}

void RangeThread::setup(uint8_t port, uint16_t msLoop) {
    delay(200);              // Sensor Startup time
    id = 'R';
    Thread::setup();
    this->port = port;
    this->msLoop = msLoop;
    om::print("RangeThread.setup");

    om::setI2CPort(port); 
    distanceSensor.init();
    distanceSensor.setTimeout(500);
    distanceSensor.setMeasurementTimingBudget((msLoop-1)*1000);
    distanceSensor.startContinuous(); // 19mA

    for (int i = 0; i < HEADING_COUNT; i++) {
        stepHeadings[i] = 0;
    }

    mode = MODE_IDLE;
}
#define SLOWFLASH 10
#define FASTFLASH 5

void RangeThread::sweepForward(uint16_t dist) {
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;

    uint32_t now = om::ticks();
    uint32_t cycleTicks = now - px->lastState;
    CRGB curLed = ledThread.leds[0];
    uint8_t blue = 0;
    uint16_t brightness = 255;
    if (dist < 250) { // Very close
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0xff,0,blue);
        lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
    } else if (dist < 400) {
        curLed = CRGB(0xff,0,blue);                  
        if (loops % 2 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 650) { // Somewhat close
        curLed = CRGB(0xcc,0x33,blue); 
        if (loops % 3 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 1000) {
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0,0xaa,blue);
        if (loops % 5 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    } else if (dist < 2000) {
        curLed = CRGB(0,0xaa,blue);
        if (loops % 8 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
        }
    }
    ledThread.brightness = brightness;
    if (curLed.r != ledThread.leds[0].r ||
        curLed.g != ledThread.leds[0].g ||
        curLed.b != ledThread.leds[0].b) 
    {
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }
}

#define STEP_DIFF 80
#define STEP_T 0.5

void RangeThread::sweepStep(uint16_t dist) {
    uint32_t now = om::ticks();
    uint32_t cycleTicks = now - px->lastState;
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;

    int8_t iHdg = 2;
    switch (py->heading) {
    case HEADING_LFT:     iHdg = 0; break;
    case HEADING_CTR_LFT: iHdg = 1; break;
    case HEADING_IDLE:    iHdg = 2; break;
    case HEADING_CTR_RHT: iHdg = 3; break;
    case HEADING_RHT:     iHdg = 4; break;
    }
    stepHeadings[iHdg] = dist*STEP_T + (1-STEP_T)*stepHeadings[iHdg];

    double y = absval(py->valSlow);
    double z = absval(pz->valSlow);
    double a = atan(z/y); // ~= atan(z/y) - da/2
    double w = 180; // distance from sensor to wrist pivot
    double r1 = stepHeadings[4] + w;    // h/cos(a-da) 
    double r2 = stepHeadings[3] + w;    // h/cos(a)
    double r3 = stepHeadings[1] + w;    // h/cos(a+da)
    double r4 = stepHeadings[0] + w;    // h/cos(a+2*da)
    double da = a - acos( cos(a) * r2/r1 );
    double h = (r1*cos(a-da) + r2*cos(a))/2;
    double er3 = h / cos(a+da);
    double er4 = h / cos(a+2*da);

    CRGB curLed = ledThread.leds[0];
    int32_t diffDist = 0;
    switch (py->heading) {
    case HEADING_LFT:     
        diffDist = r4 - er4;
        break;
    case HEADING_CTR_LFT: 
        diffDist = r3 - er3;
        break;
    case HEADING_CTR_RHT: 
        break;
    case HEADING_RHT:     
        curLed = CRGB(0,0xaa,blue);
        if (loops > stepTickLoops) {
            stepTickLoops = loops + 10;
            lraThread.setEffect(DRV2605_STRONG_CLICK_30); 
            om::print(stepHeadings[0]);
            om::print(", ");
            om::print(stepHeadings[1]);
            om::print(", ");
            om::print(stepHeadings[2]);
            om::print(", ");
            om::print(stepHeadings[3]);
            om::print(", ");
            om::print(stepHeadings[4]);
            om::println();
        }
        break;
    default: break;
    }

    uint8_t blue = 0x66;
    uint16_t brightness = 255;
    if (diffDist > STEP_DIFF) { // Very close
        brightness = (loops % FASTFLASH) < FASTFLASH/2 ? 32 : 255;
        curLed = CRGB(0xff,0,blue);
        lraThread.setEffect(DRV2605_TRANSITION_HUM_1); 
    } else if (diffDist < -STEP_DIFF) {
        brightness = (loops % SLOWFLASH) < SLOWFLASH/2 ? 32 : 255;
        curLed = CRGB(0xff,0xcc,blue);
        lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
    }
    ledThread.brightness = brightness;
    if (curLed.r != ledThread.leds[0].r ||
        curLed.g != ledThread.leds[0].g ||
        curLed.b != ledThread.leds[0].b) 
    {
        ledThread.leds[0] = curLed;
        ledThread.show(SHOWLED_FADE85);
    }
}

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;
    if (px->heading==HEADING_IDLE && 
        py->heading==HEADING_IDLE && 
        pz->heading==HEADING_IDLE) {
        ledThread.leds[0] = CRGB(0,0,0); // no contact
        if (mode != MODE_IDLE) { 
            om::println("MODE_IDLE standing by...");
            monitor.quiet(true);
            //distanceSensor.stopContinuous(); // 0.006mA
        }
        mode = MODE_IDLE;
    } else {
        if (mode == MODE_IDLE) {
            om::println("Motion detected, active...");
            monitor.quiet(false);
            //distanceSensor.startContinuous(); // 19mA
        }
        mode = absval(py->valSlow) < 0.5*absval(pz->valSlow) 
            ? MODE_SWEEP_FORWARD  // ranging forward above 60 degrees
            : MODE_SWEEP_STEP;    // ranging down below 60 degrees
    }
    if (mode == MODE_IDLE) {
        return;
    }

    uint16_t d = distanceSensor.readRangeContinuousMillimeters();
    distFast = d * DIST_FAST + (1-DIST_FAST) * distFast;
    distSlow = d * DIST_SLOW + (1-DIST_SLOW) * distSlow;
    uint16_t dist = distFast;
    if (dist < minRange || maxRange < dist) {
        return;
    }

    switch (mode) {
        default:
        case MODE_SWEEP_FORWARD:
            sweepForward(dist);
            break;
        case MODE_SWEEP_STEP:
            sweepStep(dist);
            break;
    }

    lastDist = dist;

    // Update OLED position display
    strcpy(oledThread.lines[1], "");
    px->headingToString(oledThread.lines[2]);
    py->headingToString(oledThread.lines[3]);
    pz->headingToString(oledThread.lines[4]);
}
