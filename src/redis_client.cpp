#include <assert.h>
#include <iostream>

#include "redis_client.h"

RedisClient::RedisClient(const std::string ip, uint16_t port, int minConn, int maxConn)
    : m_hostip(ip),
      m_hostport(port),
      m_minConn(minConn),
      m_maxConn(maxConn),
      m_redisPool(new RedisPool(ip, port, minConn, maxConn))
{
    m_redisPool->init();
}

RedisClient::~RedisClient()
{

    if (m_redisPool)
    {
        delete m_redisPool;
        m_redisPool = nullptr;
    }
}

RedisStatus RedisClient::command(const string &cmd)
{
    RedisStatus status;

    auto conn = m_redisPool->getConnection();
    status.m_reply = (redisReply *)redisCommand(conn->m_context, cmd.c_str());
    m_redisPool->freeConnection(conn);

    if (!status.m_reply)
    {
        status.m_ok = false;
        return status;
    }
    //根据不同的type,填充不同返回值信息
    if (status.m_reply->type == REDIS_REPLY_ERROR)
    {
        status.m_ok = false;
        status.m_detailStr = status.m_reply->str;
    }
    else if (status.m_reply->type == REDIS_REPLY_INTEGER)
    {
        status.m_resultInt = status.m_reply->integer;
    }
    else if (status.reply()->type == REDIS_REPLY_STATUS)
    {
        status.m_detailStr = status.m_reply->str;
        if (status.m_detailStr == "OK")
        {
            status.m_resultInt = 1;
        }
    }
    else if (status.reply()->type == REDIS_REPLY_STRING)
    {
    }
    else if (status.reply()->type == REDIS_REPLY_NIL)
    {
        status.m_nil = true;
    }
    else if (status.reply()->type == REDIS_REPLY_ARRAY)
    {
    }

    return status;
}

RedisStatus RedisClient::set(const string &key, const string &value)
{
    string cmd = "SET " + key + " " + value;
    RedisStatus status = command(cmd);
    return status;
}

// get成功则status.ok() == true 并且 status.nil() == false
// get失败可能是 status.ok() == false, 或者 status.nil() == true
// 为了确保一定得到了想要的值, 请判断 if ( status.ok() && ! status.nil() )
RedisStatus RedisClient::get(const string &key, string &value)
{
    string cmd = "GET " + key;
    RedisStatus status = command(cmd);
    value = "";
    if (status.m_reply->type == REDIS_REPLY_STRING)
    {
        value = status.m_reply->str;
    }
    return status;
}

