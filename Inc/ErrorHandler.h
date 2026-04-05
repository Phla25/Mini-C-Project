#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

typedef enum {
    // Nhóm hợp lệ (Được phép lưu vào DataList)
    STATUS_VALID_NORMAL = 0,
    STATUS_WARNING_HIGH,       // Vượt ngưỡng trên
    STATUS_WARNING_LOW,        // Dưới ngưỡng dưới
    
    // Nhóm lỗi (Bị loại bỏ / Drop packet)
    ERR_SENSOR_NOT_FOUND,      // Không tìm thấy cảm biến với ID này
    ERR_INVALID_TIMESTAMP,     // Thời gian bị sai (VD: nhỏ hơn thời gian cũ)
    ERR_PHYSICAL_BOUNDS        // Lỗi phần cứng: Giá trị vật lý vô lý (VD: Độ ẩm > 100%)
} ValidationStatus;

// THÊM MỚI: Cấu trúc để đếm số liệu (Thống kê)
typedef struct {
    int total_valid;         // Số bản tin hợp lệ
    int total_error;         // Số bản tin bị lỗi (ID sai, Timestamp sai...)
    int total_threshold;     // Số lần vượt ngưỡng
    int buffer_overflow;     // Số lần tràn bộ đệm
    int packets_dropped;     // Số bản tin bị bỏ qua/ghi đè
} SystemReport;

// THÊM MỚI: Khai báo các hàm để file .c triển khai
void update_stats(ValidationStatus status, SystemReport *report);
void handle_buffer_error(bool is_full, SystemStats *stats);

#endif
