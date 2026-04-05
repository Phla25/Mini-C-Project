#include "../Inc/CSVHandler.h"
#include "../Inc/LoggingHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define MAX_LINE_LEN 256
#define EXPECTED_SENSOR_COLS 8
#define EXPECTED_DATA_COLS   3

/* Chuyển tên chuỗi sang enum DataType */
static int parseDataType(const char *str) {
    if (strcmp(str, "TEMP")     == 0) return TYPE_TEMP;
    if (strcmp(str, "HUMIDITY") == 0) return TYPE_HUMIDITY;
    if (strcmp(str, "LIGHT")    == 0) return TYPE_LIGHT;
    if (strcmp(str, "GAS")      == 0) return TYPE_GAS;
    if (strcmp(str, "DUST")     == 0) return TYPE_DUST;
    return -1; // Không nhận ra
}

/* Đếm số cột trong một dòng CSV */
static int countColumns(const char *line) {
    int count = 1;
    for (const char *p = line; *p; p++) {
        if (*p == ',') count++;
    }
    return count;
}

/* Trim ký tự xuống dòng (\r\n) cuối chuỗi */
static void trimNewline(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

/* ------------------------------------------------------------------ */
/*  Đọc file sensors.csv và thêm vào SensorList                        */
/*  Format: id,type,longitude,latitude,min_threshold,                  */
/*          max_threshold,sending_cycle,buffer_size                    */
/* ------------------------------------------------------------------ */
CSVStatus loadSensorsFromCSV(const char *filename, SensorList *list) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("[CSV] Loi: Khong the mo file '%s'\n", filename);
        return CSV_ERR_FILE_NOT_FOUND;
    }

    char line[MAX_LINE_LEN];
    int lineNum = 0;
    int loaded  = 0;
    int skipped = 0;

    while (fgets(line, sizeof(line), f)) {
        lineNum++;
        trimNewline(line);
        if (strlen(line) == 0) continue; // Bỏ qua dòng trống

        // Dòng 1: header — chỉ kiểm tra số cột
        if (lineNum == 1) {
            if (countColumns(line) != EXPECTED_SENSOR_COLS) {
                printf("[CSV] Loi: Header cua '%s' khong dung dinh dang (%d cot, can %d).\n",
                       filename, countColumns(line), EXPECTED_SENSOR_COLS);
                fclose(f);
                return CSV_ERR_INVALID_HEADER;
            }
            continue;
        }

        // Kiểm tra đủ cột trước khi parse
        if (countColumns(line) != EXPECTED_SENSOR_COLS) {
            printf("[CSV] Canh bao: Dong %d trong '%s' thieu cot, da bo qua.\n", lineNum, filename);
            skipped++;
            continue;
        }

        // Parse từng trường
        uint32_t id, sending_cycle, buffer_size;
        int32_t  longitude, latitude, min_threshold, max_threshold;
        char     typeStr[32];

        int parsed = sscanf(line, "%u,%31[^,],%d,%d,%d,%d,%u,%u",
                            &id, typeStr,
                            &longitude, &latitude,
                            &min_threshold, &max_threshold,
                            &sending_cycle, &buffer_size);

        if (parsed != EXPECTED_SENSOR_COLS) {
            printf("[CSV] Canh bao: Dong %d trong '%s' gia tri khong hop le, da bo qua.\n", lineNum, filename);
            skipped++;
            continue;
        }

        // Kiểm tra type hợp lệ
        int type = parseDataType(typeStr);
        if (type < 0) {
            printf("[CSV] Canh bao: Dong %d - loai cam bien '%s' khong hop le, da bo qua.\n", lineNum, typeStr);
            skipped++;
            continue;
        }

        // Kiểm tra ID trùng
        if (findSensorByID(list, id) != NULL) {
            printf("[CSV] Canh bao: Dong %d - Sensor ID %u da ton tai, da bo qua.\n", lineNum, id);
            skipped++;
            continue;
        }

        Sensor *s = createSensor(id, (DataType)type,
                                 longitude, latitude,
                                 min_threshold, max_threshold,
                                 sending_cycle, buffer_size);
        if (s != NULL) {
            addSensor(list, s);
            loaded++;
        }
    }

    fclose(f);
    printf("[CSV] '%s': Nap thanh cong %d sensor, bo qua %d dong loi.\n",
           filename, loaded, skipped);
    return CSV_OK;
}

/* ------------------------------------------------------------------ */
/*  Đọc file data.csv và xử lý từng bản tin qua processIncomingData    */
/*  Format: sensor_id,timestamp,raw_value                              */
/*  Hàm này cần processIncomingData — forward declare ở đây            */
/* ------------------------------------------------------------------ */
void processIncomingData(SensorList *list, SystemReport *report,
                         uint32_t sensor_id, uint64_t timestamp, int32_t raw_value);

CSVStatus loadDataFromCSV(const char *filename, SensorList *list, SystemReport *report) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("[CSV] Loi: Khong the mo file '%s'\n", filename);
        return CSV_ERR_FILE_NOT_FOUND;
    }

    char line[MAX_LINE_LEN];
    int lineNum = 0;
    int processed = 0;
    int skipped   = 0;

    while (fgets(line, sizeof(line), f)) {
        lineNum++;
        trimNewline(line);
        if (strlen(line) == 0) continue;

        // Dòng 1: header
        if (lineNum == 1) {
            if (countColumns(line) != EXPECTED_DATA_COLS) {
                printf("[CSV] Loi: Header cua '%s' khong dung dinh dang (%d cot, can %d).\n",
                       filename, countColumns(line), EXPECTED_DATA_COLS);
                fclose(f);
                return CSV_ERR_INVALID_HEADER;
            }
            continue;
        }

        if (countColumns(line) != EXPECTED_DATA_COLS) {
            printf("[CSV] Canh bao: Dong %d trong '%s' thieu cot, da bo qua.\n", lineNum, filename);
            skipped++;
            continue;
        }

        uint32_t sensor_id;
        uint64_t timestamp;
        int32_t  raw_value;

        int parsed = sscanf(line, "%u,%" SCNu64 ",%d", &sensor_id, &timestamp, &raw_value);
        if (parsed != EXPECTED_DATA_COLS) {
            printf("[CSV] Canh bao: Dong %d trong '%s' gia tri khong hop le, da bo qua.\n", lineNum, filename);
            skipped++;
            continue;
        }

        processIncomingData(list, report, sensor_id, timestamp, raw_value);
        processed++;
    }

    fclose(f);
    printf("[CSV] '%s': Xu ly thanh cong %d ban tin, bo qua %d dong loi.\n",
           filename, processed, skipped);
    return CSV_OK;
}