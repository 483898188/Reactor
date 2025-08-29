#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using std::queue;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::function;

using ElemType = function<void()>;

class TaskQueue
{
public:
    TaskQueue(int capa);
    ~TaskQueue();

    void push(ElemType &&task);
    ElemType pop();

    bool empty() const;
    bool full() const;

    void wakeup();

private:
    size_t _capacity;
    queue<ElemType> _que;
    mutex _mutex;
    condition_variable _notFull;
    condition_variable _notEmpty;
    bool _flag;
};

#endif
