#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include "global_type.h"
#include "botan.h"
#include "push_id_kit.h"
#include "log.h"

#include <string>
using namespace std;

using namespace Botan;

#define PUSH_ID_LEN_BYTE        16

#define PUSH_ID_SALT_NUM		16

//Ensure Botan thread safe
static LibraryInitializer init("thread_safe=true");

static std::string PUSH_ID_SALT[PUSH_ID_SALT_NUM] = {
    "ANKIYRHPMGEHK", "JGGLPRYBJBSDY", "KTDJORUNMYSVJ", "UPVWEIYLUFQGJ",
    "GHOKSMDSZOXIO", "GNFNEFBWRBAIV", "VBMOEHNYSKFBP", "JKMVTDHHVSIMO",
    "LFXOYKGYIYWBK", "FDVCTFINGRMPL", "DJTMMXOIGOMJB", "DZJGEAJFBWWBT",
    "FUURLTLHSMLAJ", "GYHXJBFDHZJHS", "RZAWOUWAQTRNS", "BCKEIXMHGTEFQ"
};

static void convertStr2Bytes(const std::string& str, byte* bytebuf)
{
    for(int i = 0; i < PUSH_ID_LEN_BYTE * 2; i += 2){
        byte ptr = strtoul(str.substr(i, 2).c_str(), NULL, 16);
        bytebuf[i/2] = ptr;
    }
}
static void convertBytes2Str(byte* bytebuf, std::string& str)
{
    char ptr[PUSH_ID_LEN_BYTE * 2 + 1];
    snprintf(ptr, sizeof(ptr), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        bytebuf[0], bytebuf[1], bytebuf[2], bytebuf[3], bytebuf[4], bytebuf[5], bytebuf[6], bytebuf[7],
        bytebuf[8], bytebuf[9], bytebuf[10], bytebuf[11], bytebuf[12], bytebuf[13], bytebuf[14], bytebuf[15]);
    str = std::string(ptr);
}

static int PushIDDecryption(byte* cipherText, byte* plainText, int saltIndex)
{
    HashFunction* hash = get_hash("MD5");

    SymmetricKey key = hash->process(PUSH_ID_SALT[saltIndex]);

    Pipe pipe(get_cipher("AES-128/ECB", key, DECRYPTION));

    try{
        pipe.process_msg(cipherText, PUSH_ID_LEN_BYTE);
    }catch(Botan::Decoding_Error &e){
        delete hash;
        return -1;
    }
    if(PUSH_ID_LEN_BYTE != pipe.read(plainText, PUSH_ID_LEN_BYTE)){
        delete hash;
        return -1;
    }

    delete hash;
    return 0;
}
static int PushIDEncryption(byte* plainText, byte* cipherText, int saltIndex)
{
    //static pthread_mutex_t hash_locker = PTHREAD_MUTEX_INITIALIZER;
    //pthread_mutex_lock(&hash_locker);
    HashFunction* hash = get_hash("MD5");
    //pthread_mutex_unlock(&hash_locker);

    SymmetricKey key = hash->process(PUSH_ID_SALT[saltIndex]);

    Pipe pipe(get_cipher("AES-128/ECB", key, ENCRYPTION));

    try{
        pipe.process_msg(plainText, PUSH_ID_LEN_BYTE);
    }catch(Botan::Decoding_Error &e){
        delete hash;
        return -1;
    }

    if(PUSH_ID_LEN_BYTE != pipe.read(cipherText, PUSH_ID_LEN_BYTE)){
        delete hash;
        return -1;
    }

    delete hash;
    return 0;
}

static string convertIpToStr(UINT32 ip)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = ip;
    char *ip_str = inet_ntoa(ip_addr);

    return string(ip_str);
}

PushIdKit::RetCode PushIdKit::GeneratePushId(string& pushid, UINT32 ip, UINT32 adid, UINT8 bidtype)
{
    // push_id (128 bits)
    // | bidtype (8 bits) | random (16 bits) | IP (32 bits) | AdID (32 bits)  | time (40 bits)  |
    byte plainText[PUSH_ID_LEN_BYTE];
    byte cipherText[PUSH_ID_LEN_BYTE];

    struct timeval tv;
    gettimeofday(&tv, NULL);
    UINT16 randomNum = rand() & 0xFFFF;
    UINT64 currentTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    currentTime = currentTime & 0xFFFFFFFFFF;
    UINT8 cth = (UINT8)(currentTime >> 32);
    UINT32 ctl = (UINT32)(currentTime);

    /* bidtype 1 byte*/
    memcpy(plainText, (void*)&bidtype, 1);

    /* random 2 byte */
    memcpy(plainText + 1, (void*)&randomNum, 2);

    /* adid 4 byte */
    memcpy(plainText + 3, (void*)&adid, 4);

    /* ip 4 byte */
    memcpy(plainText + 7, (void*)&ip, 4);

    /* time 5 byte */
    memcpy(plainText + 11, (void*)&ctl, 4);
    memcpy(plainText + 15, (void*)&cth, 1);

    if(PushIDEncryption(plainText, cipherText, adid%PUSH_ID_SALT_NUM) != 0){
        return RC_CRYPT_ERR;
    }

    convertBytes2Str(cipherText, pushid);
    return RC_OK;
}

