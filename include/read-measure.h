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

/**
 * @brief Read the measure from the sensor
 *
 * @return ErrorOr<LL<Measure>> list of measures.
 */
auto ReadMeasureFromSensor() -> ErrorOr<LL<Measure>>
{
    // TODO: get REAL mesuares.
    auto measures = LL<Measure>();

    measures.add({
        .value = "10",
        .idType = "1",
    });

    measures.add({
        .value = "20",
        .idType = "2",
    });

    measures.add({
        .value = "30",
        .idType = "3",
    });

    return ok(measures);
}

#endif // ! _ReadMeasure_h_