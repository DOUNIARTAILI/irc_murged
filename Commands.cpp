#include"Commands.hpp"
#include<cctype>
#include "Channel.hpp"
#include"server.hpp"

void printallnicknames(std::vector<Clientx> &clients);
void printallchannels(std::vector<Channel> &chan);
Command::Command()
{
}

Command::~Command()
{
}

void toupper(std::string &str)
{
    size_t i = 0;
    while (str[i])
    {
        str[i] = std::toupper(str[i]);
        i++;
    }
}

void Command::joincommand() // fill channel vector with channel name and key
{
    size_t i = 0;
    if (command_arg.size() < 1)
    {
        std::cerr<<"not enough arguments"<<std::endl;
        return ;
    }
    std::istringstream channels(command_arg[0]);
    std::string tmp; 
    while (std::getline(channels, tmp, ','))
        channel.push_back(std::make_pair(tmp, ""));
    if (command_arg.size() > 1) // if there is a key
    {
        std::istringstream keys(command_arg[1]);
        while (std::getline(keys, tmp, ','))
        {
            if (i < channel.size())
            {
                channel[i].second = tmp;
                i++;
            }
        }
    }
}

void Command::kickcommand() 
{
    if (command_arg.size() < 1)
    { 
        std::cerr<<"not enough arguments"<<std::endl;
        return ;
    }
    if (command_arg.size() > 0) 
        channel.push_back(std::make_pair(command_arg[0], ""));
    if (command_arg.size() > 1)
    {
        std::string tmp;
        std::stringstream ss(command_arg[1]);
        while (std::getline(ss, tmp, ','))
            users.push_back(tmp);
    }
    if (command_arg.size() > 2)
    {
        if (command_arg[2][0] == ':')
        {
            comment = mainstring.substr(mainstring.find(':'));
            comment = comment.erase(comment.size() - 1);
        }
        else
            comment = command_arg[2];
    }
}

size_t iscommand(std::string &str)
{
    std::string command[10] = {"BOT","JOIN", "KICK", "TOPIC", "PRIVMSG", "INVITE", "QUIT", "PART", "NICK", "MODE"};
    size_t i = 0;
    while(i < 9)
    {
        if (str == command[i])
            return 1;
        i++;
    }
    return 0;
}

void Command::getcommand(std::string const &str, std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server)
{
    // command JOIN | args #ch | cmd 
    mainstring = str;
    std::string command_args;
    std::istringstream iss(str);
    iss >> command;
    if (command[0] == '/')
        command.erase(0, 1);
    size_t size_arg = 0;
    if (!command.empty())
        toupper(command);
    while (iss >> command_args)
    {
        command_arg.push_back(command_args); // hh
        size_arg++;
    }
    if (!command.empty())
    {
        // if (command_args.size() < 1 && iscommand(command) && command != "QUIT")
        // {
        //     std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, command);
        //     write(client.c_fd, moreparams.c_str(), moreparams.size());
        //     return ;
        // }
        toupper(command);
        if (command == "PONG")
            return ;
        if (command == "EXIT")
        {
            std::cout<<"Exiting server"<<std::endl;
            exit(77);
        }
        if (command == "JOIN")
        {
            joincommand();
            join(chan, cmd, client);
        }
        else if (command == "BOT")
        {
            std::cout<<"bot xxxxxtime"<<std::endl;
            botCommand();
            bot(chan, cmd, client);
        }
        else if (command == "KICK")
        {
            kickcommand();
            kick(chan, cmd, client);
        }
        else if (command == "TOPIC")
        {
            topiccommand();
            topicf(chan, cmd, client, clients);
        }
        else if (command == "PRIVMSG")
        {
            privmsg();
            prv(chan, cmd, client, clients);
        }
        else if (command == "INVITE")
        {
            invitecommand();
            invite(chan,cmd,client,clients);
        }
        else if (command == "QUIT")
        {
            quitcommand();
            quit(chan, cmd, client, clients, server);
        }
        else if (command == "PART")
        {
            partcommand();
            part(chan, cmd, client);
        }
        else if (command == "NICK")
        {
            nickcommand();
            nick(chan, cmd, client, clients);
        }
        else if (command == "MODE")
        {
            modecommand();
            modef(chan, cmd, client);
        }
        // else if (command == "LISTNICK")
        //     printallnicknames(clients);
        else if (command == "LISTCHANNELS")
            printallchannels(chan);
        else
        {
            std::string unknowncommand = ERR_UNKNOWNCOMMAND(client.nickname, command);
            std::cout<<cmd.command_arg.size()<<std::endl;
            write(client.c_fd, unknowncommand.c_str(), unknowncommand.size());
            return ;
        }
    }
}

