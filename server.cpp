#include"server.hpp"
#include"Commands.hpp"
#include"Channel.hpp"

class Client;
class Channel;

std::string normalize_string(std::string f)
{
    std::string s;
        for (size_t i = 0; i < f.length(); i++)
            s += std::tolower(f[i]);
        size_t index = f.find("\r");
        if (index != std::string::npos)
            s.erase(index);
        index = s.find("\n");
        if (index != std::string::npos)
            s.erase(index);
    return s;
}


void Server::addUser(int fd, std::string ip)
{
    Clientx user(fd, ip);
    clientsList.push_back(user);
}

void Server::parseCmd(int i)
{
   int index;
   std::string l;
   
   while(1)
   {
        index = clientsList[i].cmd.find_first_of(" ");
        if (index == -1)
        {
            cmds.push_back(clientsList[i].cmd);
            break;
        }
        l = clientsList[i].cmd.substr(0, index);
        if (l != "")
            cmds.push_back(l);
        clientsList[i].cmd = clientsList[i].cmd.substr(index + 1, clientsList[i].cmd.length());
   }
//    user clientx => cmd string
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

void Server::parcing(int i){
    // int i = getUserfromClientlist(fd);
    commandparsed.clear();
    std::string word;
    std::istringstream iss(clientsList[i].cmd);
    iss >> word;
    std::string firstarg = word;


    clientsList[i].cmd = trim(clientsList[i].cmd);
    // clientsList[i].cmd = toupper(clientsList[i].cmd);
    std::istringstream ss(clientsList[i].cmd);
    size_t npos = clientsList[i].cmd.find_first_of(" \t\n\r\v\f");
    if (npos == std::string::npos){
        // puts("1");
        // commanddev.push_back(firstarg);
        commandparsed.push_back(clientsList[i].cmd);
    }
    else{
        // puts("2");
        commandparsed.push_back(clientsList[i].cmd.substr(0, npos));
        commandparsed.push_back(trim(clientsList[i].cmd.substr(npos)));
    }
    std::cout << "command parsed : " << std::endl;
    size_t j = 0;
    while (j < commandparsed.size()){
        std::cout << commandparsed[j] << " ";
        j++;
    }
    std::cout << std::endl;
    std::cout << "end." << std::endl;
    // return commandparsed;
}
// void Server::parcing(int i){
//     // int i = getUserfromClientlist(fd);
//     commandparsed.clear();
//     std::string word;
//     std::istringstream iss(clientsList[i].cmd);
//     iss >> word;
//     std::string firstarg = word;


//     clientsList[i].cmd = trim(clientsList[i].cmd);
//     // clientsList[i].cmd = toupper(clientsList[i].cmd);
//     std::istringstream ss(clientsList[i].cmd);
//     size_t npos = clientsList[i].cmd.find_first_of(" \t\n\r\v\f");
//     if (npos == std::string::npos){
//         // puts("1");
//         // commanddev.push_back(firstarg);
//         commandparsed.push_back(clientsList[i].cmd);
//     }
//     else{
//         // puts("2");
//         commandparsed.push_back(clientsList[i].cmd.substr(0, npos));
//         commandparsed.push_back(trim(clientsList[i].cmd.substr(npos)));
//     }
//     std::cout << "command parsed : " << std::endl;
//     size_t j = 0;
//     while (j < commandparsed.size()){
//         std::cout << commandparsed[j] << " ";
//         j++;
//     }
//     std::cout << std::endl;
//     std::cout << "end." << std::endl;
// }

std::string Server::check_num(char c, int index)
{
    char str[] = ":#0123456789";
    int i;
    std::string error = ERR_ERRONEUSNICKNAME(clientsList[index].ip, "nick");

    i = 0;
    while (i < 12)
    {
        if (str[i] == c)
            return (error);
        i++;            
    }
    return "";
}


int  Server::nickalreadyexist(std::string nick)
{
    size_t i;

    i = 0;
    puts("here 1");
    while (i < clientsList.size())
    {
        if (clientsList[i].nickname == nick)
            return 0;
        i++;
    }
    return 1;
}

// void Server::auth(int i)
// {
//     // toupper(command)
//     std::string firstarg;
//     firstcmd = normalize_string(cmds[0]);
    
//     if (firstcmd == "pass")
//     {
//         if (cmds.size() == 2)
//         {
//             size_t index = cmds[1].find_first_of("\n");
//             if (index != std::string::npos)
//                 cmds[1].erase(index);
//             index = cmds[1].find_first_of("\r");
//             if (index != std::string::npos)
//                 cmds[1].erase(index);
//             if (cmds[1].compare(PASS) == 0)
//             {
//                 clientsList[i].pass = true;
//             }
//             else {
//                 std::string error;

//                 error =   ERR_PASSWDMISTACH(clientsList[i].ip);
//                 write(clientsList[i].c_fd, error.c_str(), error.length());
//             }
//         }
//         else {
//             std::string error;

//             error = ERR_NEEDMOREPARAMS(clientsList[i].ip, "pass");
//             write(clientsList[i].c_fd, error.c_str(), error.length());
//         }
//     }
//     else if (firstcmd == "nick")  // segfault when ,calling nick after pass
//     {
//         puts("test");
//         if (cmds.size() >= 2)
//         {
//             if (clientsList[i].pass == true)
//             {
//                 std::cout<<"cmds.size()==> "<<cmds.size()<<std::endl;
//                 size_t in = cmds[1].find_last_of("\r"); 
//                 if (in != std::string::npos)
//                     cmds[1].erase(in);
//                 in = cmds[1].find_last_of("\n");
//                 if (in != std::string::npos)
//                     cmds[1].erase(in);
//                 std::string s = check_num(cmds[1][0], i);
//                 if (s.length() != 0)
//                 {
//                     cmds.clear();
//                     return;        
//                 }
//                 s = checkExnick(this->cmds[1]);
//                 if (s.length() != 0)
//                 {
//                     close(clientsList[i].c_fd);
//                     clientsList.erase(clientsList.begin() + i);
//                     // pfds.erase(pfds.begin() + i);// ...
//                     // clientFdlist.erase(clientFdlist.begin() + i);
//                     write(clientsList[i].c_fd, s.c_str(), s.length());
//                 }
//                 else
//                 {
//                     clientsList[i].nickname = cmds[1].substr(0, 15);
//                     puts("nick done !");
//                 }

//             }
//             else{
               
//             }
//         }
//         else{
//             // std::string error;

//             // error = ERR_NEEDMOREPARAMS(clientsList[i].ipaddr, "nick");
//             // write(clientsList[i].c_fd, error.c_str(), error.length());
            
//         }
//     }
//     else if (firstcmd == "user")
//     {
//         if (cmds.size() >= 5)
//         {
//             if (cmds[2] == "0" && cmds[3] == "*" && clientsList[i].pass == true)
//             {
//                 clientsList[i].username = cmds[1];
//                 size_t j;

//                 j = 4;  
//                 while (j < cmds.size())
//                 {
//                     clientsList[i].realname += cmds[j];
//                     j++;
//                 }
//             }
//             else{
//             }
//         }
//         else {
//             std::string error;

//             error = ERR_NEEDMOREPARAMS(clientsList[i].ip, "user");
//             write(clientsList[i].c_fd, error.c_str(), error.length());
//         }
//     }
//     if (clientsList[i].pass == true && clientsList[i].nickname.empty() == false && clientsList[i].username.empty() == false)
//     {
//         // write_to_client(clientsList[i].c_fd, RPL_WELCOME(clientsList[i].nickname));
//         std::string s = RPL_WELCOME(clientsList[i].nickname);
//         write(clientsList[i].c_fd, s.c_str(), s.length());
//         // write_to_client(clientsList[i].c_fd, RPL_YOURHOST(clientsList[i].nickname, myhostname()));
//         // write_to_client(clientsList[i].c_fd, RPL_CREATED(clientsList[i].nickname));
//         // write_to_client(clientsList[i].c_fd, RPL_ISUPPORT(clientsList[i].nickname, "MAXCHANNEL=20 MAXCLIENT=51 MAXNICKLEN=15 MAXTOPICLEN=60"));
//         // write_to_client(clientsList[i].c_fd, RPL_MOTD(clientsList[i].nickname, "This server accepts IPv4 connections on the following ports: 2000-9000"));
//         clientsList[i].connected = true;
//     }
//     cmds.clear();
// }

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
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

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
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

// Add a new file descriptor to the set
void Server::add_to_pfds(int newfd)
{
    struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfds.push_back(pfd);
}

void Server::del_from_pfds(int fdd)
{
    std::vector<struct pollfd>::iterator it;
    for (it = pfds.begin(); it != pfds.end(); it++) {
        if ((*it).fd == fdd){
            pfds.erase(it);
            break;
        }
    }
    if (it == pfds.end())
        return;
}



// void Server::addnewuser()
// {

// }

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
//         char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
// struct sockaddr_in sa;      // pretend this is loaded with something

// inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);

// printf("The IPv4 address is: %s\n", ip4);


// // IPv6:

// char ip6[INET6_ADDRSTRLEN]; // space to hold the IPv6 string
// struct sockaddr_in6 sa6;    // pretend this is loaded with something

// inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
        printf("pollserver: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family, 
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET_ADDRSTRLEN),
               newfdclient);
                
        addUser(newfdclient, inet_ntop(remoteaddr.ss_family, 
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET_ADDRSTRLEN));
        // send a welcome message to the client
        std::string welcomeMsg = "Welcome to the server!\n";
        if (send(newfdclient, welcomeMsg.c_str(), welcomeMsg.length(), 0) == -1)
        {
            perror("send");
        }
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

