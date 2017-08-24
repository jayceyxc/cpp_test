#ifndef LOG_H
#define LOG_H

#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <string>
using namespace std;

#define ERROR(fmt, arg...)  syslog(LOG_ERR, "FATAL [%s:%d]" fmt, __FILE__, __LINE__, ##arg)
#define WARN(fmt, arg...)   syslog(LOG_WARNING, "WARN [%s:%d]" fmt, __FILE__, __LINE__, ##arg)
#define NOTICE(fmt, arg...) syslog(LOG_NOTICE, "NOTICE " fmt, ##arg)
#define INFO(fmt, arg...)   syslog(LOG_INFO, "INFO [%s:%d]" fmt, __FILE__, __LINE__, ##arg)

#ifndef BIDDER_DEBUG
#define DEBUG(...)
#else
#define DEBUG(fmt, args...) syslog(LOG_DEBUG, "DEBUG [%s:%d]"fmt, __FILE__, __LINE__, ##args)
#endif

#endif // LOG_H