//如果del的值本身就是不存在的,则status.integer() == 0
RedisStatus RedisClient::del(const string &key)
{
    string cmd = "DEL " + key;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::incrby(const string &key, int amount)
{
    string cmd = "INCRBY " + key + " " + std::to_string(amount);
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::rpush(const string &listKey, const string &item)
{
    string cmd = "RPUSH " + listKey + " " + item;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::lpush(const string &listKey, const string &item)
{
    string cmd = "LPUSH " + listKey + " " + item;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::lrange(const string &listKey, int start, int end, vector<string> &items)
{
    string cmd = "LRANGE " + listKey + " " + std::to_string(start) + " " + std::to_string(end);
    return getVector(cmd, items);
}

RedisStatus RedisClient::lindex(const string &listKey, int index, string &item)
{
    string cmd = "LINDEX " + listKey + " " + std::to_string(index);
    RedisStatus status = command(cmd);
    item = "";
    if (status.m_reply->type == REDIS_REPLY_STRING)
    {
        item = status.m_reply->str;
    }
    return status;
}

RedisStatus RedisClient::lpop(const string &listKey)
{
    string cmd = "LPOP " + listKey;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::rpop(const string &listKey)
{
    string cmd = "RPOP " + listKey;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::sadd(const string &setKey, const string &item)
{
    string cmd = "SADD " + setKey + " " + item;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::smembers(const string &setKey, vector<string> &members)
{
    string cmd = "SMEMBERS " + setKey;
    return getVector(cmd, members);
}

RedisStatus RedisClient::sismember(const string &setKey, const string &item, bool &isMember)
{
    string cmd = "SISMEMBER " + setKey + " " + item;
    RedisStatus status = command(cmd);
    if (status.m_reply->type == REDIS_REPLY_INTEGER)
    {
        isMember = static_cast<bool>(status.m_reply->integer);
    }
    else
    {
        isMember = false;
    }
    return status;
}

RedisStatus RedisClient::srem(const string &setKey, const string &item)
{
    string cmd = "SREM " + setKey + " " + item;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::hget(const string &key, const string &field, string &value)
{
    string cmd = "HGET " + key + " " + field;
    RedisStatus status = command(cmd);
    value = "";
    if (status.m_reply->type == REDIS_REPLY_STRING)
    {
        value = status.m_reply->str;
    }
    return status;
}

// redis server对set命令的返回是OK,不返回数字,
// hset, del之类的返回是integer数字
// redis server返回值还挺复杂的,难以记忆, 这里我对他做了简化工作
//任何一个有 set,del 语义的函数,都会返回一个integer,来表示多少个数据被成功的执行操作.
//可以通过status.integer()获得这个值,
//任何一个操作都可以通过status.ok() 来查看是否执行了
RedisStatus RedisClient::hset(const string &key, const string &field, const string &value)
{
    string cmd = "HSET " + key + " " + field + " " + value;
    RedisStatus status = command(cmd);
    return status;
}

//请查看status.integer()
RedisStatus RedisClient::hdel(const string &key, const string &field)
{
    string cmd = "HDEL " + key + " " + field;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::hlen(const string &key, long long &len)
{
    string cmd = "HLEN " + key;
    RedisStatus status = command(cmd);
    if (status.m_reply->type == REDIS_REPLY_INTEGER)
    {
        len = status.m_reply->integer;
    }
    return status;
}

// redis server 对hgetall命令不会返回nil,
//如果真的是空的,也会返回空的array,以及提示信息(empty list or set)
//但是我认为这里应该返回nil提示.所以这里设计的和官方有一点点出入
//当hgetall收到空的array的时候,status.nil()会返回true.
//在本程序里面,任意一个带有get字样或者含有获取值的行为的函数,
//当成功执行但是收到空的内容时候,status.nil()都会返回true
//当执行失败, status.ok()会返回false
//为了确保值一定得到了,请用   if(status.ok() &&  !status.nil() )
//判断执行失败请用   if( !status.ok() || status.nil() ) { return GET_VALUE_FALURE; }
RedisStatus RedisClient::hgetall(const string &key, map<string, string> &values)
{
    values.clear();
    string cmd = "HGETALL " + key;
    RedisStatus status = command(cmd);
    if (status.m_reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 1; i < status.m_reply->elements; i = i + 2)
        {
            redisReply *keyReply = (redisReply *)(status.m_reply->element[i - 1]);
            redisReply *valueReply = (redisReply *)(status.m_reply->element[i]);
            string key = keyReply->str;
            string value = valueReply->str;
            values[key] = value;
        }
    }
    if (values.size() == 0)
    {
        status.m_nil = true;
    }

    return status;
}

// status.integer() == 1 表示yes, 0 表示no
RedisStatus RedisClient::hexists(const string &key, const string &field)
{
    string cmd = "HEXISTS " + key + " " + field;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::hincrby(const string &key, const string &field, int increment)
{
    string cmd = "HINCRBY " + key + " " + field + " " + std::to_string(increment);
    RedisStatus status = command(cmd);
    return status;
}

static const int OP_HKEYS = 1;
static const int OP_HVALS = 2;
RedisStatus RedisClient::getVector(const string &cmd, vector<string> &vec)
{
    vec.clear();
    RedisStatus status = command(cmd);
    if (status.m_reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < status.m_reply->elements; i++)
        {
            redisReply *reply = (redisReply *)(status.m_reply->element[i]);
            string name = reply->str;
            vec.push_back(name);
        }
    }
    if (vec.size() == 0)
    {
        status.m_nil = true;
    }
    return status;
}

//同hgetall
RedisStatus RedisClient::hkeys(const string &key, vector<string> &keys)
{
    return getVector("HKEYS " + key, keys);
}

RedisStatus RedisClient::hvals(const string &key, vector<string> &values)
{
    return getVector("HVALS " + key, values);
}

RedisStatus RedisClient::zadd(const string &key, const int score, const string &member)
{
    string cmd = "ZADD " + key + " " + std::to_string(score) + " " + member;
    RedisStatus status = command(cmd);
    return status;
}

//如果member或者key不存在,返回nil
RedisStatus RedisClient::zscore(const string &key, const string &member, int &score)
{
    string cmd = "ZSCORE " + key + " " + member;
    RedisStatus status = command(cmd);
    if (status.m_reply->type == REDIS_REPLY_STRING)
    {
        string value = status.m_reply->str;
        score = std::stoi(value);
    }
    return status;
}

RedisStatus RedisClient::zrem(const string &key, const string &member)
{
    string cmd = "ZREM " + key + " " + member;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::zcard(const string &key, long long &num)
{
    string cmd = "ZCARD " + key;
    RedisStatus status = command(cmd);
    num = status.m_reply->integer;
    return status;
}

RedisStatus RedisClient::zcount(const string &key, const int min, const int max, long long &count)
{
    string cmd = "ZCOUNT " + key + " " + std::to_string(min) + " " + std::to_string(max);
    RedisStatus status = command(cmd);
    count = status.m_reply->integer;
    return status;
}

RedisStatus RedisClient::zrank(const string &key, const string &member, long long &rank)
{
    string cmd = "ZRANK " + key + " " + member;
    RedisStatus status = command(cmd);
    rank = status.m_reply->integer;
    return status;
}

RedisStatus RedisClient::zincrby(const string &key, int increment, const string &member)
{
    string cmd = "ZINCRBY " + key + " " + std::to_string(increment) + " " + member;
    RedisStatus status = command(cmd);
    return status;
}

RedisStatus RedisClient::zrange(const string &key, const int start, const int stop, vector<string> &members)
{
    string cmd = "ZRANGE " + key + " " + std::to_string(start) + " " + std::to_string(stop);
    return getVector(cmd, members);
}
