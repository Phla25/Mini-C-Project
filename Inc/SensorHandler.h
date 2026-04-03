#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include "ErrorHandler.h"
#include "DataHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define FILTER_WINDOW_SIZE 5
#define MAX_ALLOWED_JUMP 10000 // Độ lệch tối đa cho phép (10.0 độ/đơn vị)
#define CONNECTION_TIMEOUT_MULTIPLIER 3 // Nếu trễ gấp 3 lần sending_cycle thì coi như mất kết nối
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
    /* Bộ đệm trượt để tính trung bình động */
    int32_t moving_avg_buffer[FILTER_WINDOW_SIZE]; 
    int buffer_index;              
    int samples_collected;
    /* Kết thúc phần định nghĩa bộ đệm trượt */
    /* --- BIẾN THỐNG KÊ THEO THỜI GIAN THỰC --- */
    int32_t stat_max;           // Giá trị lớn nhất
    int32_t stat_min;           // Giá trị nhỏ nhất
    int64_t stat_sum;           
    uint32_t stat_valid_count;  // Tổng số bản tin hợp lệ đã nhận
    uint32_t stat_error_count;  // Tổng số bản tin bị lỗi / nhiễu
    /* ------------------------------------------------- */
    uint64_t last_seen_timestamp;
    DataList history;
    struct Sensor *nextSensor;
} Sensor;
/* Kết thúc định nghĩa cấu trúc cảm biến */
/* Định nghĩa cấu trúc danh sách cảm biến */
typedef struct {
    Sensor *head;
    Sensor *tail;
} SensorList;
/* Kết thúc định nghĩa cấu trúc danh sách cảm biến */
/* Hàm khởi tạo một cảm biến mới */
Sensor* createSensor(uint32_t id, DataType type, 
    int32_t longitude, int32_t latitude, int32_t min_threshold, 
    int32_t max_threshold, uint32_t sending_cycle, uint32_t buffer_size){
    Sensor *newSensor = (Sensor*)malloc(sizeof(Sensor));
    if (newSensor == NULL) {
        printf("Lỗi cấp phát bộ nhớ cho cảm biến mới.\n");
        return NULL;
    }
    newSensor->id = id;
    newSensor->type = type;
    newSensor->longitude = longitude;
    newSensor->latitude = latitude;
    newSensor->min_threshold = min_threshold;
    newSensor->max_threshold = max_threshold;
    newSensor->sending_cycle = sending_cycle;
    newSensor->buffer_size = buffer_size;
    newSensor->buffer_index = 0;
    newSensor->samples_collected = 0;
    newSensor->stat_max = -2147483648; // Giá trị nhỏ nhất có thể của int32 (INT32_MIN)
    newSensor->stat_min = 2147483647;  // Giá trị lớn nhất có thể của int32 (INT32_MAX)
    newSensor->stat_sum = 0;
    newSensor->stat_valid_count = 0;
    newSensor->stat_error_count = 0;
    newSensor->last_seen_timestamp = 0;
    newSensor->history.head = NULL;
    newSensor->history.tail = NULL;
    newSensor->nextSensor = NULL;
    return newSensor;
}
/* Kết thúc định nghĩa hàm khởi tạo cảm biến mới */
/* Thêm cảm biến vào danh sách */
void addSensor(SensorList *list, Sensor *newSensor){
    if (list->head == NULL) {
        list->head = newSensor;
        list->tail = newSensor;
    } else {
        list->tail->nextSensor = newSensor;
        list->tail = newSensor;
    }
}
/* Kết thúc định nghĩa hàm thêm cảm biến vào danh sách */
/* Tìm cảm biến theo ID */
Sensor* findSensorByID(SensorList *list, uint32_t id){
    Sensor *current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->nextSensor;
    }
    return NULL; // Không tìm thấy
}
/* Kết thúc định nghĩa hàm tìm cảm biến theo ID */
/* Hàm xóa một cảm biến khỏi danh sách và giải phóng bộ nhớ */
void deleteSensor(SensorList *list, uint32_t id){
    Sensor *current = list->head;
    Sensor *prev = NULL;
    while (current != NULL) {
        if (current->id == id) {
            if (prev == NULL) { // Xóa head
                list->head = current->nextSensor;
            } else {
                prev->nextSensor = current->nextSensor;
            }
            if (current == list->tail) { // Xóa tail
                list->tail = prev;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->nextSensor;
    }
}
/* Kết thúc định nghĩa hàm xóa cảm biến khỏi danh sách */
/* Hàm lọc nhiễu dữ liệu đầu vào */
bool filterSensorData(Sensor *sensor, int32_t raw_value, int32_t *final_value) {
    // 1. Lọc giá trị bất thường (Outlier Rejection)
    if (sensor->samples_collected > 0) {
        int last_index = (sensor->buffer_index == 0) ? (FILTER_WINDOW_SIZE - 1) : (sensor->buffer_index - 1);
        int32_t last_valid_value = sensor->moving_avg_buffer[last_index];

        if (abs(raw_value - last_valid_value) > MAX_ALLOWED_JUMP) {
            return false; // Phát hiện nhiễu, từ chối dữ liệu
        }
    }

    // 2. Cập nhật bộ đệm trượt
    sensor->moving_avg_buffer[sensor->buffer_index] = raw_value;
    sensor->buffer_index = (sensor->buffer_index + 1) % FILTER_WINDOW_SIZE;

    if (sensor->samples_collected < FILTER_WINDOW_SIZE) {
        sensor->samples_collected++;
    }

    // 3. Tính trung bình trượt
    int64_t sum = 0;
    for (int i = 0; i < sensor->samples_collected; i++) {
        sum += sensor->moving_avg_buffer[i];
    }

    *final_value = (int32_t)(sum / sensor->samples_collected);
    return true; // Lọc thành công
}
/* Kết thúc hàm lọc nhiễu */
/* Hàm cập nhật dữ liệu mới cho cảm biến */
void updateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value){
    sensor->last_seen_timestamp = timestamp;
    appendDataNode(&sensor->history, timestamp, value);
    if (value > sensor->stat_max) {
        sensor->stat_max = value;
    }
    if (value < sensor->stat_min) {
        sensor->stat_min = value;
    }
    sensor->stat_sum += value;
    sensor->stat_valid_count++;
}
/* Kết thúc định nghĩa hàm cập nhật dữ liệu mới cho cảm biến */
/* Hàm xóa dữ liệu cũ của cảm biến */
void deleteSensorData(Sensor *sensor, uint64_t timestamp, int32_t value){
    deleteDataNode(&sensor->history, timestamp, value);
}
/* Kết thúc định nghĩa hàm xóa dữ liệu cũ của cảm biến */
/* Hàm quét và phát hiện các cảm biến mất kết nối */
int checkSensorConnections(SensorList *list, uint64_t current_timestamp) {
    Sensor *current = list->head;
    while (current != NULL) {
        // Nếu đã từng gửi data, nhưng hiện tại quá chu kỳ gửi (VD: trễ gấp 3 lần sending_cycle) thì báo lỗi
        // sending_cycle là mili-giây, giả sử timestamp là mili-giây
        uint64_t timeout_threshold = current->sending_cycle * CONNECTION_TIMEOUT_MULTIPLIER; 

        if (current->last_seen_timestamp > 0 && 
           (current_timestamp - current->last_seen_timestamp > timeout_threshold)) {
            return 1; // Phát hiện cảm biến mất kết nối
        }
        current = current->nextSensor;
    }
    return 0;
}
/* Kết thúc hàm quét kết nối */
/* Hàm kiểm tra tính hợp lệ và cảnh báo vượt ngưỡng */
ValidationStatus validateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value) {
    if (sensor == NULL) return ERR_SENSOR_NOT_FOUND;

    // 1. Kiểm tra tính hợp lệ của thời gian (Không được lùi về quá khứ)
    if (sensor->history.tail != NULL && timestamp <= sensor->history.tail->timestamp) {
        return ERR_INVALID_TIMESTAMP;
    }

    // 2. Kiểm tra giới hạn vật lý (Ví dụ độ ẩm không thể < 0 hoặc > 100%)
    if (sensor->type == TYPE_HUMIDITY && (value < 0 || value > 100 * FIXED_POINT_SCALE)) {
        return ERR_PHYSICAL_BOUNDS;
    }
    if ((sensor->type == TYPE_LIGHT || sensor->type == TYPE_DUST) && value < 0) {
        return ERR_PHYSICAL_BOUNDS; 
    }

    // 3. Phát hiện vượt ngưỡng cảnh báo
    if (value > sensor->max_threshold) {
        return STATUS_WARNING_HIGH;
    }
    if (value < sensor->min_threshold) {
        return STATUS_WARNING_LOW;
    }

    return STATUS_VALID_NORMAL;
}
/* Kết thúc hàm kiểm tra tính hợp lệ */
#endif