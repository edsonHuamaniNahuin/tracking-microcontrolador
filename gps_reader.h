/*
 * ============================================================
 * gps_reader.h — Lectura y parsing de datos GPS
 * ============================================================
 * Patrón: Adapter
 * Encapsula TinyGPS++ y expone una interfaz limpia
 * con los datos que necesita el sistema.
 */

#ifndef GPS_READER_H
#define GPS_READER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include "config.h"

struct GpsData
{
  double latitude;
  double longitude;
  double speed;    // km/h
  double altitude; // metros
  double course;   // grados 0-360
  int satellites;
  double hdop; // precisión horizontal
  bool isValid;
  uint32_t age; // ms desde última lectura válida
};

class GpsReader
{
public:
  void begin()
  {
    _serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.printf("[GPS] UART2 iniciado (RX=%d, TX=%d, %d baud)\n",
                  GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD);
  }

  /** Llamar en cada iteración del loop para alimentar el parser */
  void update()
  {
    while (_serial.available() > 0)
    {
      _gps.encode(_serial.read());
    }
  }

  /** Retorna los datos GPS actuales */
  GpsData read()
  {
    GpsData data;
    data.latitude = _gps.location.lat();
    data.longitude = _gps.location.lng();
    data.speed = _gps.speed.kmph();
    data.altitude = _gps.altitude.meters();
    data.course = _gps.course.deg();
    data.satellites = _gps.satellites.value();
    data.hdop = _gps.hdop.hdop();
    data.isValid = _gps.location.isValid() &&
                   _gps.satellites.value() >= GPS_MIN_SATELLITES;
    data.age = _gps.location.age();
    return data;
  }

  /** Caracteres procesados (para debug) */
  unsigned long charsProcessed()
  {
    return _gps.charsProcessed();
  }

  /** ¿Hay datos frescos desde la última consulta? */
  bool isUpdated()
  {
    return _gps.location.isUpdated();
  }

private:
  TinyGPSPlus _gps;
  HardwareSerial _serial{2};
};

#endif
