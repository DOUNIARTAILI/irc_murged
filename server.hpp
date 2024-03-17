/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drtaili <drtaili@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/17 23:30:45 by drtaili           #+#    #+#             */
/*   Updated: 2024/03/17 23:30:51 by drtaili          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include"Channel.hpp"
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
#include <list>

class Channel;

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
        
    }
};




class Command;

class Server{
private:
    static std::string PASS;
    std::list<Clientx> clients_list;
    static std::vector<pollfd> pfds;
    std::string port;
    static std::string hostname;
    std::vector<std::string>    cmds;        
    std::vector<Channel>        channels;

    int listenerSock; 
public:
    Server();
    Server(const std::string &port, const std::string pass); 
    std::vector<pollfd> &getPfds();

    void *get_in_addr(struct sockaddr *sa);
    int get_listener_socket(void);
    void add_to_pfds(int newfd);
    void del_from_pfds(int fdd);
    std::string getHostname();
    std::string getPASS();
    void handleNewConnection(void);
    void handleClientDataMsg(int fd);
    void runServer();
    void addUser(int fd, std::string ip);
    std::vector<std::string> parcing(Clientx &Guest);
    std::string toupper(std::string &str);
    std::string trim(const std::string &str);
    std::vector<std::string> splitt(const std::string &str, char del);
    void Authenticate(Clientx &Guest);
    void validatePass(std::string &str, Clientx &Guest);
    void validateNick(std::string &str, Clientx &Guest);
    void validateUser(std::string &str, Clientx &Guest);
    int  nickalreadyexist(std::string nick);
    void Register(Clientx &guest);
    std::list<Clientx>::iterator getUserfromClientlist(int fd);
    std::vector<std::string> splitingCmd(const std::string &str, char del);
    std::string userNicknameFromFd(int fd);
    void remove_from_channels(Server &server, int fd, Command &cmd);
};
