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
#include "../printing_macros.h"

void getlasterror(int code)
{
    if (code < 0)
    {
        switch (errno)
        {
        case 10035:
        case 997:
            code = 0;
            break;
        case 10051:
        case 10065:
            code = -5;
            break;
        case 10057:
            code = -2;
            break;
        case 10061:
            code = -6;
            break;
        case 10022:
            code = -11;
            break;
        default:
            code = errno != 10054 ? -7 : -13;
            break;
        }
    }

    return code;
}

void dnslookup(const char *hostname, void *unk, void **unk_sema, unsigned int *unk_addr)
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

ssize_t send_wrapper(int fd, const void *buf, size_t n, int flags)
{
    PROCESS_MESSAGE__DEBUG("Sending %d bytes to socket 0x%X:", n, fd);
    PROCESS_BUFMESSAGE__DEBUG(buf, n);

    return send(fd, buf, n, flags);
}

ssize_t recv_wrapper(int fd, void *buf, size_t n, int flags)
{
    int sz = recv(fd, buf, n, flags);

    PROCESS_MESSAGE__DEBUG("Received %d bytes from socket 0x%X:", sz, fd);
    PROCESS_BUFMESSAGE__DEBUG(buf, sz);

    return sz;
}

// This handler is an exact copy of CnC-Online code (from cnconline.dll) with minor changes
extern struct hostent *gethostbyname_wrapper(const char *name)
{
    const char* res = name;

	if (!strcmp(name, "servserv.generals.ea.com") ||
		!strcmp(name, "na.llnet.eadownloads.ea.com"))
		res = "http.server.cnc-online.net";

	else if (!strcmp(name, "bfme.fesl.ea.com") ||
		!strcmp(name, "bfme2.fesl.ea.com") ||
		!strcmp(name, "bfme2-ep1-pc.fesl.ea.com") ||
		!strcmp(name, "cnc3-pc.fesl.ea.com") ||
		!strcmp(name, "cnc3-ep1-pc.fesl.ea.com") ||
		!strcmp(name, "cncra3-pc.fesl.ea.com"))
		res = "login.server.cnc-online.net";

	else if (!strcmp(name, "gpcm.gamespy.com"))
		res = "gpcm.server.cnc-online.net";

	else if (!strcmp(name, "peerchat.gamespy.com"))
		res = "peerchat.server.cnc-online.net";

	else if (!strcmp(name, "lotrbme.available.gamespy.com") ||
		!strcmp(name, "lotrbme.master.gamespy.com") ||
		!strcmp(name, "lotrbme.ms13.gamespy.com") ||
		!strcmp(name, "lotrbme2r.available.gamespy.com") ||
		!strcmp(name, "lotrbme2r.master.gamespy.com") ||
		!strcmp(name, "lotrbme2r.ms9.gamespy.com") ||
		!strcmp(name, "ccgenerals.ms19.gamespy.com") ||
		!strcmp(name, "ccgenzh.ms6.gamespy.com") ||
		!strcmp(name, "cc3tibwars.available.gamespy.com") ||
		!strcmp(name, "cc3tibwars.master.gamespy.com") ||
		!strcmp(name, "cc3tibwars.ms17.gamespy.com") ||
		!strcmp(name, "cc3xp1.available.gamespy.com") ||
		!strcmp(name, "cc3xp1.master.gamespy.com") ||
		!strcmp(name, "cc3xp1.ms18.gamespy.com") ||
		!strcmp(name, "redalert3pc.available.gamespy.com") ||
		!strcmp(name, "redalert3pc.master.gamespy.com") ||
		!strcmp(name, "redalert3pc.ms1.gamespy.com") ||
		!strcmp(name, "master.gamespy.com"))
		res = "master.server.cnc-online.net";

	else if (!strcmp(name, "redalert3pc.natneg1.gamespy.com") ||
		!strcmp(name, "redalert3pc.natneg2.gamespy.com") ||
		!strcmp(name, "redalert3pc.natneg3.gamespy.com"))
		res = "natneg.server.cnc-online.net";

	else if (!strcmp(name, "lotrbme.gamestats.gamespy.com") ||
		!strcmp(name, "lotrbme2r.gamestats.gamespy.com") ||
		!strcmp(name, "gamestats.gamespy.com"))
		res = "gamestats.server.cnc-online.net";

	else if (!strcmp(name, "lotrbfme.arenasdk.gamespy.com") ||
		!strcmp(name, "arenasdk.gamespy.com") ||
		!strcmp(name, "launch.gamespyarcade.com") ||
		!strcmp(name, "www.gamespy.com") ||
		!strcmp(name, "ingamead.gamespy.com"))
		res = "server.cnc-online.net";

    if (res != name)
        PROCESS_MESSAGE__DEBUG("Changed host \"%s\" to \"%s\"", name, res);

	return gethostbyname(res);
}

void init_socket_api_callbacks()
{
    struct PlatformSocketAPICallbacks *callbacks = GetPlatformSocketAPICallbacksInstance();

    callbacks->accept = accept;
    callbacks->bind = bind;
    callbacks->connect = connect;
    callbacks->gethostbyaddr = gethostbyaddr;
    callbacks->gethostbyname = gethostbyname_wrapper;
    callbacks->dnslookup = dnslookup;
    callbacks->getpeername = getpeername;
    callbacks->getsockopt = getsockopt;
    callbacks->listen = listen;
    callbacks->recv = recv_wrapper;
    callbacks->recvfrom = recvfrom;
    callbacks->send = send_wrapper;
    callbacks->sendto = sendto;
    callbacks->setsockopt = setsockopt_wrapper;
    callbacks->shutdown = shutdown;
    callbacks->socket = socket;
    callbacks->close = close;
    callbacks->poll = poll;
    callbacks->getlasterror = getlasterror;
}