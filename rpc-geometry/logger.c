#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#include "logger.h"

void logger_init_file(FILE *file) {
    if(g_logger.f != NULL){
        printf("Logger has already been enabled\n");
        return;
    }
    g_logger.f = file;
    
    g_logger.level.error = "alert";
    g_logger.level.warning = "warning";
    g_logger.level.info = "info";
    g_logger.level.fine = "fine";
    
    printf("Logger enabled\n");
    glog(LOG_FINE, "Logger enabled");
    
}

void logger_init(time_t time) {
    char filename[64];
    FILE *f;
    struct tm *t = localtime(&time);
    
    sprintf(filename, "logs/tts_%4d-%02d-%02d_%02d-%02d-%02d.log",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    printf("Log filename: %s \n", filename);
    mkdir("logs", 644);
    f = fopen(filename, "w");
    
    if(!f){
        printf("Failed creating log file\n");
        f = stdout;
    }
    logger_init_file(f);
}

void logger_close() {
    fclose(g_logger.f);
}

char *_logger_label(int level) {
    switch (level) {
        case LOG_ERROR: return g_logger.level.error;
        case LOG_WARNING: return g_logger.level.warning;
        case LOG_INFO: return g_logger.level.info;
        case LOG_FINE: return g_logger.level.fine;
        
        default: return "???";
    }
}

void glog(int level, const char *msg_format, ...) {
    char message[GLOG_BUFFER];
    time_t raw_time = time(NULL);
    struct tm *now = localtime(&raw_time);
    va_list args;
    char *lvl_label;
    
    va_start(args, msg_format);
    vsnprintf(message, GLOG_BUFFER - 1, msg_format, args);
    va_end(args);
    lvl_label = _logger_label(level);
    
    fprintf(g_logger.f, "%02d:%02d:%02d <%8s> : %s\n", 
            now->tm_hour, now->tm_min, now->tm_sec, lvl_label, message);
    fflush(g_logger.f);
}