#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "ErrorHandler.h"
#include "LoggingHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Định nghĩa dấu phẩy tĩnh */
#define FIXED_POINT_SCALE 1000
/* Kết thúc định nghĩa*/

/* Định nghĩa MỘT BẢN TIN dữ liệu (Node) */
typedef struct DataNode {
    uint64_t timestamp;
    int32_t value;         
    struct DataNode *next;  // Trỏ tới bản tin tiếp theo
} DataNode;
/* Kết thúc định nghĩa MỘT BẢN TIN dữ liệu */
/* Định nghĩa DANH SÁCH quản lý dữ liệu */
typedef struct {
    DataNode *head;
    DataNode *tail;
} DataList;
/* Kết thúc định nghĩa DANH SÁCH quản lý dữ liệu */
/* Hàm khởi tạo một bản tin dữ liệu mới */
DataNode* createDataNode(uint64_t timestamp, int32_t value) {
    DataNode *newNode = (DataNode*)malloc(sizeof(DataNode));
    if (newNode == NULL) {
        printf("Lỗi cấp phát bộ nhớ cho bản tin dữ liệu mới.\n");
        return NULL;
    }
    newNode->timestamp = timestamp;
    newNode->value = value;
    newNode->next = NULL;
    return newNode;
}
/* Kết thúc hàm khởi tạo một bản tin dữ liệu mới */
/* Hàm thêm một bản tin dữ liệu vào cuối danh sách */
void appendDataNode(DataList *list, uint64_t timestamp, int32_t value)
{
    DataNode *newNode = createDataNode(timestamp, value);
    if (newNode == NULL) {
        return; // Lỗi đã được in ra trong createDataNode
    }
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}
/* Kết thúc hàm thêm một bản tin dữ liệu vào cuối danh sách */
/* Hàm xóa một bản tin dữ liệu khỏi danh sách và giải phóng bộ nhớ */
void deleteDataNode(DataList *list, uint64_t timestamp, int32_t value) {
    DataNode *current = list->head;
    DataNode *prev = NULL;
    while (current != NULL) {
        if (current->timestamp == timestamp && current->value == value) {
            if (prev == NULL) { // Xóa head
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == list->tail) { // Xóa tail
                list->tail = prev;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}
/* Kết thúc hàm xóa một bản tin dữ liệu khỏi danh sách và giải phóng bộ nhớ */
#endif