void Server::handleClientDataMsg(int index)
{
    char buf[2048];    // Buffer for client data
    size_t nbytes = recv(pfds[index].fd, buf, sizeof buf, 0);
    buf[nbytes] = '\0';
    this->clientsList[index].cmd += buf;
    this->clientsList[index].c_fd = pfds[index].fd;
    if (nbytes <= 0)
    {
        int sender_fd = pfds[index].fd;

        // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(sender_fd); // Bye!

        del_from_pfds(index);
    }
    else
    {
    puts("1");
        // Process received data
        buf[nbytes] = '\0';  // Ensure null-termination
        Command cmd;
        // cmd.getcommand(this->clientsList[index].cmd, this->channels, cmd, this->clientsList[index], this->clientsList);
        // if (this->clientsList[index].cmd.find("\r\n") != std::string::npos)
        // {
            parcing(index);
            if (this->clientsList[index].connected == true){
                puts("no");
                cmd.getcommand(this->clientsList[index].cmd, this->channels, cmd, this->clientsList[index], this->clientsList);
            }
            else
            {
                // this->auth(index);
                // try {
                    puts("yup");
                    std::cout << "fd =>" << pfds[index].fd << std::endl;
                    this->Authenticate(pfds[index].fd);
                // }
                // catch (const std::exception &e)
                // {
                //     std::string msgerr = e.what();
                //     if (msgerr.length() == 0)
                //         return;
                //     if (send(clientsList[index].c_fd, msgerr.c_str(), msgerr.length(), 0) == -1)
                //     {
                //         perror("send");
                //     }
                //     fdHandler(index);
                // }
            }
            this->clientsList[index].cmd = "";
        // }
    }
}

