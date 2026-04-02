#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H
#include "ErrorHandler.h"
#include "LoggingHandler.h"
#include "DataHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    int id;
    char name[50];
    struct Data *root;
} Sensor;
#end