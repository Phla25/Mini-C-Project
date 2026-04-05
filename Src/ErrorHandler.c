#include "../Inc/ErrorHandler.h"
#include <stdio.h>
#include <stdbool.h>

void update_stats(ValidationStatus status, SystemReport *report) {
    if (report == NULL) return;

    // Nếu dữ liệu bình thường hoặc chỉ bị cảnh báo ngưỡng
    if (status == STATUS_VALID_NORMAL) {
        report->total_valid++;
    } 
    else if (status == STATUS_WARNING_HIGH || status == STATUS_WARNING_LOW) {
        report->total_valid++;     // Cảnh báo vẫn tính là bản tin hợp lệ để xử lý
        report->total_threshold++;  // tăng thêm biến đếm vượt ngưỡng
    } 
    else {
        // Các trường hợp còn lại  là lỗi hệ thống hoặc dữ liệu hỏng
        report->total_error++;
    }
}


void handle_buffer_error(bool is_full, SystemReport *report) {
    if (report == NULL) return;

    if (is_full) {
        report->buffer_overflow++; // Thống kê số lần overflow
        report->packets_dropped++; // Một bản tin cũ bị ghi đè coi như bị bỏ qua
    }
}