void Server::runServer()
{
    // Set up and get a listening socket
    this->listenerSock = get_listener_socket();
    if (this->listenerSock == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }
    // Add the listener to set
    add_to_pfds(this->listenerSock);// Report ready to read on incoming connection
    // Main loop
    for(;;) {
        int poll_count = poll(pfds.data(), pfds.size(), -1);

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
                    handleClientDataMsg(i);
                }
            }
        }
    } 
}

// void Server::auth(int i)
// {
//     std::cout<<"client fd ==> "<<clientsList[i].c_fd<<std::endl;
//     std::string firstarg = toupper(this->commandparsed[0]); 
//     std::cout << "this->commandparsed[1] "<< this->commandparsed[1] << "this->PASS " << this->PASS << std::endl;
//     if (firstarg == "PASS")
//     {
//         if (commandparsed.size() == 2)
//         {
//             // std::cout << "this->commandparsed[1] "<< this->commandparsed[1] << "this->PASS " << this->PASS << std::endl;
//             if (this->commandparsed[1].compare(this->PASS) == 0)
//             {
//                 clientsList[i].pass = true;
//             }
//             else {
//                 std::string error;
//                 error = ERR_PASSWDMISTACH(clientsList[i].ip);
//                 if (send(clientsList[i].c_fd, error.c_str(), error.length(), 0) == -1)
//                 {
//                     perror("send");
//                 }
//             }
//         }
//         else {
//             std::string error;

