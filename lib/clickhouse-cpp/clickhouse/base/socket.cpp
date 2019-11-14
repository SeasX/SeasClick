#include "socket.h"
#include "singleton.h"

#include <assert.h>
#include <stdexcept>
#include <system_error>
#include <unordered_set>
#include <memory.h>
#include "socket_hook.h"

#if !defined(_win_)
#   include <errno.h>
#   include <netdb.h>
#   include <signal.h>
#   include <unistd.h>
#endif

namespace clickhouse {
namespace {

    class LocalNames : public std::unordered_set<std::string> {
    public:
        LocalNames() {
            emplace("localhost");
            emplace("localhost.localdomain");
            emplace("localhost6");
            emplace("localhost6.localdomain6");
            emplace("::1");
            emplace("127.0.0.1");
        }

        inline bool IsLocalName(const std::string& name) const noexcept {
            return find(name) != end();
        }
    };
}

NetworkAddress::NetworkAddress(const std::string& host, const std::string& port)
    : info_(nullptr)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (!Singleton<LocalNames>()->IsLocalName(host)) {
        // https://linux.die.net/man/3/getaddrinfo
        // If hints.ai_flags includes the AI_ADDRCONFIG flag,
        // then IPv4 addresses are returned in the list pointed to by res only
        // if the local system has at least one IPv4 address configured,
        // and IPv6 addresses are only returned if the local system
        // has at least one IPv6 address configured.
        // The loopback address is not considered for this case
        // as valid as a configured address.
        hints.ai_flags |= AI_ADDRCONFIG;
    }

    const int error = getaddrinfo(host.c_str(), port.c_str(), &hints, &info_);

    if (error) {
        throw std::system_error(errno, std::system_category());
    }
}

NetworkAddress::~NetworkAddress() {
    if (info_) {
        freeaddrinfo(info_);
    }
}

const struct addrinfo* NetworkAddress::Info() const {
    return info_;
}


SocketHolder::SocketHolder()
    : handle_(-1)
{
}

SocketHolder::SocketHolder(SOCKET s)
    : handle_(s)
{
}

SocketHolder::SocketHolder(SocketHolder&& other)
    : handle_(other.handle_)
{
    other.handle_ = -1;
}

SocketHolder::~SocketHolder() {
    Close();
}

void SocketHolder::Close() noexcept {
    if (handle_ != -1) {
#if defined(_win_)
        closesocket(handle_);
#else
        close(handle_);
#endif
        handle_ = -1;
    }
}

bool SocketHolder::Closed() const noexcept {
    return handle_ == -1;
}

SocketHolder& SocketHolder::operator = (SocketHolder&& other) noexcept {
    if (this != &other) {
        Close();

        handle_ = other.handle_;
        other.handle_ = -1;
    }

    return *this;
}

SocketHolder::operator SOCKET () const noexcept {
    return handle_;
}


SocketInput::SocketInput(SOCKET s)
    : s_(s)
{
}

SocketInput::~SocketInput() = default;

size_t SocketInput::DoRead(void* buf, size_t len) {
    const ssize_t ret = ::recv(s_, (char*)buf, (int)len, 0);

    if (ret > 0) {
        return (size_t)ret;
    }

    if (ret == 0) {
        throw std::system_error(
            errno, std::system_category(), "closed"
        );
    }

    throw std::system_error(
        errno, std::system_category(), "can't receive string data"
    );
}


SocketOutput::SocketOutput(SOCKET s)
    : s_(s)
{
}

SocketOutput::~SocketOutput() = default;

void SocketOutput::DoWrite(const void* data, size_t len) {
#if defined (_linux_)
    static const int flags = MSG_NOSIGNAL;
#else
    static const int flags = 0;
#endif

    if (::send(s_, (const char*)data, len, flags) != (int)len) {
        throw std::system_error(
            errno, std::system_category(), "fail to send data"
        );
    }
}


NetrworkInitializer::NetrworkInitializer() {
    struct NetrworkInitializerImpl {
        NetrworkInitializerImpl() {
#if defined (_win_)
            WSADATA data;
            const int result = WSAStartup(MAKEWORD(2, 2), &data);
            if (result) {
                assert(false);
                exit(-1);
            }
#elif defined(_unix_)
            signal(SIGPIPE, SIG_IGN);
#endif
        }
    };


    (void)Singleton<NetrworkInitializerImpl>();
}


SOCKET SocketConnect(const NetworkAddress& addr) {
    for (auto res = addr.Info(); res != nullptr; res = res->ai_next) {
        SOCKET s(socket(res->ai_family, res->ai_socktype, res->ai_protocol));

        if (s == -1) {
            continue;
        }

        if (connect(s, res->ai_addr, (int)res->ai_addrlen)) {
            if (errno == EINPROGRESS ||
                errno == EAGAIN ||
                errno == EWOULDBLOCK)
            {
                pollfd fd;
                fd.fd = s;
                fd.events = POLLOUT;
                int rval = Poll(&fd, 1, 1000);

                if (rval > 0) {
                    int opt;
                    socklen_t len = sizeof(opt);
                    getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&opt, &len);

                    return opt;
                } else {
                    continue;
                }
            }
        } else {
            return s;
        }
    }

    throw std::system_error(
        errno, std::system_category(), "fail to connect"
    );
}


ssize_t Poll(struct pollfd* fds, int nfds, int timeout) noexcept {
#if defined(_win_)
    int rval = WSAPoll(fds, nfds, timeout);
#else
    return poll(fds, nfds, timeout);
#endif
    return -1;
}

}
