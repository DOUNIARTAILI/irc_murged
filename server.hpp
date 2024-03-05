#pragma once
// #include<iostream>
// #include<sys/socket.h>
// #include<sys/types.h>
// #include <netdb.h>
// #include <unistd.h>
// #include<vector>
// #include <poll.h>
// #include <fcntl.h>
#define backlog 7
// #include<string>
#include"Channel.hpp"
// struct addrinfo {
//     int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
//     int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
//     int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
//     int              ai_protocol;  // use 0 for "any"
//     size_t           ai_addrlen;   // size of ai_addr in bytes
//     struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
//     char            *ai_canonname; // full canonical hostname
//     struct sockaddr_in  *ai_addr_in;
//     addrinfo *ai_next;      // linked list, next node
// };

// struct pollfd {
//     int fd;         // the socket descriptor
//     short events;   // bitmap of events we're interested in
//     short revents;  // when poll() returns, bitmap of events that occurred
// };

class Channel;

// class Clientx
// {
//     public:
//     std::string username;
//     std::string nickname;
//     std::string realname;
//     std::string ip;
//     std::string password;
//     int c_fd;
//     Clientx()
//     {}
//     Clientx(const std::string &nick)
//     {
//         nickname = nick;
//     }
//     bool operator==(const Clientx& other)
//     {
//         if (nickname == other.nickname)
//             return true;
//         return false;
//     }
// };
class Clientx
{
public:
    std::string nickname;
    std::string username;
    std::string realname;
    std::string cmd;
    int c_fd;
    bool pass;
    bool connected;
    // struct sockaddr_in cl;
    char ip[100];

    Clientx()
    {
        nickname = "";
        username = "";
        pass = false;
        connected = false;
        c_fd = -1;
    }
    Clientx(const std::string &s)
    {
        nickname = s;
    }
    Clientx(int fd, std::string ip)
    {
        c_fd = fd;
        strcpy(this->ip, ip.c_str());
    }
    void setUsername(const std::string &value)
    {
        this->username = value;
    }
    void setNickname(const std::string &value)
    {
        this->nickname = value;
    }

    void setRealname(const std::string &value)
    {
        this->realname = value;
    }
    bool    operator==(const Clientx &c) const
    {
        if (this->nickname == c.nickname)
            return true;
        return false;
    }
     bool operator()(const Clientx& client) const {
        return client.nickname == this->nickname;
    }
    ~Clientx(){
        // delete ipaddr;
    }
};
// class Server
// {
//     public:
//         Server(std::string const &port, std::string const &pass);
//         ~Server();
//         int _socket;
//         addrinfo *_addr;
//         int s_fd;
//         struct pollfd fds[7];
//         std::vector<Clientx> clients;
//         std::vector<Channel> chan;
//         std::string mybuffer;
//         void authenticate(int idx);
// };

// class Server{
//     public:
//         /* Attributes */
//         time_t pingTime;
//         int             serverFd;
//         // std::string     sever_ip;
//         std::string     cmd;
//         std::string     password;
//         std::string     port;
//         struct addrinfo l;
//         struct addrinfo *p;
//         struct pollfd   *clientFd;
//         int botIn;
//         char ip[100];
//         std::vector<Channel>        channels;
//         std::vector<Clientx>   clientsList;
//         std::vector<std::string>    cmds;        
//         std::vector<struct pollfd>  clientFdlist;  
//         /* Methods */ 
        
//         Server(std::string const &port, std::string const &pass);
//         ~Server();
//         void    createSocket();
//         void    bindingSocket();
//         void    socketOnListening();
//         void    acceptingClient();
//         void    readingFromClient();
//         void parseCmd(int i);
//         void    auth(int i);
//         std::string check_num(char c, int index);
//         // void    execute(Command &cmd, Client &user);
//         std::string    checkExnick(std::string nick);
// };


#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>




class Data{

};
// class Data;

class Server{
private:
    // Data *data;
    static std::string PASS;
    std::vector<Clientx>   clientsList;
    std::string port;
    static std::string hostname;
    static std::vector<pollfd> pfds;
    std::vector<std::string>    cmds;        
    std::vector<Channel>        channels;
    // std::vector<std::string> commandparsed;

    int listenerSock; 
public:
    Server(const std::string &port, const std::string pass); 
    std::vector<pollfd> &getPfds();

    void *get_in_addr(struct sockaddr *sa);
    int get_listener_socket(void);
    void add_to_pfds(int newfd);
    void del_from_pfds(int fdd);
    std::string getHostname();
    std::string getPASS();
    void newConnection(struct pollfd *pfds, int newfd, int fd_count, int fd_size, struct sockaddr_storage remoteaddr, char *remoteIP);
    void newMsg();
    void handleNewConnection();
    void handleClientDataMsg(int index);
    void parseCmd(int i);
    void    auth(int i);
    std::string check_num(char c, int index);
    std::string    checkExnick(std::string nick);
    void runServer();
    void addUser(int fd, std::string ip);
    void parcing(int i);
    std::string toupper(std::string &str);
    std::string trim(const std::string &str);
    std::vector<std::string> splitt(const std::string &str, char del);
    void Authenticate(int fd);
    void validatePass(std::string &str, int i);
    void validateNick(std::string &str, int i);
    void validateUser(std::string &str, int i);
    int  nickalreadyexist(std::string nick);
    void Register(int i);
    void fdHandler(int i);
    void clearVec(std::vector<std::string> &vec);
    int getUserfromClientlist(int fd);
};
