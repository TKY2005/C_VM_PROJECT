#include<debuglogger.h>

#include<stdio.h>
#include<time.h>
#include<stdarg.h>

FILE* log_file;

void log_init() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char buff[100];

    strftime(buff, sizeof(buff), "VM_LOG %d-%m-%Y %H-%M-%S.log", t);

    char filename[1024];
    sprintf(filename, "%s/%s", LOGS_PATH, buff);
    log_file = fopen(filename, "w");

    if (!log_file){
        perror("Unable to create log file");
    }
    else printf("Log file: '%s' has been created.\n", filename);
}

void log_close() {
    if (log_file) fclose(log_file);
}

void printd(char* log, ...) {
    time_t time_now = time(NULL);
    struct tm* t = localtime(&time_now);
    char time[100];
    strftime(time, sizeof(time), "[%d/%m/%Y %H:%M:%S] ", t);
    fputs(time, log_file);

    char buff[4096] = {0};
    va_list args;
    va_start(args, log);

    vsnprintf(buff, sizeof(buff), log, args);
    
    va_end(args);

    fputs(buff, log_file);
}