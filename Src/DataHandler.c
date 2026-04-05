#include "../Inc/DataHandler.h"
#include <stdio.h>
#include <stdlib.h>

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

/* Hàm thêm một bản tin dữ liệu vào cuối danh sách */
void appendDataNode(DataList *list, uint64_t timestamp, int32_t value) {
    DataNode *newNode = createDataNode(timestamp, value);
    if (newNode == NULL) return;

    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

/* Hàm xóa một bản tin dữ liệu khỏi danh sách và giải phóng bộ nhớ */
void deleteDataNode(DataList *list, uint64_t timestamp, int32_t value) {
    DataNode *current = list->head;
    DataNode *prev = NULL;
    while (current != NULL) {
        if (current->timestamp == timestamp && current->value == value) {
            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == list->tail) {
                list->tail = prev;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

/* FIX: Giải phóng toàn bộ DataList - dùng khi deleteSensor */
void freeDataList(DataList *list) {
    DataNode *current = list->head;
    while (current != NULL) {
        DataNode *next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
}