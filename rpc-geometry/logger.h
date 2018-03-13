#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 4
#define LOG_FINE 8

#define GLOG_BUFFER 1024



struct
{
    FILE *f;
    
    struct {
        char *fine, *info, *error, *warning;
    } level;
} g_logger;

void logger_init_file(FILE *file);

void logger_init(time_t time);

void glog(int level, const char *msg_format, ...);

#endif /* LOGGER_H */

