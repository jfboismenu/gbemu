#pragma once

#include <pthread.h>

namespace gbemu {

class Mutex
{
public:
    Mutex();
    ~Mutex();
private:
    friend class MutexGuard;
    pthread_mutex_t _mutex;
};

class MutexGuard
{
public:
    MutexGuard(Mutex& mutex);
    ~MutexGuard();
private:
    Mutex& _mutex;
};

}
