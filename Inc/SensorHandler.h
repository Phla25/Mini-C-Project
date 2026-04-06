#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include "ErrorHandler.h"
#include "DataHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define FILTER_WINDOW_SIZE 5
#define MAX_ALLOWED_JUMP 10000
#define CONNECTION_TIMEOUT_MULTIPLIER 3

/* Định nghĩa loại cảm biến */
typedef enum { 
    TYPE_TEMP, 
    TYPE_HUMIDITY, 
    TYPE_LIGHT, 
    TYPE_GAS, 
    TYPE_DUST 
} DataType;

/* Giới hạn vật lý cho từng loại cảm biến */
// Tập trung định nghĩa giới hạn vào một chỗ để dễ bảo trì
#define TEMP_MIN_PHYSICAL   (-60000)  // -60.0 độ C (fixed-point x1000)
#define TEMP_MAX_PHYSICAL   ( 85000)  // +85.0 độ C
#define GAS_MIN_PHYSICAL    (0)
#define GAS_MAX_PHYSICAL    (100000)  // 100% nồng độ

typedef struct Sensor {
    uint32_t id;
    DataType type;
    int32_t longitude;
    int32_t latitude;
    int32_t min_threshold;
    int32_t max_threshold;
    uint32_t sending_cycle;
    uint32_t buffer_size;
    int32_t moving_avg_buffer[FILTER_WINDOW_SIZE]; 
    int buffer_index;              
    int samples_collected;
    int32_t stat_max;
    int32_t stat_min;
    int64_t stat_sum;           
    uint32_t stat_valid_count;
    uint32_t stat_error_count;
    uint64_t last_seen_timestamp;
    DataList history;
    struct Sensor *nextSensor;
} Sensor;

typedef struct {
    Sensor *head;
    Sensor *tail;
} SensorList;

/* Chỉ để prototype trong .h */
Sensor* createSensor(uint32_t id, DataType type, 
    int32_t longitude, int32_t latitude, int32_t min_threshold, 
    int32_t max_threshold, uint32_t sending_cycle, uint32_t buffer_size);
void addSensor(SensorList *list, Sensor *newSensor);
Sensor* findSensorByID(SensorList *list, uint32_t id);
void deleteSensor(SensorList *list, uint32_t id);
bool filterSensorData(Sensor *sensor, int32_t raw_value, int32_t *final_value);
void updateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value, SystemReport *report);
void deleteSensorData(Sensor *sensor, uint64_t timestamp, int32_t value);
int checkSensorConnections(SensorList *list, uint64_t current_timestamp);
ValidationStatus validateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value);

#endif