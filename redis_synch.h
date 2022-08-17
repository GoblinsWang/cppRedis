#ifndef REDIS_SYNCH_H
#define REDIS_SYNCH_H

#include <assert.h>
#include <pthread.h>
#include <errno.h>

class MutexLock
{
public:
    MutexLock()
    {
        int ret = pthread_mutex_init(&m_mutex, NULL);
        assert(ret == 0);
        (void)ret;
    }

    ~MutexLock()
    {
        int ret = pthread_mutex_destroy(&m_mutex);
        assert(ret == 0);
        (void)ret;
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t *getPthreadMutex() /* non-const */
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

class MutexLockGuard
{
public:
    explicit MutexLockGuard(MutexLock &mutex)
        : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    ~MutexLockGuard()
    {
        m_mutex.unlock();
    }

private:
    MutexLock &m_mutex;
};

class Condition
{
public:
    explicit Condition(MutexLock &mutex)
        : m_mutex(mutex)
    {
        pthread_cond_init(&m_pcond, NULL);
    }

    ~Condition()
    {
        pthread_cond_destroy(&m_pcond);
    }

    void wait()
    {
        pthread_cond_wait(&m_pcond, m_mutex.getPthreadMutex());
    }

    // returns true if time out, false otherwise.
    bool waitForSeconds(int seconds)
    {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += seconds;
        return ETIMEDOUT == pthread_cond_timedwait(&m_pcond, m_mutex.getPthreadMutex(), &abstime);
    }

    void notify()
    {
        pthread_cond_signal(&m_pcond);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&m_pcond);
    }

private:
    MutexLock &m_mutex;
    pthread_cond_t m_pcond;
};

#endif // REDIS_SYNCH_H
