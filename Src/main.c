#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../Inc/ErrorHandler.h"
#include "../Inc/DataHandler.h"
#include "../Inc/SensorHandler.h"
#include "../Inc/LoggingHandler.h"
#include "../Inc/CSVHandler.h"

void processIncomingData(SensorList *list, SystemReport *sysReport,
                         uint32_t sensor_id, uint64_t timestamp, int32_t raw_value) {

    Sensor *sensor = findSensorByID(list, sensor_id);
    if (sensor == NULL) {
        writeEventLog("ERROR", sensor_id, timestamp, "Khong tim thay ID cam bien trong he thong.");
        sysReport->total_error++;
        return;
    }

    // Cảnh báo log nếu buffer sắp bị ghi đè (việc ghi đè thực sự xảy ra trong updateSensorData)
    if (sensor->buffer_size > 0 && sensor->stat_valid_count >= sensor->buffer_size) {
        writeEventLog("WARNING", sensor_id, timestamp, "Buffer day! Ban tin cu nhat bi ghi de.");
    }

    ValidationStatus status = validateSensorData(sensor, timestamp, raw_value);
    update_stats(status, sysReport);

    if (status == ERR_INVALID_TIMESTAMP || status == ERR_PHYSICAL_BOUNDS) {
        sensor->stat_error_count++;
        writeEventLog("ERROR", sensor_id, timestamp, "Du lieu vo ly hoac sai lech thoi gian. Da bo qua.");
        return;
    }

    if (status == STATUS_WARNING_HIGH || status == STATUS_WARNING_LOW) {
        writeEventLog("WARNING", sensor_id, timestamp, "Gia tri vuot nguong canh bao an toan!");
    }

    int32_t clean_value;
    if (filterSensorData(sensor, raw_value, &clean_value)) {
        // Truyền sysReport để updateSensorData tự xử lý ghi đè và thống kê
        updateSensorData(sensor, timestamp, clean_value, sysReport);
    } else {
        sensor->stat_error_count++;
        sysReport->total_error++;
        writeEventLog("NOISE", sensor_id, timestamp, "Phat hien nhieu gai dot bien. Da loc bo.");
    }
}

int main(int argc, char *argv[]) {
    const char *sensorFile = (argc > 1) ? argv[1] : "sensors.csv";
    const char *dataFile   = (argc > 2) ? argv[2] : "data.csv";

    SensorList   mySystem = {NULL, NULL};
    SystemReport myReport = {0, 0, 0, 0, 0};

    remove("system_events.log");

    printf("=== NAP CAU HINH SENSOR ===\n");
    CSVStatus s1 = loadSensorsFromCSV(sensorFile, &mySystem);
    if (s1 != CSV_OK) {
        printf("Khong the nap sensor. Ket thuc chuong trinh.\n");
        return 1;
    }

    printf("\n=== XU LY DU LIEU ===\n");
    CSVStatus s2 = loadDataFromCSV(dataFile, &mySystem, &myReport);
    if (s2 != CSV_OK) {
        printf("Khong the doc du lieu.\n");
    }

    printf("\n=== THONG KE HE THONG ===\n");
    printf("Hop le         : %d\n", myReport.total_valid);
    printf("Loi            : %d\n", myReport.total_error);
    printf("Vuot nguong    : %d\n", myReport.total_threshold);
    printf("Buffer overflow: %d\n", myReport.buffer_overflow);
    printf("Packet dropped : %d\n", myReport.packets_dropped);

    exportFullReport("BaoCao_TongHop.txt", &mySystem);

    Sensor *cur = mySystem.head;
    while (cur != NULL) {
        Sensor *next = cur->nextSensor;
        freeDataList(&cur->history);
        free(cur);
        cur = next;
    }

    return 0;
}