//             error = ERR_NEEDMOREPARAMS(clientsList[i].ip, "pass");
//             if (send(clientsList[i].c_fd, error.c_str(), error.length(), 0) == -1)
//             {
//                 perror("send");
//             }
//         }
//     }
//     else if (firstarg == "NICK")  // segfault when ,calling nick after pass
//     {
//         if (commandparsed.size() >= 2)
//         {
//             if (clientsList[i].pass == true)
//             {
//                 std::string s = check_num(commandparsed[1][0], i);
//                 if (s.length() != 0)
//                 {
//                     close(clientsList[i].c_fd);
//                     clientsList.erase(clientsList.begin() + i);
//                     del_from_pfds(clientsList[i].c_fd);
//                     if (send(clientsList[i].c_fd, s.c_str(), s.length(), 0) == -1)
//                     {
//                         perror("send");
//                     }    
//                 }
//                 s = checkExnick(this->commandparsed[1]);
//                 if (s.length() != 0)
//                 {
//                     close(clientsList[i].c_fd);
//                     clientsList.erase(clientsList.begin() + i);
//                     del_from_pfds(clientsList[i].c_fd);
//                     // pfds.erase(pfds.begin() + i);// ...
//                     // clientFdlist.erase(clientFdlist.begin() + i);
//                     if (send(clientsList[i].c_fd, s.c_str(), s.length(), 0) == -1)
//                     {
//                         perror("send");
//                     }
//                 }
//                 else
//                 {
//                     clientsList[i].nickname = commandparsed[1].substr(0, 15);
//                     puts("nick done !");
//                 }
//             }
//         }
//         else{
//             // std::string error;

//             // error = ERR_NEEDMOREPARAMS(clientsList[i].ipaddr, "nick");
//             // write(clientsList[i].c_fd, error.c_str(), error.length());
//             std::string error;

//             error =  ERR_NONICKNAMEGIVEN(clientsList[i].ip);
//             if (send(clientsList[i].c_fd, error.c_str(), error.length(), 0) == -1)
//             {
//                 perror("send");
//             }
//         }
//     }
//     else if (firstarg == "USER")
//     {
//         std::cout << "commandparsed.size()" << commandparsed.size() << std::endl;
//         if (commandparsed.size() >= 5)
//         {
//             if (commandparsed[2] == "0" && commandparsed[3] == "*" && clientsList[i].pass == true)
//             {
//                 clientsList[i].username = commandparsed[1];
//                 size_t j;

//                 j = 4;  
//                 while (j < commandparsed.size())
//                 {
//                     clientsList[i].realname += commandparsed[j];
//                     j++;
//                 }
//             }
//             else{
//             }
//         }
//         else {
//             std::string error;