/*****************************************************************************
 函 数 名  : PushIdKit.CheckPushId
 功能描述  : 检查pushid是否合法
 输入参数  : const string& pushid
             UINT32 ip
             UINT32 adid
             UINT8 bidtype
             UINT32 timeout_ms 超时时间,单位毫秒,即本次请求的时间和pushid生成的
                               时间的最大时间间隔
 输出参数  : 无
 返 回 值  : PushIdKit
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : Saturday, July 16, 2016
    作    者   : yuxuecheng
    修改内容   : 新生成函数

*****************************************************************************/
PushIdKit::RetCode PushIdKit::CheckPushId(const string& pushid, UINT32 ip, UINT32 adid, UINT8 bidtype, UINT32 timeout_ms)
{
    // push_id (128 bits)
    // | bidtype (8 bits) | random (16 bits) | IP (32 bits) | AdID (32 bits)  | time (40 bits)  |

    DEBUG("%s, %d", pushid.c_str(), pushid.length());
    if(pushid.length() != 2 * PUSH_ID_LEN_BYTE){
        return RC_INVALID_PUSHID;
    }

    byte cipherText[PUSH_ID_LEN_BYTE];
    convertStr2Bytes(pushid, cipherText);
    byte plainText[PUSH_ID_LEN_BYTE];
    if(PushIDDecryption(cipherText, plainText, adid%PUSH_ID_SALT_NUM) != 0){
        return RC_CRYPT_ERR;
    }

    /* bidtype 1 byte*/
    UINT8 tmp_bidtype;
    memcpy((void*)&tmp_bidtype, plainText, 1);

    /* random 2 byte */
    UINT16 tmp_randnum;
    memcpy((void*)&tmp_randnum, plainText, 2);

    /* adid 4 byte */
    UINT32 tmp_adid;
    memcpy((void*)&tmp_adid, plainText + 3, 4);

    /* ip 4 byte */
    UINT32 tmp_ip;
    memcpy((void*)&tmp_ip, plainText + 7, 4);

    /* time 5 byte */
    UINT32 tmp_ctl;
    UINT8 tmp_cth;
    memcpy((void*)&tmp_ctl, plainText + 11, 4);
    memcpy((void*)&tmp_cth, plainText + 15, 1);
    UINT64 tmp_time = tmp_cth;
    tmp_time <<= 32;
    tmp_time |= tmp_ctl;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    UINT64 currentTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    currentTime = currentTime&0xFFFFFFFFFF;

    if(tmp_bidtype != bidtype){
        ERROR("pushid: %s. real adid: %d, bidtype error, expect: %d, real: %d",
               pushid.c_str(), adid, tmp_bidtype, bidtype);
        return RC_INVALID_BIDTYPE;
    }
    if(tmp_adid != adid){
        ERROR("pushid: %s. adid error, expect: %d, real: %d",
               pushid.c_str(), tmp_adid, adid);
        return RC_INVALID_ADID;
    }
    if(tmp_ip != ip){
        ERROR("pushid: %s. real adid: %d, ip error, expect: %s, real: %s",
               pushid.c_str(), adid, (convertIpToStr(tmp_ip)).c_str(), (convertIpToStr(ip)).c_str());
        return RC_INVALID_IP;
    }
    if(tmp_time + timeout_ms < currentTime){
        ERROR("pushid: %s. real adid: %d, timeout error, begin time: %ld, now: %ld",
               pushid.c_str(), adid, tmp_time, currentTime);
        return RC_TIMEOUT_ERR;
    }
    return RC_OK;
}


PushIdKit::RetCode PushIdKit::GetIpFromPushId(const std::string& pushid, UINT32 adid, std::string& ip)
{
    // push_id (128 bits)
    // | bidtype (8 bits) | random (16 bits) | IP (32 bits) | AdID (32 bits)  | time (40 bits)  |

    DEBUG("%s, %d", pushid.c_str(), pushid.length());
    if(pushid.length() != 2 * PUSH_ID_LEN_BYTE){
        return RC_INVALID_PUSHID;
    }

    byte cipherText[PUSH_ID_LEN_BYTE];
    convertStr2Bytes(pushid, cipherText);
    byte plainText[PUSH_ID_LEN_BYTE];
    if(PushIDDecryption(cipherText, plainText, adid%PUSH_ID_SALT_NUM) != 0){
        return RC_CRYPT_ERR;
    }

    /* ip 4 byte */
    UINT32 tmp_ip;
    memcpy((void*)&tmp_ip, plainText + 7, 4);
    ip = convertIpToStr(tmp_ip);

    return RC_OK;
}





