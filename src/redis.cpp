#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include <string.h>

#include "log.h"
#include "dpc_redis.h"

using namespace std;

uint16_t Redis::connCount = 0;

Redis::Redis(const string &ip,const int port,struct timeval tv,const int db, const int on)
{
    _ison = on;
    if (_ison != 0) {
        _ip = ip;
        _port = port;
        _db = db;
        _timeout = tv;
        _conn = NULL;
        pthread_mutex_init(&_mutex,NULL);
        if (connect() != 0) {
            throw "Connect ERR";
        }
    }
}

Redis::~Redis()
{
    if (_ison != 0) {
        free();
        pthread_mutex_destroy(&_mutex);
    }
}

int Redis::connect()
{
    if (_ison == 0) {
        return 0;
    }
    if (pthread_mutex_trylock(&_mutex) == 0) {
        ++connCount;
        sleep(connCount);
        if (Redis::connCount > 10) {
            ERROR("Connection redis (%s:%d %d) failed 10 times and exit!",_ip.c_str(),_port,_db);
            exit(-1);
        }   
        if(_conn != NULL) {
            free();
        }
        _conn = NULL;
        _conn = redisConnectWithTimeout(_ip.c_str(),_port,_timeout);
        if (_conn == NULL)
            return -1;
        if(_conn->err != 0) {
            ERROR("Redis %s:%d %d  has err: %s",_ip.c_str(),_port,_db,_conn->errstr);
            free();
            return -1;
        }
        pthread_mutex_unlock(&_mutex);
        int rtn = select(_db);
        if(rtn == 0) {
            connCount = 0;
            return 0;
        }
    }
    return -1;
}


void Redis::free() 
{
    if (_conn != NULL) {
        redisFree(_conn); 
    }
}

int Redis::select(const int db)
{
    if (_ison == 0) {
        return 0;
    }
    if (db < 0)
        return -1;
    redisReply *reply = (redisReply *)redisCommand(_conn,"select %d",db);
    if (_checkReply(reply) == 0) {
        if (reply->type != REDIS_REPLY_STATUS || 
                0 != strcmp(reply->str,"OK") ) {
            freeReplyObject(reply);
            return -1;
        } else {
            _db = db;
            freeReplyObject(reply);
            return 0;
        }
    } else {
        return -1;
    }
    return 0;
}

int Redis::dbsize()
{
    if (_ison == 0) {
        return 0;
    }
    int size = 0;
    redisReply *reply = (redisReply *)redisCommand(_conn,"dbsize");
    if (_checkReply(reply) == 0) {

        if (reply->type != REDIS_REPLY_INTEGER) {
            freeReplyObject(reply);
            return -1;
        } else {
            size = reply->integer;
            freeReplyObject(reply);
        }
    } else {
        return -1;
    }
    return size;
}

int Redis::keys(const string &regular,vector<string> &keys)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"keys %s",regular.c_str());
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i=0;i<reply->elements;i++) {
                if (reply->element[i]->type == REDIS_REPLY_STRING) {
                    keys.push_back(reply->element[i]->str);
                }
            }
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::del(const string &key)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"DEL %s",key.c_str());
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_INTEGER) {
            int count = reply->integer;
            freeReplyObject(reply);
            return count;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::del(vector<string> &keys)
{
    if (_ison == 0) {
        return 0;
    }
    string key;
    for(vector<string>::iterator it = keys.begin();it != keys.end();it++) {
        key.append(*it);
        key.append(" ");
    }
    return del(key);
}


int Redis::incr(const string& key)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"INCR %s ",key.c_str());
    int ret = _checkReply(reply);
    freeReplyObject(reply);
    return ret;
}

int Redis::get(const string &key,string &value)
{
    if (_ison == 0) {
        return 0;
    }
    //value.clear();
    redisReply *reply = (redisReply *)redisCommand(_conn,"GET %s ",key.c_str());
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_STRING) {
            value = reply->str;
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::mget(vector<string> &keys,vector<string> &vals)
{
    if (_ison == 0) {
        return 0;
    }
    string key;
    for(vector<string>::iterator it = keys.begin();it!= keys.end();it++) {
        key += " " + *it;
    }

    redisReply *reply = (redisReply *)redisCommand(_conn,"MGET %s",key.c_str());
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_ARRAY) {
            for(size_t i =0;i<reply->elements;i++) {
                vals.push_back(reply->element[i]->str);
            }
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::set(const string &key,const char* value)
{
    if (_ison == 0) {
        return 0;
    }
    //redisReply *reply = (redisReply *)redisCommand(_conn,"set %s %s",key.c_str(),value.c_str());
    redisReply *reply = (redisReply *)redisCommand(_conn,"set %s %s",key.c_str(),value);
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_STATUS && 
                0 == strcmp(reply->str,"OK")) {
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::expire(const string &key,const int timeout)
{
	if (_ison == 0) {
		return 0;
	}
	redisReply *reply = (redisReply *)redisCommand(_conn,"expire %s %d",key.c_str(), timeout);
	if (_checkReply(reply) == 0) {
		if (reply->type == REDIS_REPLY_INTEGER && 1 == reply->integer) {
			freeReplyObject(reply);
			return 0;
		} else {
			freeReplyObject(reply);
			return -1;
		}
	} else {
		return -1;
	}
}



int Redis::zadd(const string &key,const double score,const string &member)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"zadd %s %f %s",
            key.c_str(),score,member.c_str());
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_INTEGER &&
                1 == reply->integer) {
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}

int Redis::zrange(const string &key,vector<string> &values)
{
    if (_ison == 0) {
        return 0;
    }
    return zrange(key,0,-1,values);
}

int Redis::zrange(const string &key,const int start,const int end,vector<string> &values)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"zrange %s %d %d",
            key.c_str(),start,end);
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i=0;i<reply->elements;i++) {
                if (reply->element[i]->type == REDIS_REPLY_STRING) {
                    values.push_back(reply->element[i]->str);
                }
            }
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }

}

int Redis::zrange_withscores(const string &key,map<string,double> &values)
{
    if (_ison == 0) {
        return 0;
    }
    return zrange_withscores(key,0,-1,values);
}
int Redis::zrange_withscores(const string &key,const int start,const int end,map<string,double> &values)
{
    if (_ison == 0) {
        return 0;
    }
    redisReply *reply = (redisReply *)redisCommand(_conn,"zrange %s %d %d withscores",
            key.c_str(),start,end);
    if (_checkReply(reply) == 0) {
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i=0;i<reply->elements;(++i)++) {
                if (reply->element[i]->type == REDIS_REPLY_STRING) {
                    values[reply->element[i]->str] = atof(reply->element[1+i]->str);
                }
            }
            freeReplyObject(reply);
            return 0;
        } else {
            freeReplyObject(reply);
            return -1;
        }
    } else {
        return -1;
    }
}


//private
int Redis::_checkReply(redisReply *reply)
{
    if (_ison == 0) {
        return 0;
    }
    if (reply == NULL) {
        //判断是否链接出现err
        if (_conn->err != 0) {
            ERROR("Redis %s:%d %d  has err: %s",_ip.c_str(),_port,_db,_conn->errstr);
            return connect();
        }
        return -1;
    } else {
        if (reply->type == REDIS_REPLY_ERROR)
        {
            freeReplyObject(reply);
            return -1;
        } else {
            return 0;
        }
    }
}