void Command::privmsg()
{
    if (command_arg.size() > 1)
    {
        std::istringstream users(command_arg[0]);
        std::string tmp;
        while (std::getline(users, tmp, ','))
            privmsg_list.push_back(tmp);
    }
    if (command_arg.size() >= 2)
    {
        size_t pos = mainstring.find(command_arg[1]);
        privmessage = mainstring.substr(pos);
        privmessage = privmessage.erase(privmessage.size() - 1);
    }
}

void Command::topiccommand()
{
    if (command_arg.size() > 0)
        channel.push_back(std::make_pair(command_arg[0], ""));
    if (command_arg.size() > 1)
    {
        if (command_arg[1][0] == ':')
        {
            topic = mainstring.substr(mainstring.find(':') + 1);
            topic = topic.erase(topic.size() - 1);
        }
        else
            topic = command_arg[1];
    }
}

void Command::invitecommand()
{
    std::cout<<"invite cmdsize "<<command_arg.size()<<std::endl;
    if (command_arg.size() > 2)
    {
        std::cerr<<"too many arguments"<<std::endl;
        return ;
    }   
    if (command_arg.size() > 0)
            nickname = command_arg[0];
    if (command_arg.size() > 1)
        channel.push_back(std::make_pair(command_arg[1], ""));
}

void Command::nickcommand()
{
    if (command_arg.size() < 1)
    { 
        std::cerr<<"not enough arguments"<<std::endl;
        return ;
    }
    if (command_arg[0][0] == ':')
        nickname = command_arg[0].substr(1);
    else
        nickname = command_arg[0];
}

void Command::partcommand()
{
    if (command_arg.size() < 1)
    {
        std::cerr<<"not enough arguments"<<std::endl;
        return ;
    }
    std::istringstream channels(command_arg[0]);
    std::string tmp;
    while(std::getline(channels, tmp, ','))
        channel.push_back(std::make_pair(tmp, ""));
    if (command_arg.size() > 1)
    {
        if (command_arg[1][0] == ':')
        {
            comment = mainstring.substr(mainstring.find(':'));
            comment = comment.erase(comment.size() - 1);
        }
        else
            comment = command_arg[1];
    }
}

void Command::quitcommand()
{
    if (command_arg.size() > 1)
    {
        if (command_arg[0][0] == ':')
        {
            comment = mainstring.substr(mainstring.find(':'));
            comment = comment.erase(comment.size() - 1);
        }
        else
            comment = command_arg[0];
    }
}

void Command::modecommand()
{
    if (command_arg.size() > 0)
        channel.push_back(std::make_pair(command_arg[0], ""));
    if (command_arg.size() > 1)
    {
        size_t i = 0;
            char tmp = '+';
        while (command_arg[1][i])
        {
            if (command_arg[1][i] == '+' || command_arg[1][i] == '-')
                tmp = command_arg[1][i];
            else
                mode.push_back(std::make_pair(tmp, command_arg[1][i]));
            i++;
        }
        i = 2;
        while (i < command_arg.size())
        {
            mode_args.push_back(command_arg[i]);
            i++;
        }
    }
}


void Command::botCommand()
{
    // puts("bot 2");

    // std::cout<<"Command_arg[1] ==>"<<command_arg[0]<<std::endl;
    // std::cout<<"command_arg.size() "<<command_arg.size()<<std::endl;
    if (command_arg.size() == 0 || command_arg.size() > 1)
    {
        bot_arg = "";
        return ;
    }
    bot_arg = command_arg[0];
}