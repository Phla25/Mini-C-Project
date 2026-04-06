#include "../Inc/SensorHandler.h"
#include <stdlib.h>
#include <stdio.h>

/* Hàm khởi tạo một cảm biến mới */
Sensor* createSensor(uint32_t id, DataType type, 
    int32_t longitude, int32_t latitude, int32_t min_threshold, 
    int32_t max_threshold, uint32_t sending_cycle, uint32_t buffer_size) {

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
    newSensor->stat_max = -2147483648;
    newSensor->stat_min = 2147483647;
    newSensor->stat_sum = 0;
    newSensor->stat_valid_count = 0;
    newSensor->stat_error_count = 0;
    newSensor->last_seen_timestamp = 0;
    newSensor->history.head = NULL;
    newSensor->history.tail = NULL;
    newSensor->nextSensor = NULL;
    return newSensor;
}

/* Thêm cảm biến vào danh sách */
void addSensor(SensorList *list, Sensor *newSensor) {
    if (list->head == NULL) {
        list->head = newSensor;
        list->tail = newSensor;
    } else {
        list->tail->nextSensor = newSensor;
        list->tail = newSensor;
    }
}

/* Tìm cảm biến theo ID */
Sensor* findSensorByID(SensorList *list, uint32_t id) {
    Sensor *current = list->head;
    while (current != NULL) {
        if (current->id == id) return current;
        current = current->nextSensor;
    }
    return NULL;
}

/* Giải phóng cả DataList bên trong trước khi free Sensor */
void deleteSensor(SensorList *list, uint32_t id) {
    Sensor *current = list->head;
    Sensor *prev = NULL;
    while (current != NULL) {
        if (current->id == id) {
            if (prev == NULL) {
                list->head = current->nextSensor;
            } else {
                prev->nextSensor = current->nextSensor;
            }
            if (current == list->tail) {
                list->tail = prev;
            }
            freeDataList(&current->history);
            free(current);
            return;
        }
        prev = current;
        current = current->nextSensor;
    }
}

/* Hàm lọc nhiễu dữ liệu đầu vào */
bool filterSensorData(Sensor *sensor, int32_t raw_value, int32_t *final_value) {
    if (sensor->samples_collected > 0) {
        int last_index = (sensor->buffer_index == 0) ? (FILTER_WINDOW_SIZE - 1) : (sensor->buffer_index - 1);
        int32_t last_valid_value = sensor->moving_avg_buffer[last_index];
        if (abs(raw_value - last_valid_value) > MAX_ALLOWED_JUMP) {
            return false;
        }
    }

    sensor->moving_avg_buffer[sensor->buffer_index] = raw_value;
    sensor->buffer_index = (sensor->buffer_index + 1) % FILTER_WINDOW_SIZE;
    if (sensor->samples_collected < FILTER_WINDOW_SIZE) {
        sensor->samples_collected++;
    }

    int64_t sum = 0;
    for (int i = 0; i < sensor->samples_collected; i++) {
        sum += sensor->moving_avg_buffer[i];
    }
    *final_value = (int32_t)(sum / sensor->samples_collected);
    return true;
}

/* Hàm cập nhật dữ liệu mới — ghi đè node cũ nhất nếu buffer đầy */
void updateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value, SystemReport *report) {
    // Nếu buffer đầy: xóa node head (bản tin cũ nhất) trước khi thêm mới
    if (sensor->buffer_size > 0 && sensor->stat_valid_count >= sensor->buffer_size) {
        handle_buffer_error(true, report); // Cập nhật thống kê overflow/dropped

        DataNode *old = sensor->history.head;
        if (old != NULL) {
            sensor->history.head = old->next;
            if (sensor->history.head == NULL) {
                sensor->history.tail = NULL;
            }
            sensor->stat_sum -= old->value;  // Trừ giá trị cũ ra khỏi tổng
            free(old);
            sensor->stat_valid_count--;  // Nhường chỗ cho bản tin mới
        }
    }

    sensor->last_seen_timestamp = timestamp;
    appendDataNode(&sensor->history, timestamp, value);
    if (value > sensor->stat_max) sensor->stat_max = value;
    if (value < sensor->stat_min) sensor->stat_min = value;
    sensor->stat_sum += value;
    sensor->stat_valid_count++;
}

/* Hàm xóa một bản tin cụ thể khỏi history */
void deleteSensorData(Sensor *sensor, uint64_t timestamp, int32_t value) {
    deleteDataNode(&sensor->history, timestamp, value);
}

/* Hàm quét và phát hiện các cảm biến mất kết nối */
int checkSensorConnections(SensorList *list, uint64_t current_timestamp) {
    Sensor *current = list->head;
    while (current != NULL) {
        uint64_t timeout_threshold = current->sending_cycle * CONNECTION_TIMEOUT_MULTIPLIER;
        if (current->last_seen_timestamp > 0 && 
           (current_timestamp - current->last_seen_timestamp > timeout_threshold)) {
            return 1;
        }
        current = current->nextSensor;
    }
    return 0;
}

/* Kiểm tra tính hợp lệ và cảnh báo vượt ngưỡng */
ValidationStatus validateSensorData(Sensor *sensor, uint64_t timestamp, int32_t value) {
    if (sensor == NULL) return ERR_SENSOR_NOT_FOUND;

    if (sensor->history.tail != NULL && timestamp <= sensor->history.tail->timestamp) {
        return ERR_INVALID_TIMESTAMP;
    }

    switch (sensor->type) {
        case TYPE_TEMP:
            if (value < TEMP_MIN_PHYSICAL || value > TEMP_MAX_PHYSICAL)
                return ERR_PHYSICAL_BOUNDS;
            break;
        case TYPE_HUMIDITY:
            if (value < 0 || value > 100 * FIXED_POINT_SCALE)
                return ERR_PHYSICAL_BOUNDS;
            break;
        case TYPE_LIGHT:
        case TYPE_DUST:
            if (value < 0)
                return ERR_PHYSICAL_BOUNDS;
            break;
        case TYPE_GAS:
            if (value < GAS_MIN_PHYSICAL || value > GAS_MAX_PHYSICAL)
                return ERR_PHYSICAL_BOUNDS;
            break;
        default:
            break;
    }

    if (value > sensor->max_threshold) return STATUS_WARNING_HIGH;
    if (value < sensor->min_threshold) return STATUS_WARNING_LOW;

    return STATUS_VALID_NORMAL;
}