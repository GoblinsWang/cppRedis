#include "redis_pool.h"

RedisConnection::RedisConnection(RedisPool *redisPool, int64_t timeout)
	: m_context(NULL),
	  m_lastActiveTime(std::chrono::steady_clock::now()),
	  m_redisPool(redisPool),
	  m_timeout(timeout)
{
}

RedisConnection::~RedisConnection()
{
	if (m_context)
	{
		redisFree(m_context);
		m_context = NULL;
	}
}

bool RedisConnection::connect()
{
	struct timeval timeout = {0, 1000000}; // 1s
	m_context = redisConnectWithTimeout(m_redisPool->getServerIP(), m_redisPool->getServerPort(), timeout);
	if (!m_context || m_context->err)
	{
		std::cout << "connect failed!" << std::endl;
		if (m_context)
		{
			redisFree(m_context);
			m_context = NULL;
		}
		return false;
	}
	return true;
}

bool RedisConnection::ping()
{
	redisReply *reply = static_cast<redisReply *>(redisCommand(m_context, "PING"));
	if (reply == NULL)
		return false;

	freeReplyObject(reply);
	return true;
}

void RedisConnection::updateActiveTime()
{
	if (m_timeout > 0)
		m_lastActiveTime = std::move(std::chrono::steady_clock::now());
}

bool RedisConnection::isExpire()
{
	if (m_timeout > 0)
	{
		auto curTime = std::move(std::chrono::steady_clock::now());
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(curTime - m_lastActiveTime);
		if (duration.count() >= m_timeout)
			return true;
	}
	return false;
}

RedisPool::RedisPool(const std::string ip, uint16_t port, int minConn, int maxConn)
	: m_hostip(ip),
	  m_hostport(port),
	  m_minConn(minConn),
	  m_maxConn(maxConn),
	  m_mutex(),
	  m_notEmpty(m_mutex),
	  m_connQueue(),
	  m_quit(false)
{
}

RedisPool::~RedisPool()
{
	MutexLockGuard lock(m_mutex);

	m_quit = true;
	m_cronThread->join();

	m_connQueue.clear();
	m_minConn = 0;
}

int RedisPool::init()
{
	for (int i = 0; i < m_minConn; i++)
	{
		auto conn = std::make_shared<RedisConnection>(this);
		if (conn->connect())
			m_connQueue.push_back(conn);
	}

	m_cronThread = new std::thread(std::bind(&RedisPool::serverCron, this));

	return 0;
}

// move out the disabled connections
void RedisPool::serverCron()
{
	while (!m_quit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // 10s检测一次
		MutexLockGuard lock(m_mutex);

		for (auto it = m_connQueue.begin(); it != m_connQueue.end(); it++)
		{
			if ((*it)->isExpire() || !(*it)->ping())
				m_connQueue.remove(*it);
		}
	}
}

RedisConnection::ptr RedisPool::getConnection()
{
	MutexLockGuard lock(m_mutex);

	while (m_connQueue.empty())
	{
		if (m_minConn >= m_maxConn)
		{
			m_notEmpty.wait();
		}
		else
		{
			auto conn = std::make_shared<RedisConnection>(this, 120); // 可以为额外新增的连接设置有效期
			if (conn->connect())
			{
				m_connQueue.push_back(conn);
				m_minConn++;
			}
		}
	}

	auto pConn = m_connQueue.front();
	m_connQueue.pop_front();

	return pConn;
}

void RedisPool::freeConnection(RedisConnection::ptr conn)
{
	MutexLockGuard lock(m_mutex);
	auto it = m_connQueue.begin();
	while (it != m_connQueue.end())
	{
		if (*it == conn)
			break;
		it++;
	}

	if (it == m_connQueue.end())
	{
		m_connQueue.push_back(conn);
	}

	m_notEmpty.notify(); //这个场景下，notifyall不能用，释放一次，通知一次才对
}
