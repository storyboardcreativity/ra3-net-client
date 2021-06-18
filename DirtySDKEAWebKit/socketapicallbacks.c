#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "platformsocketapi.h"
#include "system_types.h"

void getlasterror()
{
    return errno;
}

void dnslookup(const char* hostname, void* unk, void** unk_sema, unsigned int* unk_addr)
{
    printf("dnslookup() is not implemented! Exiting.\n");
    exit(-1);
}

int setsockopt_wrapper(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    // This fix is used for linux system without correct SO_NBIO
    if (optname == SO_NBIO)
    {
        int flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        return 0;
    }

    return setsockopt(fd, level, optname, optval, optlen);
}

void init_socket_api_callbacks()
{
    struct PlatformSocketAPICallbacks *callbacks = GetPlatformSocketAPICallbacksInstance();

    callbacks->accept = accept;
    callbacks->bind = bind;
    callbacks->connect = connect;
    callbacks->gethostbyaddr = gethostbyaddr;
    callbacks->gethostbyname = gethostbyname;
    callbacks->dnslookup = dnslookup;
    callbacks->getpeername = getpeername;
    callbacks->getsockopt = getsockopt;
    callbacks->listen = listen;
    callbacks->recv = recv;
    callbacks->recvfrom = recvfrom;
    callbacks->send = send;
    callbacks->sendto = sendto;
    callbacks->setsockopt = setsockopt_wrapper;
    callbacks->shutdown = shutdown;
    callbacks->socket = socket;
    callbacks->close = close;
    callbacks->poll = poll;
    callbacks->getlasterror = getlasterror;
}