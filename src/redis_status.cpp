#include "redis_status.h"

RedisStatus::RedisStatus() : m_reply(NULL), m_ok(true), m_nil(false), m_detailStr(), m_resultInt(0)
{
}

bool RedisStatus::ok() const
{
    return m_ok;
}

bool RedisStatus::nil() const
{
    return m_nil;
}

redisReply *RedisStatus::reply() const
{
    return m_reply;
}

string RedisStatus::detail() const
{
    return m_detailStr;
}

long long RedisStatus::integer() const
{
    return m_resultInt;
}
