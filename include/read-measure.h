/**
 * @file read-measure.h
 * @brief Read the measure from the sensor.
 * @details Read the measure from the sensor.
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-23-02
 *
 */

#ifndef _ReadMeasure_h_
#define _ReadMeasure_h_

#include <measure.h>
#include <file.h>
#include <SoftwareSerial.h>

#define RE 6
#define DE 7

const byte TEMPERATURE[] = {0x01, 0x03, 0x02, 0x00, 0x00, 0x01, 0x85, 0xc0};
const byte WATER[] = {0x01, 0x03, 0x02, 0x01, 0x00, 0x01, 0x44, 0x01};
const byte PH[] = {0x01, 0x03, 0x02, 0x03, 0x00, 0x01, 0x04, 0x41};
const byte NITROGEN[] = {0x01, 0x03, 0x02, 0x04, 0x00, 0x01, 0x45, 0x01};
const byte PHOSPHORUS[] = {0x01, 0x03, 0x02, 0x05, 0x00, 0x01, 0x85, 0xc0};
const byte POTASSIUM[] = {0x01, 0x03, 0x02, 0x06, 0x00, 0x01, 0x44, 0x01};

byte values[11];

SoftwareSerial mod(2, 3);

byte nitrogen()
{
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (mod.write(NITROGEN, sizeof(NITROGEN)) == 8)
    {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++)
        {
            // Serial.print(mod.read(),HEX);
            values[i] = mod.read();
            Serial.print(values[i], HEX);
        }
        Serial.println();
    }
    return values[4];
}

/**
 * @brief Read the measure from the sensor
 *
 * @return ErrorOr<LL<Measure>> list of measures.
 */
auto ReadMeasureFromSensor() -> ErrorOr<LL<Measure>>
{
    // TODO: get REAL mesuares.
    auto measures = LL<Measure>();

    return ok(measures);
}

#endif // ! _ReadMeasure_h_