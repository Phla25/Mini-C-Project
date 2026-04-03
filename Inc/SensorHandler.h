#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include "ErrorHandler.h"
#include "LoggingHandler.h"
#include "DataHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Định nghĩa loại cảm biến*/
typedef enum { 
    TYPE_TEMP, 
    TYPE_HUMIDITY, 
    TYPE_LIGHT, 
    TYPE_GAS, 
    TYPE_DUST 
} DataType;
/* Kết thúc định nghĩa loại cảm biến*/

/* Khai báo có tên (struct Sensor) để dùng được self-referential con trỏ nextSensor */
typedef struct Sensor {
    uint32_t id;
    DataType type;
    int32_t longitude;
    int32_t latitude;
    int32_t min_threshold;
    int32_t max_threshold;
    uint32_t sending_cycle; // chu kỳ gửi dữ liệu (ms)
    uint32_t buffer_size;   // kích thước bộ đệm dữ liệu riêng của sensor này
    DataList history;
    struct Sensor *nextSensor;
} Sensor;

#endif