//             error = ERR_NEEDMOREPARAMS(clientsList[i].ip, "user");
//             if (send(clientsList[i].c_fd, error.c_str(), error.length(), 0) == -1)
//             {
//                 perror("send");
//             }
//         }
//     }
//     if (clientsList[i].pass == true && clientsList[i].nickname.empty() == false && clientsList[i].username.empty() == false)
//     {
//         // write_to_client(clientsList[i].c_fd, RPL_WELCOME(clientsList[i].nickname));
//         std::string s = RPL_WELCOME(clientsList[i].nickname);
//         if (send(clientsList[i].c_fd, s.c_str(), s.length(), 0) == -1)
//         {
//             perror("send");
//         }
//         // write_to_client(clientsList[i].c_fd, RPL_YOURHOST(clientsList[i].nickname, myhostname()));
//         // write_to_client(clientsList[i].c_fd, RPL_CREATED(clientsList[i].nickname));
//         // write_to_client(clientsList[i].c_fd, RPL_ISUPPORT(clientsList[i].nickname, "MAXCHANNEL=20 MAXCLIENT=51 MAXNICKLEN=15 MAXTOPICLEN=60"));
//         // write_to_client(clientsList[i].c_fd, RPL_MOTD(clientsList[i].nickname, "This server accepts IPv4 connections on the following ports: 2000-9000"));
//         clientsList[i].connected = true;
//     }
//     commandparsed.clear();
// }

void Server::validatePass(std::string &str, int i){
    if (clientsList[i].pass == true){
        std::string rp = ERR_ALREADYREGISTERED(Server::hostname);
        if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
        // fdHandler(i);
    }
    std::vector<std::string> splited = this->splitt(str, ' ');
    if (splited.size() == 1){
        if (splited[0] == this->PASS)
            clientsList[i].pass = true;
        else{
            std::string rp = ERR_PASSWDMISTACH(clientsList[i].ip);
            if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
            // fdHandler(i);
        }
    }
}

int_fast16_t checkCHANTYPES(std::string &str){
// CHANTYPES=#

// CHANTYPES=&#

// CHANTYPES=#&
    if (str[0] == '#' || str[0] == '&' || str[0] == ':' || isdigit(str[0]))
        return 0;
    return 1;
}

