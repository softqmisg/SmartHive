#include "FreqCountESP.h"

volatile uint8_t _FreqCountESP::sIsFrequencyReady = false;
volatile uint32_t _FreqCountESP::sCount = 0;
volatile uint32_t _FreqCountESP::sFrequency = 0;

portMUX_TYPE _FreqCountESP::sMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onFreqIORise()
{
  portENTER_CRITICAL_ISR(&_FreqCountESP::sMux);
  _FreqCountESP::sCount++;
  portEXIT_CRITICAL_ISR(&_FreqCountESP::sMux);
}

void IRAM_ATTR onFreqTimer()
{
  portENTER_CRITICAL_ISR(&_FreqCountESP::sMux);
  _FreqCountESP::sFrequency = _FreqCountESP::sCount;
  _FreqCountESP::sCount = 0;
  _FreqCountESP::sIsFrequencyReady = true;
  portEXIT_CRITICAL_ISR(&_FreqCountESP::sMux);
}

_FreqCountESP::_FreqCountESP()
{
  mTimer = NULL;
}

_FreqCountESP::~_FreqCountESP()
{
  stop();
}

void _FreqCountESP::begin(uint8_t pin, uint16_t timerMs, uint8_t hwTimerId, uint8_t mode)
{
  mPin = pin;
  mTimerMs = timerMs;
  sIsFrequencyReady = false;
  sCount = 0;
  sFrequency = 0;

  pinMode(mPin, mode);

//  attachInterrupt(mPin, &onFreqIORise, RISING);

  mTimer = timerBegin(hwTimerId, 80, true);
  timerAlarmWrite(mTimer, mTimerMs * 1000, true);
  //timerAttachInterrupt(mTimer, &onFreqTimer, true);
  //timerAlarmEnable(mTimer);

}
void _FreqCountESP::start()
{
  attachInterrupt(mPin, &onFreqIORise, RISING);
  timerAttachInterrupt(mTimer, &onFreqTimer, true);
  timerStart(mTimer);
  timerAlarmEnable(mTimer);
}
void _FreqCountESP::stop()
{
  detachInterrupt(mPin);

  timerAlarmDisable(mTimer);
  timerDetachInterrupt(mTimer);
  timerEnd(mTimer);
}
uint32_t _FreqCountESP::read()
{
  sIsFrequencyReady = false;
  return sFrequency;
}

uint8_t _FreqCountESP::available()
{
  return sIsFrequencyReady;
}



_FreqCountESP FreqCountESP;