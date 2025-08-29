#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <vector>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <mutex>

using std::map;
using std::vector;
using std::shared_ptr;
using std::function;
using std::mutex;
using std::lock_guard;

class Acceptor;
class TcpConnection;

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;
using Functor = function<void()>;

class EventLoop
{
public:
    EventLoop(Acceptor &acceptor);
    ~EventLoop();
    void loop();
    void unloop();
    
    void setNewConnectionCallback(TcpConnectionCallback &&cb);
    void setMessageCallback(TcpConnectionCallback &&cb);
    void setCloseCallback(TcpConnectionCallback &&cb);

private:
    void waitEpollFd();
    void handleNewConnection();
    void handleMessage(int fd);
    int createEpollFd();
    void addEpollReadFd(int fd);
    void delEpollReadFd(int fd);

public:
    int createEventFd();
    void handleRead();
    void wakeup();
    void doPendingFunctors();
    void runInLoop(Functor &&cb);


private:
    int _epfd;
    vector<struct epoll_event> _evtList;
    bool _isLooping;
    Acceptor &_acceptor;
    map<int, TcpConnectionPtr> _conns;

    TcpConnectionCallback _onNewConnection;
    TcpConnectionCallback _onMessage;
    TcpConnectionCallback _onClose;

    int _evtfd;
    vector<Functor> _pendings;
    mutex _mutex;

};

#endif
