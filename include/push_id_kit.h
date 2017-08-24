#ifndef PUSH_ID_KIT_H
#define PUSH_ID_KIT_H

#include <string>

#include "global_type.h"

class PushIdKit
{
    public:
        enum RetCode{
            RC_OK = 0,
            RC_INVALID_PUSHID,
            RC_INVALID_IP,
            RC_INVALID_ADID,
            RC_INVALID_BIDTYPE,
            RC_TIMEOUT_ERR,
            RC_CRYPT_ERR,
        };

        static RetCode GeneratePushId(std::string& pushid, UINT32 ip, UINT32 adid, UINT8 bidtype);
        static RetCode CheckPushId(const std::string& pushid, UINT32 ip, UINT32 adid, UINT8 bidtype, UINT32 timeout_ms);
        static RetCode GetIpFromPushId(const std::string& pushid, UINT32 adid, std::string& ip);
};

#endif // PUSH_ID_KIT_H
