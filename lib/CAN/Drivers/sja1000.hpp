#pragma once

#if !defined(ARDUINO_ARCH_ESP8266) && !defined(PIO_UNIT_TESTING)
#include <driver/twai.h>
#include "../driver.hpp"

class DriverSJA1000 : public CANDriver
{
private:
  twai_general_config_t g_config;
  twai_filter_config_t f_config;
  twai_timing_config_t t_config;
  bool driverIsRunning;

  bool setMode(CanDriverMode mode);
  bool getRate(const uint16_t speed, twai_timing_config_t &t_config);
  bool stopInterface();

public:
  DriverSJA1000(const CAN_Config* CANConfig, IDebugStream* const debugStream);
  bool initInterface();
  void writeLoopbackTest();
  void sendCommand(CANCommand* cmd, bool setValue = false, int value = 0);
  void handleLoop();
  ~DriverSJA1000();
};

#endif