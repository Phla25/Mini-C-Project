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

#endif