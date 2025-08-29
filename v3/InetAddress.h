#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <arpa/inet.h>
#include <string>

using std::string;

class InetAddress
{
public:
    InetAddress();
    ~InetAddress();
    InetAddress(const struct sockaddr_in &);
    InetAddress(const string &ip, unsigned short port);
    string ip() const;
    unsigned short port() const;
    const struct sockaddr_in *getInetAddrPtr() const;
private:
    struct sockaddr_in _addr;
};

#endif
