#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "TaskQueue.h"

#include <vector>
#include <thread>
#include <functional>

using std::vector;
using std::thread;
using std::function;

using Task = function<void()>;

class ThreadPool
{
public:
    ThreadPool(size_t threadNum, size_t queSize);
    ~ThreadPool();

    void start();
    void stop();

    void addTask(Task &&task);

private:
    Task getTask();
    void doTask();

private:
    size_t _threadNum;
    vector<thread> _threads;
    size_t _queSize;
    TaskQueue _taskQue;
    bool _isExit;
};

#endif
