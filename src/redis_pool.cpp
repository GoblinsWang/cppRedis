#include "redis_pool.h"

RedisConnection::RedisConnection(RedisPool *redisPool)
	: m_context(NULL),
	  m_lastActiveTime(time(NULL)),
	  m_redisPool(redisPool)
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

RedisPool::RedisPool(const std::string ip, uint16_t port, int minConn, int maxConn)
	: hostip_(ip),
	  hostport_(port),
	  minConn_(minConn),
	  maxConn_(maxConn),
	  mutex_(),
	  notEmpty_(mutex_),
	  connections_(),
	  quit_(false)
{
}

RedisPool::~RedisPool()
{
	MutexLockGuard lock(mutex_);

	quit_ = true;
	cronThread->join();

	connections_.clear();
	minConn_ = 0;
}

int RedisPool::init()
{
	for (int i = 0; i < minConn_; i++)
	{
		auto conn = std::make_shared<RedisConnection>(this);
		if (conn->connect())
			connections_.push_back(conn);
	}

	cronThread = new std::thread(std::bind(&RedisPool::serverCron, this));

	return 0;
}

// move out the disabled connections
void RedisPool::serverCron()
{
	while (!quit_)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		MutexLockGuard lock(mutex_);

		for (auto it = connections_.begin(); it != connections_.end(); it++)
		{
			if (!(*it)->ping())
				connections_.remove(*it);
		}
	}
}

RedisConnection::ptr RedisPool::getConnection()
{
	MutexLockGuard lock(mutex_);

	while (connections_.empty())
	{
		if (minConn_ >= maxConn_)
		{
			notEmpty_.wait();
		}
		else
		{
			auto conn = std::make_shared<RedisConnection>(this);
			if (conn->connect())
			{
				connections_.push_back(conn);
				minConn_++;
			}
		}
	}

	auto pConn = connections_.front();
	connections_.pop_front();

	return pConn;
}

void RedisPool::freeConnection(RedisConnection::ptr conn)
{
	MutexLockGuard lock(mutex_);
	auto it = connections_.begin();
	while (it != connections_.end())
	{
		if (*it == conn)
			break;
		it++;
	}

	if (it == connections_.end())
	{
		connections_.push_back(conn);
	}

	notEmpty_.notify(); //这个场景下，notifyall不能用，释放一次，通知一次才对
}
