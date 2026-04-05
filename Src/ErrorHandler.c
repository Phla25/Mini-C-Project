#include "../Inc/ErrorHandler.h"
#include <stdio.h>
#include <stdbool.h>

void update_stats(ValidationStatus status, SystemReport *report) {
    if (report == NULL) return;

    if (status == STATUS_VALID_NORMAL) {
        report->total_valid++;
    } 
    else if (status == STATUS_WARNING_HIGH || status == STATUS_WARNING_LOW) {
        report->total_valid++;      // Cảnh báo vẫn tính là bản tin hợp lệ
        report->total_threshold++;  // Tăng thêm biến đếm vượt ngưỡng
    } 
    else {
        report->total_error++;
    }
}

/* Hàm này giờ được gọi thực sự từ processIncomingData trong main.c */
void handle_buffer_error(bool is_full, SystemReport *report) {
    if (report == NULL) return;

    if (is_full) {
        report->buffer_overflow++;
        report->packets_dropped++;
    }
}