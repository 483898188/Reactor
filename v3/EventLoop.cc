#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include <unistd.h>
#include <iostream>
#include <sys/eventfd.h>

using std::cout;
using std::endl;
using std::cerr;

EventLoop::EventLoop(Acceptor &acceptor)
: _epfd(createEpollFd())
, _evtList(1024)
, _isLooping(false)
, _acceptor(acceptor)
, _evtfd(createEventFd())
, _pendings()
, _mutex()
{
    int listenfd = _acceptor.fd();
    addEpollReadFd(listenfd);
    addEpollReadFd(_evtfd);
}

EventLoop::~EventLoop()
{
    close(_epfd);
    close(_evtfd);
}

void EventLoop::loop()
{
    _isLooping = true;
    while(_isLooping)
    {
        waitEpollFd();
    }
}

void EventLoop::unloop()
{
    _isLooping = false;
}

void EventLoop::waitEpollFd()
{
    int nready = 0;
    do
    {
        nready = ::epoll_wait(_epfd, &*_evtList.begin(), _evtList.size(), 3000);
    }while(-1 == nready && errno == EINTR);

    if(-1 == nready)
    {
        cerr<< "-1 == nready"<< endl;
        return;
    }
    else if(0 == nready)
    {
        cout<< ">>epoll_wait timeout"<< endl;
    }
    else
    {
        if(nready == (int)_evtList.size())
        {
            _evtList.resize(2 * nready);
        }
        for(int idx = 0; idx < nready; ++idx)
        {
            int listenfd = _acceptor.fd();
            int fd = _evtList[idx].data.fd;
            if(listenfd == fd)
            {
                handleNewConnection();
            }
            else if(_evtfd == fd)
            {
                handleRead();
                doPendingFunctors();
            }
            else
            {
                handleMessage(fd);
            }
        }
    }
}

void EventLoop::handleNewConnection()
{
    int connfd = _acceptor.accept();
    if(connfd < 0)
    {
        perror("handleNewConnection");
        return ;
    }

    addEpollReadFd(connfd);
    TcpConnectionPtr con(new TcpConnection(connfd, this));

    con->setNewConnectionCallback(_onNewConnection);
    con->setMessageCallback(_onMessage);
    con->setCloseCallback(_onClose);

    _conns.insert({connfd, con});

    con->handleNewConnectionCallback();
}

void EventLoop::handleMessage(int fd)
{
    auto it = _conns.find(fd);
    if(it != _conns.end())
    {
        bool flag = it->second->isClosed();
        if(flag)
        {
            it->second->handleCloseCallback();
            delEpollReadFd(fd);
            _conns.erase(it);
        }
        else
        {
            it ->second->handleMessageCallback();
        }
    }
    else
    {
        cout<< "该链接不存在"<< endl;
        return;
    }
}

int EventLoop::createEpollFd()
{
    int fd = ::epoll_create(1);
    if(fd < 0)
    {
        perror("createEpollFd");
        return -1;
    }
    return fd;
}

void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = ::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt);
    if(ret < 0)
    {
        perror("addEpollReadFd");
        return;
    }
}

void EventLoop::delEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;

    int ret = ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &evt);
    if(ret <0)
    {
        perror("delEpollReadFd");
        return ;
    }
}

void EventLoop::setNewConnectionCallback(TcpConnectionCallback &&cb)
{
    _onNewConnection = std::move(cb);
}

void EventLoop::setMessageCallback(TcpConnectionCallback &&cb)
{
    _onMessage = std::move(cb);
}

void EventLoop::setCloseCallback(TcpConnectionCallback &&cb)
{
    _onClose = std::move(cb);
}

int EventLoop::createEventFd()
{
    int fd = eventfd(10, 0);
    if(fd < 0)
    {
        perror("createEventfd");
        return -1;
    }
    return fd;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    int ret = read(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("handleRead");
        return ;
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    int ret = write(_evtfd, &one, sizeof(uint64_t));
    if(ret != sizeof(uint64_t))
    {
        perror("wakeup");
        return ;
    }
}

void EventLoop::doPendingFunctors()
{
    vector<Functor> tmp;
    {
        lock_guard<mutex> lg(_mutex);
        tmp.swap(_pendings);
    }

    for(auto &cb : tmp)
    {
        cb();
    }
}

void EventLoop::runInLoop(Functor &&cb)
{
    {
        lock_guard<mutex> lg(_mutex);
        _pendings.push_back(std::move(cb));
    }

    wakeup();
}
