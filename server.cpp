#include"server.hpp"
#include"Commands.hpp"
#include"Channel.hpp"
#include <fcntl.h>
class Client;
class Channel;

Server::Server(){

}
void Server::addUser(int fd, std::string ip)
{
    Clientx user(fd, ip);
    this->clients_list.push_back(user);
    // clientsList.push_back(user);
}

std::string Server::toupper(std::string &str)
{
    size_t i = 0;
    while (str[i])
    {
        str[i] = std::toupper(str[i]);
        i++;
    }
    return str;
}

std::string Server::trim(const std::string &str)
{
    size_t leftPos = str.find_first_not_of(" \t\n\r\v\f");
    size_t rightPos = str.find_last_not_of(" \t\n\r\v\f");

    if (leftPos == std::string::npos || rightPos == std::string::npos)
        return std::string("");  // Empty string if all characters are whitespace

    return str.substr(leftPos, rightPos - leftPos + 1);
}

std::vector<std::string>  Server::parcing(Clientx &user){
    std::vector<std::string> commandparsed;
    std::string word;
    std::istringstream iss(user.cmd);
    iss >> word;
    std::string firstarg = word;


    user.cmd = trim(user.cmd);
    std::istringstream ss(user.cmd);
    size_t npos = user.cmd.find_first_of(" \t\n\r\v\f");
    if (npos == std::string::npos){
        commandparsed.push_back(user.cmd);
    }
    else{
        commandparsed.push_back(user.cmd.substr(0, npos));
        commandparsed.push_back(trim(user.cmd.substr(npos)));
    }
    return commandparsed;
}

int  Server::nickalreadyexist(std::string nick)
{
    std::list<Clientx>::iterator i = this->clients_list.begin();
    while (i != this->clients_list.end())
    {
        if (i->nickname == nick)
            return 0;
        ++i;
    }
    return 1;
}

std::string Server::userNicknameFromFd(int fd)
{
    std::list<Clientx>::iterator it;
    for(it =  clients_list.begin(); it != clients_list.end(); it++)
    {
        if((*it).c_fd == fd)
            return (*it).nickname;
    }
    return "";
}

std::vector<pollfd> Server::pfds = std::vector<pollfd>();
std::string Server::hostname = "";
std::string Server::PASS = "";


Server::Server(const std::string &port, const std::string pass)
{
    Server::PASS = pass;
    Server::port = port;

    char hostbuffer[256];
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1)
    {
        perror("gethostname");
        exit(1);
    }

    Server::hostname = hostbuffer;
} 

std::vector<pollfd> &Server::getPfds()
{
    return Server::pfds;
}

std::string Server::getHostname()
{
    return Server::hostname;
}

std::string Server::getPASS()
{
    return Server::PASS;
}

// Get sockaddr, IPv4 or IPv6:
void *Server::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int Server::get_listener_socket()
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, this->port.c_str(), &hints, &ai)) != 0) {
        std::cerr << "selectserver: " << gai_strerror(rv) << std::endl;
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
            perror("setsockopt");
            continue;
        }

        // we need to set the socket to non-blocking
        if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
            perror("fcntl");
            continue;
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

void Server::add_to_pfds(int newfd)
{
    struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pfds.push_back(pfd);
}

void Server::del_from_pfds(int fdd)
{
    std::vector<struct pollfd>::iterator it;
    for(std::list<Clientx>::iterator i = clients_list.begin(); i != clients_list.end(); i++)
    {
        if((*i).c_fd == fdd)
            clients_list.erase(i);
    }
    for (it = pfds.begin(); it != pfds.end(); it++) {
        if ((*it).fd == fdd){
            pfds.erase(it);
            break;
        }
    }
    if (it == pfds.end())
        return;

    size_t i = 0;
    while (i < pfds.size()){
        std::cout << "pfd after deletion " << pfds[i].fd << std::endl;
        i++;
    }
}

