#include "EchoServer.h"
#include "TcpConnection.h"

#include <iostream>
#include <functional>

using std::cout;
using std::endl;
using std::bind;


MyTask::MyTask(const string &msg, const TcpConnectionPtr & con)
: _msg(msg)
, _con(con)
{

}

void MyTask::process()
{
    _con->sendInLoop(_msg);
}

EchoServer::EchoServer(size_t threadNum, size_t queSize,
                       const string &ip, unsigned short port)
: _pool(threadNum, queSize)
, _server(ip, port)
{
    
}

EchoServer::~EchoServer()
{

}

void EchoServer::start()
{
    _pool.start();
    
    using namespace std::placeholders;
    _server.setAllCallback(bind(&EchoServer::onNewConnection, this, _1),
                           bind(&EchoServer::onMessage, this,_1),
                           bind(&EchoServer::onClose, this, _1));
    _server.start();
}

void EchoServer::stop()
{
    _pool.stop();
    _server.stop();
}

void EchoServer::onNewConnection(const TcpConnectionPtr &con)
{
    cout<< con->toString()<< " has connected!!!"<< endl;
}

void EchoServer::onMessage(const TcpConnectionPtr & con)
{
    string msg = con->receive();
    cout<< ">>recv msg from client: "<< msg<< endl;

    MyTask task(msg, con);
    _pool.addTask(bind(&MyTask::process, task));
}

void EchoServer::onClose(const TcpConnectionPtr & con)
{
    cout<< con->toString()<< "has closed!!!"<< endl;
}
