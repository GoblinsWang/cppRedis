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

namespace cppredis
{
	class RedisPool;

	class RedisConnection
	{
	public:
		using ptr = std::shared_ptr<RedisConnection>;

		RedisConnection(RedisPool *redisPool, int64_t timeout = 0);

		~RedisConnection();

		bool connect();

		bool ping();

		void updateActiveTime();

		bool isExpire();

	public:
		redisContext *m_context;

	private:
		std::chrono::steady_clock::time_point m_lastActiveTime;

		RedisPool *m_redisPool;

		int64_t m_timeout; // 默认该连接失效时间，为0时不失效
	};

	class RedisPool
	{
	public:
		RedisPool(const std::string ip, uint16_t port, int minConn, int maxConn);

		~RedisPool();

		int init();

		void serverCron();

		const char *getServerIP()
		{
			return m_hostip.c_str();
		}

		int getServerPort()
		{
			return m_hostport;
		}

		RedisConnection::ptr getConnection();

		void freeConnection(RedisConnection::ptr conn);

	private:
		const std::string m_hostip;
		uint16_t m_hostport;
		int m_minConn;
		int m_maxConn;

		mutable MutexLock m_mutex;
		Condition m_notEmpty;

		std::list<RedisConnection::ptr> m_connQueue;
		std::thread *m_cronThread; //用于定时检测连接情况的线程
		bool m_quit;
	};

} // namespace cppredis
#endif // REDIS_POOL_H
