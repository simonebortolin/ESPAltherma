#ifndef MQTT_CALLBACKS_H
#define MQTT_CALLBACKS_H

#include "mqtt.hpp"
#include "restart.hpp"
#include "Config/config.hpp"
#include "persistence.hpp"
#include "restart.hpp"

extern std::function<void(const String &label, const char *payload, const uint32_t length)> callbackCAN;

void mqttInitCallbacks();

#endif