void Server::handleNewConnection(void)
{
    Clientx client;
    int newfdclient; // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr;// Client address
    socklen_t addrlen;
    char remoteIP[INET_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    newfdclient = accept(this->listenerSock, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfdclient == -1) {
        perror("accept");
    } else {
        add_to_pfds(newfdclient);
        std::cout << "pollserver: new connection from " << 
        inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET_ADDRSTRLEN) << 
        " on socket " << newfdclient << std::endl;
                
        addUser(newfdclient, inet_ntop(remoteaddr.ss_family, 
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET_ADDRSTRLEN));
    }
}

std::vector<std::string> Server::splitt(const std::string &str, char del)
{
    std::istringstream iss(str);
    std::string arg;
    std::vector<std::string> v;

    while (std::getline(iss, arg, del))
    {
        if (del == ' ')
            arg = trim(arg);
        if (!arg.empty())
            v.push_back(arg);
    }
    return v;
}

void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server);

std::vector<std::string> Server::splitingCmd(const std::string &str, char del) {
    std::vector<std::string> v;
    size_t start = 0;
    size_t pos = 0;
    while ((pos = str.find(del, start)) != std::string::npos) {
        std::string arg = str.substr(start, pos - start);
        if (!arg.empty())
            v.push_back(arg);
        start = pos + 1; // Move past the delimiter
    }

    // Handle the last substring (no delimiter after it)
    if (start < str.size()) { // Ensure there is still content to extract
        std::string  lastArg = str.substr(start);
        if (!lastArg.empty())
            v.push_back(lastArg);
    }
    return v;
}

void Server::remove_from_channels(Server &server, int fd, Command &cmd)
{

    (void)server;
    std::vector<Channel>::iterator iter = channels.begin();

    if(iter == channels.end())
        std::cout << "channels khawin \n";

    puts("pfff");
    for(; iter != channels.end();iter++)
    {
        puts("dd");
        std::string nickname = userNicknameFromFd(fd);
        // if(nickname.empty())
        //     continue;
        if((*iter).is_user(nickname))
        {
            (*iter).remove_user(nickname);
            std::cout << nickname << " removed from " << (*iter).name << std::endl;
            std::list<Clientx>::iterator  client_it = getUserfromClientlist(fd);
            for(std::vector<Clientx *>::iterator it = (*iter).user_list.begin(); it != (*iter).user_list.end(); it++)
            {
                std::string msg = QUIT_MSG(nickname, (*client_it).username, (*client_it).ip, cmd.comment);
                if ((*it)->c_fd != -1)
                {
                    if (send((*it)->c_fd, msg.c_str(), msg.size(), 0) == -1)
                    {
                        // server.del_from_pfds(i->c_fd);
                        // for()
                        perror("send 1");
                    }
                }
            }
            for(std::list<Clientx>::iterator i = clients_list.begin(); i != clients_list.end(); i++)
            {
                if((*i).c_fd == fd)
                    clients_list.erase(i);
            }
        }
    }
}

void Server::handleClientDataMsg(int fd)
{
    const int buffer_len = 1024;
    char buf[buffer_len];
    int nbytes = recv(fd, buf, buffer_len, 0);
    //std::cout << "buffer |" << buf  << "|" << std::endl;
    Server server = *this;

    std::list<Clientx>::iterator it = getUserfromClientlist(fd);
    // if (it != this->clients_list.end())
    // {
    //     puts("hh");
    //     it->cmd += buf;
    //     it->c_fd = fd;
    // }

    for(size_t x = 0; x < channels.size(); x++)
    {
        if (channels[x].user_list.size() == 0)
            channels.erase(channels.begin() + x);
    }

    Command cmd;

    if (nbytes <= 0)
    {
        // Got error or connection closed by client
        if (nbytes == 0) {
        while(it != clients_list.end())
        {
            if (it->c_fd == fd)
            {
                std::cout<<"before killing clients !"<<std::endl;
                // printpfds(pfds);
                //quit(this->channels,cmd, *it, this->clients_list, server);
                std::cout << "pollserver: socket " << fd << " hung up" << std::endl;
                break;
            }
            ++it;
        }
            // quit(this->channels,cmd, *it, this->clients_list, server);
            // std::cout << "pollserver: socket " << sender_fd << " hung up" << std::endl;
        } else {
            perror("recv");
        }
        remove_from_channels(server, fd, cmd);
        close(fd);
        del_from_pfds(fd);
    }
    else
    {
        buf[nbytes] = 0;

        std::string message(buf);
        size_t pos = message.find_last_of("\n");
        if (pos != std::string::npos)
        {
            it->cmd += message.substr(0, pos);
            std::vector<std::string> v;
            v = splitingCmd(it->cmd, '\n');
            for(size_t i = 0; i < v.size(); i++)
            {
                it->cmd = v[i];
                if (it->connected == true)
                    cmd.getcommand(it->cmd, this->channels, cmd, *it, this->clients_list, server);
                else
                    this->Authenticate(*it);
            }
            it->cmd = message.substr(pos + 1);
        }
        else
        {
            it->cmd += message;
        }
    }
}


