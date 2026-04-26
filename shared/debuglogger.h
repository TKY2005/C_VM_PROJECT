#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H

#include<stdio.h>
#include<stdarg.h>
#include<time.h>

#define LOGS_PATH "./logs"

extern FILE* log_file;

void log_init();

void log_close();

void printd(char* log, ...);

#endif