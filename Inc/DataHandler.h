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

#endif