void Server::runServer()
{
    // Set up and get a listening socket
    this->listenerSock = get_listener_socket();
    if (this->listenerSock == -1) {
        std::cerr << "error getting listening socket" << std::endl;
        exit(1);
    }
    // Add the listener to set
    add_to_pfds(this->listenerSock);// Report ready to read on incoming connection
    // Main loop
    for(;;) {
        signal(SIGPIPE, SIG_IGN);
        int poll_count = poll(pfds.data(), pfds.size(), 0); // kant -1

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for(size_t i = 0; i < pfds.size(); i++)
        {
            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN)
            { // We got one!!

                if (pfds[i].fd == this->listenerSock)
                {
                    // If this->listenerSock is ready to read, handle new connection
                    handleNewConnection();
                }
                else
                {
                    handleClientDataMsg(pfds[i].fd);
                }
            }
        }
    } 
}

void Server::validatePass(std::string &str, Clientx &user){
    // std::cout << "pass of server " << this->PASS << std::endl;
    // std::cout << "pass of client " << str<< std::endl;
    if (user.connected == true){
        std::string rp = ERR_ALREADYREGISTERED(Server::hostname);
        if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }
    if (str == this->PASS){
        std::cout << "pass of server " << this->PASS << std::endl;
        user.pass = true;
    }
    else{
        std::string rp = ERR_PASSWDMISTACH(user.ip);
        if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }
    // std::vector<std::string> splited = this->splitt(str, ' ');
    // if (splited.size() == 1){
    //     if (splited[0] == this->PASS){
    //         std::cout << "pass of server " << this->PASS << std::endl;
    //         user.pass = true;
    //     }
    //     else{
    //         std::string rp = ERR_PASSWDMISTACH(user.ip);
    //         if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
    //         {
    //             perror("send");
    //         }
    //     }
    // }
}

int_fast16_t checkCHANTYPES(std::string &str){
    if (str[0] == '#' || str[0] == '~' || str[0] == '+' || str[0] == '%' || str[0] == '$' || str[0] == '&' || str[0] == ':' || isdigit(str[0]))
        return 0;
    size_t i = 0;
    while (i < str.size()){
        if (str[i] == ',' || str[i] == '*' || str[i] == '?' || str[i] == '!' || str[i] == '@' || str[i] == '.')
            return 0;
        i++;
    }
    return 1;
}

