#include "Server.hpp"

/* -------------------------------------------------------------------------- */
/*                            Server constructors                             */
/* -------------------------------------------------------------------------- */

//   Creates a TCP, IPv4, Passive socket
Server::Server(const in_port_t portNum, std::string password) : 
                                password(password), listenPort(portNum)
{
    // used for bind function
    struct sockaddr_in  sockAddr;
    // For setsockopt(), the parameter should be non-zero to enable a boolean option
    int optVal = 1;
    //  creates server socket and store fd
    int sfd = socket(SOCK_DOMAIN, SOCK_STREAM, 0);
    if (sfd == -1) {
        throw std::runtime_error("Error socket() : " + static_cast<std::string>
            (strerror(errno)));
    }
    listenFd = sfd; //  listenFd variable is used to close fd in destructor
    //  Makes all I/O non blocking
    if (fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error fcntl() : " << std::endl;
    }

    // set socket option. SO_REUSEADDR : enables local address reuse
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal))) {
        throw std::runtime_error("Error setsocketopt() : " + 
            static_cast<std::string>(strerror(errno)));
    }
    // bind the listenning socket to a specified port and address
    initSockAddrStruct(&sockAddr, listenPort);
    if (bind(sfd, (sockaddr *)&sockAddr, sizeof(sockAddr)) == -1) {
       throw std::runtime_error("Error setsocketopt() : " + 
            static_cast<std::string>(strerror(errno)));
    }
    // Marks the socket as a passive socket
    if (listen(sfd, BACKLOG) == -1) {
        throw std::runtime_error("Error listen() : " + 
            static_cast<std::string>(strerror(errno)));
    }
    #if defined(LOG)
        serverWelcomeMessage(sockAddr, sfd);
    #endif  // LOG
}

Server::Server(Server &rhs) : password(rhs.password), listenPort(rhs.listenPort),
                            listenFd(rhs.listenFd) {
    *this = rhs;
}

Server& Server::operator=(Server &rhs)
{
    if (this == &rhs)
        return (*this);
    this->listenFd = rhs.listenFd;
    return (*this);
}

Server::~Server()
{
    //  Close server fd
    if (close(listenFd) == -1) {
        std::cerr << "Error close() : " << strerror(errno) << std::endl;
    }
    //  Close all clients fd
    std::map<int, Client>::iterator it = clients.begin();
    for (; it != clients.end(); it++) {
        if (close(it->second.fd) == -1)
            std::cerr << "Error close() : " << strerror(errno) << std::endl;
    }
}

/* -------------------------------------------------------------------------- */
/*                             Setters & Getters                              */
/* -------------------------------------------------------------------------- */

const unsigned short &  Server::getListenPort() const {
    return (this->listenPort);
}

const int &             Server::getListenFd() const {
    return (this->listenFd);
}

const std::string &     Server::getPassword() const {
    return (this->password);
}

/* -------------------------------------------------------------------------- */
/*                               Server Methods                               */
/* -------------------------------------------------------------------------- */

//  Adds the socket fd to the vector<struct pollfd>
void    Server::addNewPollfd(int fd, std::vector<struct pollfd> &fds, nfds_t &nfds)
{
    struct pollfd   newSock;

    memset(&newSock, 0, sizeof(newSock));
    newSock.fd = fd;
    newSock.events = POLLIN;
    fds.push_back(newSock);
    nfds++;
}

int Server::isPollReady(std::vector<struct pollfd> &fds, nfds_t &nfds)
{
    int ret = 0;
    if ((ret = poll(fds.data(), nfds, POLL_TIMEOUT)) == -1) {
        throw std::runtime_error("Error poll() : " + 
            static_cast<std::string>(strerror(errno)));
    } else if (ret > OPEN_MAX) {
        std::cerr 
            << "Warning poll() : max open files per process is reached"
            << std::endl;
        return (0);
    }
    return (ret);
}

//  returns 0 if connection is done successfully, otherwise 0 is returned
bool    Server::addNewClient(std::vector<struct pollfd> &fds, nfds_t *nfds, int &fdsLeft)
{
    Client      clt;
    socklen_t   sockLen = 0; // used for accept()

    //  Accepting incoming connection
    memset(&clt.hints, 0, sizeof(clt.hints));
    sockLen = sizeof(clt.hints);
    if ((clt.fd = accept(listenFd, (sockaddr *)&clt.hints, &sockLen)) == -1) {
        fdsLeft--;
        std::cerr << "Error accept() : " << strerror(errno) << std::endl;
        return (EXIT_FAILURE);
    }
    //  Setting the client fd to NON_BLOCKING mode
    if (fcntl(clt.fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error fcntl() : undefined error" << std::endl;
        fdsLeft--;
        return (EXIT_FAILURE);
    }
    //  Add client fd to the poll of file descriptors
    Server::addNewPollfd(clt.fd, fds, *nfds);

    #if defined(LOG)
        printNewClientInfoOnServerSide(clt.hints);
        clientWelcomeMessage(clt.fd);
    #endif  // LOG

    //  Push the new client to map<> in the server
    clients.insert(std::pair<int, Client>(clt.fd, clt));
    // Decrement number of fds returned by poll()
    fdsLeft--;
    return (EXIT_SUCCESS);
}

//  Accepts incoming connections
void            Server::handleIncomingConnections()
{
    std::map<int, std::string>  map; // used as a buff when "\n" is not found
    std::vector<struct pollfd>  fds; // holds all connection accepted
    std::string                 buff; // for recv()
    ssize_t                     bytes; // for recv()
    nfds_t                      nfds = 0; // size of fds vector

    buff.resize(1024);

    //  add the server socket fd to the pollfd vector
    Server::addNewPollfd(listenFd, fds, nfds);

    int fdsLeft = 0;
    while (1) {
        if ((fdsLeft = isPollReady(fds, nfds))) {

            //  Checks if there is a new connection to accept
            if (isNewConnection(fds[0], listenFd)) {
                if ( addNewClient(fds, &nfds, fdsLeft)) {
                    // continue ;
                }
            }

            for (unsigned long i = 1; (i < nfds) && (fdsLeft > 0); i++) {
                // if (isReadable(fds[i], listenFd) && isRegistred())
                if (isReadable(fds[i])) {
                    //  Read from client file descriptor
                    memset((void *)buff.data(), 0, sizeof(buff));
                    bytes = recv(fds[i].fd, (void *)buff.data(), sizeof(buff), 0);
                    if (bytes == -1) {
                        std::cout << "Error recv(): an error occured" << std::endl;
                        std::cout.flush();
                    } else if (bytes > 0) {
                        ReadIncomingMsg(buff, map, fds, i);
                    }
                    fdsLeft--;

                } else if (isError(fds[i].revents, fds[i].fd, listenFd)) {

                    #if defined(LOG)
                        std::cout << geTime() << " | client disconnected " << std::endl;
                    #endif // LOG

                    //  Close client file descriptor
                    close(fds[i].fd);
                    // remove client from list clients that may have a buff not complete
                    map.erase(fds[i].fd);
                    // delete client from vector given to poll()
                    fds.erase(fds.begin() + i);
                    // delete client from list of clients in server
                    clients.erase(clients[i].fd);
                    // decrement number of file descriptors in pollfd
                    nfds--;
                    // decrement number of file descriptors handeled
                    fdsLeft--;
                }
            }
        }
    }
}
