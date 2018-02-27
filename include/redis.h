#ifndef DPC_REDIS_H_
#define DPC_REDIS_H_

#include <string>
#include <vector>
#include <map>
#include <pthread.h>

#include "hiredis.h"


using namespace std;

class Redis {
    public:
        Redis(const string &ip,const int port,struct timeval tv, const int db = 0, const int on = 0);
        ~Redis();

        int select(const int db = 0);
        void free();
        int dbsize();
        int keys(const string &regular,vector<string> &keys);

        int err() {
            return _conn->err;
        }
        /* @brif delete the key from redis
         * key[in]  the key to del
         * [return] the count of the key deleted
         **/
        int del(const string &key);

        /* @brif delete some keys from redis
         * key[in]  the key to del
         * [return] the count of the key deleted
         **/
        int del(vector<string> &keys);

        int incr(const string& key);
        int get(const string &key,string &val);
        int mget(vector<string> &keys,vector<string> &vals);
        //int set(const string &key,const string &val);
        int set(const string &key,const char* val);

		int expire(const string &key, const int timeoutSec);

        int zadd(const string &key,const double score,const string &member);
        int zrange(const string &key,vector<string> &values);
        int zrange(const string &key,const int start,const int end,vector<string> &values);
        int zrange_withscores(const string &key,map<string,double> &values);
        int zrange_withscores(const string &key,const int start,const int end,map<string,double> &values);

    private:
        int _ison;
        string _ip;
        int _port;
        int _db;
        pthread_mutex_t  _mutex;
        redisContext * _conn;
        struct timeval _timeout;

        int _checkReply(redisReply *reply);
        int connect();
        static uint16_t connCount;
};

#endif