void Server::validateNick(std::string &str, int i){
    std::cout << "str " << str << std::endl; 
    std::vector<std::string> splited = this->splitt(str, ' ');
    std::cout << "splited[0] " << splited[0] << std::endl; 
    if (!checkCHANTYPES(str)){
        std::string rp = ERR_ERRONEUSNICKNAME(clientsList[i].ip, "NICK");
        if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
        // fdHandler(i);
    }else{
        puts("here");
        if (nickalreadyexist(splited[0].substr(0, 15)) != 0){
            puts("here2");
            if (splited[0].size() > 15){
                puts("here3");
                std::string rp = ERR_ERRONEUSNICKNAME(clientsList[i].ip, "NICK");
                if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
                // fdHandler(i);
            }
            else{
                puts("here4");
                // clientsList[i].nickname = splited[0];
                if (!clientsList.empty()) {
                    puts("yes dkhl");
                    std::cout << "clientsList size " << clientsList.size() << std::endl; 
                    std::cout << "i dyal clientsList  " << i << std::endl; 
                    clientsList[i].setNickname(splited[0]);
                }
                puts("here5");
            }
        }
        else{
            std::string rp = ERR_NICKINUSE(clientsList[i].ip, "NICK");
            if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
            // fdHandler(i);
        }
    }
}
void Server::validateUser(std::string &str, int i){
    // user hala 0 * loka | strlen = 4
    std::vector<std::string> splited = this->splitt(str, ' ');
    if (splited.size() == 4){
        if (clientsList[i].connected == false){
            if (splited[1] == "0" && splited[2] == "*"){
                clientsList[i].username = splited[0];
                clientsList[i].realname = splited[3];
            }
        }
        else{
            std::string rp = ERR_ALREADYREGISTERED(Server::hostname);
            if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    else{
        std::string rp = ERR_NEEDMOREPARAMS(clientsList[i].ip, "USER");
        if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }
}

void Server::Register(int i)
{
    if (clientsList[i].nickname.empty() || clientsList[i].username.empty() || clientsList[i].pass == false)
        return;
    std::string rp = RPL_WELCOME(clientsList[i].nickname);
    if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
    {
        perror("send");
    }
    std::cout << "after replay" << std::endl;
    std::cout << "nick "<< clientsList[i].nickname << " user " << clientsList[i].username << std::endl;
    clientsList[i].connected = true;
}

// void Server::fdHandler(int i){
//     int client_fd = clientsList[i].c_fd; // Save the client_fd before erasing
//     close(client_fd);
//     clientsList.erase(clientsList.begin() + i);
//     del_from_pfds(client_fd);
// }

void Server::fdHandler(int i){
    if (i >= 0 && i < clientsList.size()) {
        int client_fd = clientsList[i].c_fd;
        close(client_fd);
        
        // Erase the client from clientsList
        clientsList.erase(clientsList.begin() + i);

        // Check if clientsList is not empty before calling del_from_pfds
        if (!clientsList.empty()) {
            del_from_pfds(client_fd);
        }
    }
}

int Server::getUserfromClientlist(int fd){
    size_t i = 0;
    while (i < this->clientsList.size()){
        if (this->clientsList[i].c_fd == fd)
            return i;
        i++;
    }
    return -1;
}
void Server::Authenticate(int fd)
{
    int i = getUserfromClientlist(fd);
    std::vector<std::string> commandparsed = parcing(i);
    std::string firstarg = toupper(this->commandparsed[0]);
    std::cout << "this->commandparsed[0] " << this->commandparsed[0] << std::endl;
    std::cout << "this->commandparsed[1] " << this->commandparsed[1] << std::endl;
    std::cout << "firstarg " << firstarg << std::endl;
    std::cout << "i of my client " << i << std::endl;
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
            if (!this->commandparsed[1].empty())
                validatePass(this->commandparsed[1], i);
            else {
                std::string rp = ERR_NEEDMOREPARAMS(clientsList[i].ip, "PASS");
                if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
                // fdHandler(i);
            }
            break;
        case 1:
            if (clientsList[i].pass == true){
                std::cout << "this->commandparsed[1]|" << this->commandparsed[1] << "|"<< std::endl; 
                if (!this->commandparsed[1].empty()){
                    puts("some nickname given");

                    validateNick(this->commandparsed[1], i);
                }
                else {
                    puts("no nickname given");
                    std::string rp = ERR_NONICKNAMEGIVEN(clientsList[i].ip);
                    if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                    {
                        perror("send");
                    }
                    // fdHandler(i);
                }
            }
            else{
                std::string rp = ERR_NOPASSGIVEN(clientsList[i].ip);
                if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
                // fdHandler(i);
            }
            Register(i);
            break;
        case 2:
            if (clientsList[i].pass == true){
                if (!this->commandparsed[1].empty())
                    validateUser(this->commandparsed[1], i);
                else {
                    std::string rp = ERR_NEEDMOREPARAMS(clientsList[i].ip, "PASS");
                    if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else{
                std::string rp = ERR_NOPASSGIVEN(clientsList[i].ip);
                if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
                {
                    perror("send");
                }
                // fdHandler(i);
            }
            Register(i);
            break;
        default :
            std::string rp = ERR_NOTREGISTERED(clientsList[i].ip);
            if (send(clientsList[i].c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
            // fdHandler(i);
    }
    // clearVec(commandparsed);
    firstarg.clear();
    commandparsed.clear();
    cmds.clear();
    // commandparsed[1] = "";
    // commandparsed[0] = "";
}

    // std::cout << "command parsed : " << std::endl;
    // size_t j = 0;
    // while (j < commandparsed.size()){
    //     std::cout << "i = : " << j << " " << commandparsed[j] << " ";
    //     j++;
    // }
    // std::cout << std::endl;
    // std::cout << "end." << std::endl;