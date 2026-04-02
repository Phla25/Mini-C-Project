#ifndef DATAHANDLER_H
#define DATAHANDLER_H
#include "ErrorHandler.h"
#include "LoggingHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    int timestamp;
    float value;
    struct Data *leftData;
    struct Data *rightData;
} Data;
#endif 