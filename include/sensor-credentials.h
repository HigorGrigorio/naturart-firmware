/**
 * @file sensor-credentials.h
 * @brief Sensor credentials
 * @details This file contains declarations and functions for manipuling the credentials of the sensors.
 * @version 1.0
 * @date 2021-04-15
 *
 */

#ifndef _SensorCredentials_h_
#define _SensorCredentials_h_

#include <file.h>

/**
 * @brief The credential of a sensor
 */
struct SensorType
{
    String type = "";
    String id = "";
};

/**
 * @brief The group of credentials of a sensor
 */
using SensorCredentials = LL<struct SensorType>;

#endif // ! _SensorCredentials_h_