#ifndef REDIS_POOL_H
#define REDIS_POOL_H

#include <list>
#include <new>
#include <stdint.h>
#include <unistd.h>
#include <stdint.h>
#include <thread>
#include <functional>
#include <chrono>

#include "common.h"
#include "redis_synch.h"

class RedisPool;

class RedisConnection
{
public:
	using ptr = std::shared_ptr<RedisConnection>;

	RedisConnection(RedisPool *redisPool);

	~RedisConnection();

	bool connect();

	bool ping();

public:
	redisContext *m_context;

private:
	uint64_t m_lastActiveTime;
	RedisPool *m_redisPool;
};

class RedisPool
{
public:
	RedisPool(const std::string ip, uint16_t port, int minConn, int maxConn);

	~RedisPool();

	int init();
	void serverCron();

	int getDBNo() { return dbNo_; }

	const char *getServerIP() { return hostip_.c_str(); }
	int getServerPort() { return hostport_; }

	RedisConnection::ptr getConnection();

	void freeConnection(RedisConnection::ptr conn);

private:
	const std::string hostip_;
	uint16_t hostport_;
	int minConn_;
	int maxConn_;
	int dbNo_ = 0;

	mutable MutexLock mutex_;
	Condition notEmpty_;

	std::list<RedisConnection::ptr> connections_;
	std::thread *cronThread;
	bool quit_;
};
#endif // REDIS_POOL_H
