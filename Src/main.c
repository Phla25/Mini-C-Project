#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ErrorHandler.h"
#include "LoggingHandler.h"
#include "DataHandler.h"

void processIncomingData(SensorList *list, uint32_t sensor_id, uint64_t timestamp, int32_t raw_value) {
    Sensor *sensor = findSensorByID(list, sensor_id);
    if (sensor == NULL) return;

    ValidationStatus status = validateSensorData(sensor, timestamp, raw_value);
    if (status == ERR_INVALID_TIMESTAMP || status == ERR_PHYSICAL_BOUNDS) {
        sensor->stat_error_count++;
        return; 
    }

    int32_t clean_value;
    if (filterSensorData(sensor, raw_value, &clean_value)) {
        updateSensorData(sensor, timestamp, clean_value);
    } else {
        sensor->stat_error_count++;
    }
}


int main() {

    return 0;
}