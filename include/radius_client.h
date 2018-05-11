#ifndef RADIUS_CLIENT_H
#define RADIUS_CLIENT_H

#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <boost/program_options.hpp>

#include "redis.h"

using namespace std;

#define SERV_ACN_PROTOCOL  0

#define USER_ONLINE     1
#define USER_OFFLINE    2
#define USER_UPDATE     3

#define IP_MAX_LEN      (32)
#define APN_MAX_LEN     (16)
#define SESSION_MAX_LEN    (64)

#ifdef YINNI
#define IMSI_MAX_LEN    (60)
#define NAME_MAX_LEN    (60)
#define ADDRESS_LEN     (32)
#define IMEI_MAX_LEN    (60)
#else
#define IMSI_MAX_LEN    (32)
#define NAME_MAX_LEN    (28)
#define IMEI_MAX_LEN    (60)
#endif

#define SECONDS_OF_DAY 86400

typedef struct acn_simple_info_s {
    uint32_t start;
    uint32_t end;
    char     name[NAME_MAX_LEN];
}acn_simple_info_t;

#ifdef YINNI
typedef struct acn_user_info_s
{
    uint8_t  flag;
    uint16_t lac;
    uint16_t ci;
    uint32_t nat444_begin_port;
    uint32_t nat444_end_port;
    uint32_t ip;
    uint32_t type;
    uint32_t teid;
    uint64_t phone;

    char     apn[APN_MAX_LEN];
    char     imsi[IMSI_MAX_LEN];
    char     imei[IMSI_MAX_LEN];
    char     name[NAME_MAX_LEN]; //adsl name, or phone
    //char     session_id[SESSION_MAX_LEN];
    char     net_type[16];
    char     address[ADDRESS_LEN] ;  // 基站编号
    std::map<uint32_t, acn_simple_info_t*> *location_map_;
    uint32_t add_time;
} acn_user_info_t;
#elifdef WIRELESS
typedef struct acn_user_info_s
{
    uint8_t  flag;
    uint16_t lac;
    uint16_t ci;
    uint16_t tac;//3g
    uint32_t eci;//3g
    uint32_t nat444_begin_port;
    uint32_t nat444_end_port;
    uint32_t ip;
    uint32_t type;
    uint32_t teid;
    uint64_t phone;

    char     apn[APN_MAX_LEN];
    char     imsi[IMSI_MAX_LEN];
    char     imei[IMSI_MAX_LEN];
    char     name[NAME_MAX_LEN]; //adsl name, or phone
    //char     session_id[SESSION_MAX_LEN];
    char     net_type[16];
    std::map<uint32_t, acn_simple_info_t*> *location_map_;
    uint32_t add_time;
} acn_user_info_t;
#else
typedef struct acn_user_info_s
{
    uint8_t  flag;
    uint16_t lac;
    uint16_t ci;
    uint32_t nat444_begin_port;
    uint32_t nat444_end_port;
    uint32_t ip;
    uint32_t type;
    uint32_t teid;
    uint64_t phone;

    char     apn[APN_MAX_LEN];
    char     imsi[IMSI_MAX_LEN];
    char     name[NAME_MAX_LEN]; //adsl name, or phone
    char     session_id[SESSION_MAX_LEN];
    char     net_type[16];
    std::map<uint32_t, acn_simple_info_t*> *location_map_;
    uint32_t add_time;
} acn_user_info_t;
#endif


int store_msg_redis(std::string host, int port, int db, const char* ip, std::string adsl);

#endif
