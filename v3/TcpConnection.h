#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include "Socket.h"
#include "SocketIO.h"
#include "InetAddress.h"

#include <functional>
#include <memory>

using std::function;
using std::shared_ptr;

class EventLoop;

class TcpConnection
: public std::enable_shared_from_this<TcpConnection>
{
    using TcpConnectionPtr = shared_ptr<TcpConnection>;
    using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;
public:
    TcpConnection(int fd, EventLoop *loop);
    ~TcpConnection();
    void send(const string &msg);

    void sendInLoop(const string &msg);
    string receive();
    
    bool isClosed() const;

    //调试
    string toString();

private:
    InetAddress getLocalAddr();
    InetAddress getPeerAddr();

public:
    void setNewConnectionCallback(const TcpConnectionCallback &cb);
    void setMessageCallback(const TcpConnectionCallback &cb);
    void setCloseCallback(const TcpConnectionCallback &cb);

    void handleNewConnectionCallback();
    void handleMessageCallback();
    void handleCloseCallback();

private:
    EventLoop *_loop;
    SocketIO _sockIO;

    //调试·
    Socket _sock;
    InetAddress _localAddr;
    InetAddress _peerAddr;

    TcpConnectionCallback _onNewConnection;
    TcpConnectionCallback _onMessage;
    TcpConnectionCallback _onClose;
};

#endif
