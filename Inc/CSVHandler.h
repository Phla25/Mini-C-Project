#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include "SensorHandler.h"

// Kết quả đọc CSV
typedef enum {
    CSV_OK = 0,
    CSV_ERR_FILE_NOT_FOUND,
    CSV_ERR_INVALID_HEADER,
    CSV_ERR_INVALID_ROW
} CSVStatus;

CSVStatus loadSensorsFromCSV(const char *filename, SensorList *list);
CSVStatus loadDataFromCSV(const char *filename, SensorList *list, SystemReport *report);

#endif