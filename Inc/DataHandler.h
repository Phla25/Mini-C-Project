#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "ErrorHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Định nghĩa dấu phẩy tĩnh */
#define FIXED_POINT_SCALE 1000

/* Định nghĩa MỘT BẢN TIN dữ liệu (Node) */
typedef struct DataNode {
    uint64_t timestamp;
    int32_t value;         
    struct DataNode *next;
} DataNode;

/* Định nghĩa DANH SÁCH quản lý dữ liệu */
typedef struct {
    DataNode *head;
    DataNode *tail;
} DataList;

/* Prototype các hàm xử lý dữ liệu */
DataNode* createDataNode(uint64_t timestamp, int32_t value);
void appendDataNode(DataList *list, uint64_t timestamp, int32_t value);
void deleteDataNode(DataList *list, uint64_t timestamp, int32_t value);
void freeDataList(DataList *list); // FIX: Thêm hàm giải phóng toàn bộ list

#endif