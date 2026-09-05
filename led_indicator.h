/*
 * ============================================================
 * led_indicator.h — Indicador visual de estado por LED
 * ============================================================
 * Patrón: State
 * El LED comunica el estado del dispositivo al técnico
 * sin necesidad de monitor serial.
 */

#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <Arduino.h>
#include "config.h"

enum class DeviceState
{
  BOOTING,            // LED encendido fijo
  CONNECTING_NETWORK, // Parpadeo rápido (Ethernet DHCP)
  WAITING_GPS,        // Parpadeo lento
  RUNNING,            // LED encendido fijo
  ERROR_NETWORK,      // 2 parpadeos rápidos + pausa
  ERROR_API,          // 3 parpadeos rápidos + pausa
  ERROR_NO_TOKEN,     // 5 parpadeos rápidos + pausa
  CONFIG_SERVER,      // Parpadeo medio (servidor web activo)
  RESETTING           // Parpadeo muy rápido
};

class LedIndicator
{
public:
  void begin()
  {
    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, LOW);
  }

  void setState(DeviceState state)
  {
    _state = state;
  }

  /** Llamar en cada iteración del loop */
  void update()
  {
    unsigned long now = millis();

    switch (_state)
    {
    case DeviceState::BOOTING:
    case DeviceState::RUNNING:
      digitalWrite(LED_STATUS_PIN, HIGH);
      break;

    case DeviceState::CONNECTING_NETWORK:
    case DeviceState::RESETTING:
      _blink(now, LED_BLINK_FAST);
      break;

    case DeviceState::WAITING_GPS:
      _blink(now, LED_BLINK_SLOW);
      break;

    case DeviceState::CONFIG_SERVER:
      _blink(now, 500);
      break;

    case DeviceState::ERROR_NETWORK:
      _blinkPattern(now, 2);
      break;

    case DeviceState::ERROR_API:
      _blinkPattern(now, 3);
      break;

    case DeviceState::ERROR_NO_TOKEN:
      _blinkPattern(now, 5);
      break;
    }
  }

private:
  DeviceState _state = DeviceState::BOOTING;
  unsigned long _lastBlink = 0;
  bool _ledOn = false;
  int _patternCount = 0;
  bool _inPause = false;

  void _blink(unsigned long now, unsigned long interval)
  {
    if (now - _lastBlink >= interval)
    {
      _lastBlink = now;
      _ledOn = !_ledOn;
      digitalWrite(LED_STATUS_PIN, _ledOn ? HIGH : LOW);
    }
  }

  // N parpadeos rápidos + pausa larga
  void _blinkPattern(unsigned long now, int count)
  {
    if (_inPause)
    {
      if (now - _lastBlink >= 1500)
      {
        _inPause = false;
        _patternCount = 0;
      }
      return;
    }

    if (now - _lastBlink >= LED_BLINK_FAST)
    {
      _lastBlink = now;
      _ledOn = !_ledOn;
      digitalWrite(LED_STATUS_PIN, _ledOn ? HIGH : LOW);
      if (!_ledOn)
      {
        _patternCount++;
        if (_patternCount >= count)
        {
          _inPause = true;
          _lastBlink = now;
        }
      }
    }
  }
};

#endif
