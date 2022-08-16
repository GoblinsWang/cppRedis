#ifndef REDISCLIENT_H
#define REDISCLIENT_H
#include "common.h"
#include "redis_status.h"
#include "redis_pool.h"

using std::map;
using std::string;
using std::vector;

class RedisClient
{
public:
    RedisClient(const std::string ip, uint16_t port, int minConn = 1, int maxConn = 8);

    ~RedisClient();

    RedisStatus set(const string &key, const string &value);
    RedisStatus get(const string &key, string &value);
    RedisStatus del(const string &key);
    RedisStatus incrby(const string &key, int amount);

    RedisStatus rpush(const string &listKey, const string &item);
    RedisStatus lpush(const string &listKey, const string &item);
    RedisStatus lrange(const string &listKey, int start, int end, vector<string> &items);
    RedisStatus lindex(const string &listKey, int index, string &item);
    RedisStatus lpop(const string &listKey);
    RedisStatus rpop(const string &listKey);

    RedisStatus sadd(const string &setKey, const string &item);
    RedisStatus smembers(const string &setKey, vector<string> &members);
    RedisStatus sismember(const string &setKey, const string &item, bool &isMember);
    RedisStatus srem(const string &setKey, const string &item);

    RedisStatus hget(const string &key, const string &field, string &value);
    RedisStatus hset(const string &key, const string &field, const string &value);
    RedisStatus hdel(const string &key, const string &field);
    RedisStatus hlen(const string &key, long long &len);
    RedisStatus hgetall(const string &key, map<string, string> &values);
    RedisStatus hexists(const string &key, const string &field);
    RedisStatus hincrby(const string &key, const string &field, int increment);
    RedisStatus hkeys(const string &key, vector<string> &keys);
    RedisStatus hvals(const string &key, vector<string> &values);

    RedisStatus zadd(const string &key, const int score, const string &member);
    RedisStatus zscore(const string &key, const string &member, int &score);
    RedisStatus zrem(const string &key, const string &member);
    RedisStatus zcard(const string &key, long long &num);
    RedisStatus zcount(const string &key, const int min, const int max, long long &count);
    RedisStatus zrank(const string &key, const string &member, long long &rank);
    RedisStatus zincrby(const string &key, int increment, const string &member);
    RedisStatus zrange(const string &key, const int start, const int stop, vector<string> &members);

private:
    RedisStatus command(const string &cmd);

private:
    RedisStatus getVector(const string &cmd, vector<string> &vec);

    const std::string m_hostip;
    uint16_t m_hostport;
    int m_minConn;
    int m_maxConn;

    RedisPool *m_redisPool;
};

#endif // REDISCLIENT_H
