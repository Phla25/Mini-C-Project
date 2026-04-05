#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include "SensorHandler.h"
#include <time.h>
#include <stdint.h>

/* Prototype các hàm xử lý logging */
void exportFullReport(const char* filename, SensorList *list);
void writeEventLog(const char* level, uint32_t sensor_id, uint64_t timestamp, const char* message);

#endif