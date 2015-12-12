#include "mutex.h"

namespace gbemu {


Mutex::Mutex()
{
    pthread_mutex_init(&_mutex, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&_mutex);
}

MutexGuard::MutexGuard(Mutex& mutex) : _mutex(mutex)
{
    pthread_mutex_lock(&_mutex._mutex);
}

MutexGuard::~MutexGuard()
{
    pthread_mutex_unlock(&_mutex._mutex);
}

}
