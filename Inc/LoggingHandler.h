#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include "SensorHandler.h"
#include <time.h>

/* Hàm xuất báo cáo tổng hợp ra file .txt */
void exportFullReport(const char* filename, SensorList *list) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Lỗi: Không thể tạo file báo cáo %s\n", filename);
        return;
    }

    // Ghi tiêu đề báo cáo
    time_t now = time(NULL);
    fprintf(f, "====================================================\n");
    fprintf(f, "       BÁO CÁO HỆ THỐNG GIÁM SÁT MÔI TRƯỜNG        \n");
    fprintf(f, "       Thời gian xuất: %s", ctime(&now));
    fprintf(f, "====================================================\n\n");

    Sensor *current = list->head;
    if (current == NULL) {
        fprintf(f, "Hệ thống chưa có cảm biến nào được đăng ký.\n");
    }

    while (current != NULL) {
        fprintf(f, "--- CẢM BIẾN ID: %u ---\n", current->id);
        fprintf(f, "Loại thiết bị      : %d (0:Temp, 1:Humid, 2:Light...)\n", current->type);
        fprintf(f, "Vị trí             : Lat(%d), Lon(%d)\n", current->latitude, current->longitude);
        fprintf(f, "Số bản tin hợp lệ  : %u\n", current->stat_valid_count);
        fprintf(f, "Số bản tin lỗi/nhiễu: %u\n", current->stat_error_count);

        if (current->stat_valid_count > 0) {
            int32_t avg = (int32_t)(current->stat_sum / current->stat_valid_count);
            
            // Xuất số phẩy tĩnh (chia 1000)
            fprintf(f, "Giá trị LỚN NHẤT   : %d.%03d\n", current->stat_max / 1000, abs(current->stat_max % 1000));
            fprintf(f, "Giá trị NHỎ NHẤT   : %d.%03d\n", current->stat_min / 1000, abs(current->stat_min % 1000));
            fprintf(f, "Giá trị TRUNG BÌNH : %d.%03d\n", avg / 1000, abs(avg % 1000));
        } else {
            fprintf(f, "Tình trạng         : Không có dữ liệu hoạt động.\n");
        }
        fprintf(f, "----------------------------------------------------\n\n");
        
        current = current->nextSensor;
    }

    fprintf(f, "============= KẾT THÚC BÁO CÁO =============\n");
    fclose(f);
    printf("Đã xuất báo cáo thành công ra file: %s\n", filename);
}

#endif