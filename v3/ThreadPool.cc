#include "ThreadPool.h"

#include <iostream>

using std::endl;
using std::cout;

ThreadPool::ThreadPool(size_t threadNum, size_t queSize)
: _threadNum(threadNum)
, _queSize(queSize)
, _taskQue(_queSize)
, _isExit(false)
{

}

ThreadPool::~ThreadPool()
{

}

void ThreadPool::start()
{
    for(size_t idx = 0; idx != _threadNum; ++idx)
    {
        _threads.push_back(thread(&ThreadPool::doTask, this));
    }
}

void ThreadPool::stop()
{
    while(!_taskQue.empty())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    _isExit = true;
    _taskQue.wakeup();

    for(auto & th : _threads)
    {
        th.join();
    }
}

void ThreadPool::addTask(Task &&task)
{
    if(task)
    {
        _taskQue.push(std::move(task));
    }
}

Task ThreadPool::getTask()
{
    return _taskQue.pop();
}

void ThreadPool::doTask()
{
    while(!_isExit)
    {
        Task taskcb = getTask();
        if(taskcb)
        {
            taskcb();
        }
        else
        {
            cout<< "nullptr = ptask"<< endl;
        }
    }
}