void Server::validateNick(std::string &str, Clientx &user){
    std::vector<std::string> splited = this->splitt(str, ' ');
    if (!checkCHANTYPES(str)){
        std::string rp = ERR_ERRONEUSNICKNAME(user.ip, "NICK");
        if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }else{
        if (nickalreadyexist(splited[0].substr(0, 15)) != 0){
            if (splited[0].size() > 15){
                std::string rp = ERR_ERRONEUSNICKNAME(user.ip, "NICK");
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
            else{
                if (!clients_list.empty()) {
                    user.nickname = splited[0];
                }
            }
        }
        else{
            std::string rp = ERR_NICKINUSE(user.ip, "NICK");
            if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
        }
    }
}
void Server::validateUser(std::string &str, Clientx &user){
    std::vector<std::string> splited = this->splitt(str, ' ');
    if (splited.size() == 4){
        if (user.connected == false){
            if (splited[1] == "0" && splited[2] == "*"){
                user.username = splited[0];
                user.realname = splited[3];
            }
        }
        else{
            std::string rp = ERR_ALREADYREGISTERED(Server::hostname);
            if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    else{
        std::string rp = ERR_NEEDMOREPARAMS(user.ip, "USER");
        if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }
}

void Server::resetGuest()
{
    this->guest.nickname = "";
    this->guest.username = "";
    this->guest.pass = false;
    this->guest.connected = false;
    this->guest.c_fd = -1;
}

void Server::Register(Clientx &user)
{
    if (user.nickname.empty() || user.username.empty() || user.pass == false)
        return;
    std::string rp = RPL_WELCOME(user.nickname);
    std::string rp2 = RPL_YOURHOST(user.nickname);
    std::string rp3 = RPL_CREATED(user.nickname);
    std::string rp4 = RPL_MYINFO(user.client);
    std::string rp5 = RPL_ISUPPORT(user.nickname);
    if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
    {
        perror("send");
    }
    if (send(user.c_fd, rp2.c_str(), rp2.length(), 0) == -1)
    {
        perror("send");
    }
    if (send(user.c_fd, rp3.c_str(), rp3.length(), 0) == -1)
    {
        perror("send");
    }
    if (send(user.c_fd, rp4.c_str(), rp4.length(), 0) == -1)
    {
        perror("send");
    }
    if (send(user.c_fd, rp5.c_str(), rp5.length(), 0) == -1)
    {
        perror("send");
    }
    user.connected = true;
}

std::list<Clientx>::iterator Server::getUserfromClientlist(int fd){
   
   std::list<Clientx>::iterator it = this->clients_list.begin();
   while (it != this->clients_list.end())
   {
        if (it->c_fd == fd)
            return it;
        ++it;
   }
   return this->clients_list.end();
}

void Server::Authenticate(Clientx &user)
{
    try
    {
    std::vector<std::string> commandparsed = parcing(user);
    std::string firstarg = toupper(commandparsed[0]);
    std::string cmd[3]= {"PASS","NICK","USER"};
    int idx = 0;
    while(idx < 3)
    {
        if (cmd[idx] == firstarg)
            break;
        idx++;
    }
    switch(idx)
    {
        case 0:
            if (!commandparsed[1].empty()){
                validatePass(commandparsed[1], user);
            }
            else {
                std::string rp = ERR_NEEDMOREPARAMS(user.ip, "PASS");
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
            break;
        case 1:
            if (user.pass == true){
                if (!commandparsed[1].empty()){
                    validateNick(commandparsed[1], user);
                }
                else {
                    std::string rp = ERR_NONICKNAMEGIVEN(user.ip);
                    if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else{
                std::string rp = ERR_NOPASSGIVEN(user.ip);
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
            Register(user);
            break;
        case 2:
            if (user.pass == true){
                if (!commandparsed[1].empty())
                    validateUser(commandparsed[1], user);
                else {
                    std::string rp = ERR_NEEDMOREPARAMS(user.ip, "PASS");
                    if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else{
                std::string rp = ERR_NOPASSGIVEN(user.ip);
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
            Register(user);
            break;
        default :
            std::string cmd[10]= {"EXIT","JOIN", "BOT","KICK", "TOPIC", "PRIVMSG", "INVITE", "QUIT", "PART", "MODE"};
            int d = 0;
            while(d < 10)
            {
                if (cmd[d] == firstarg)
                    break;
                d++;
            }
            if (d == 10)
            {
                std::string rp = ERR_UNKNOWNCOMMAND(user.ip, firstarg);
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
            else
            {
                std::string rp = ERR_NOTREGISTERED(user.ip);
                if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
            }
    }

    firstarg.clear();
    commandparsed.clear();
    cmds.clear();

    }
    catch(std::exception &e)
    {

    }
}

