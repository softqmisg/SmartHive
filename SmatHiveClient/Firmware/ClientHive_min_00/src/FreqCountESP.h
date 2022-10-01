#ifndef __FreqCountESP_H__
#define __FreqCountESP_H__

#include <Arduino.h>

void IRAM_ATTR onFreqIORise();
void IRAM_ATTR onFreqTimer();

class _FreqCountESP
{
private:
  uint8_t mPin;
  uint16_t mTimerMs;
  hw_timer_t *mTimer;

public:
  static volatile uint8_t sIsFrequencyReady;
  static volatile uint32_t sCount;
  static volatile uint32_t sFrequency;

  static portMUX_TYPE sMux;

  _FreqCountESP();
  ~_FreqCountESP();

  void begin(uint8_t pin, uint16_t timerMs, uint8_t hwTimerId = 0, uint8_t mode = INPUT);
  uint32_t read();
  uint8_t available();
  void start();
  void stop();
};

extern _FreqCountESP FreqCountESP;

#endif // kapraran_FreqCountESP_h