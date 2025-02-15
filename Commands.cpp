/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drtaili <drtaili@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/17 23:29:56 by drtaili           #+#    #+#             */
/*   Updated: 2024/03/17 23:30:03 by drtaili          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

std::string trim__(const std::string &str)
{
    size_t leftPos = str.find_first_not_of(" \t\n\r\v\f");
    size_t rightPos = str.find_last_not_of(" \t\n\r\v\f");

    if (leftPos == std::string::npos || rightPos == std::string::npos)
        return std::string("");  // Empty string if all characters are whitespace

    return str.substr(leftPos, rightPos - leftPos + 1);
}

void Command::joincommand() // fill channel vector with channel name and key
{
    size_t i = 0;
    if (command_arg.size() < 1)
    {
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
    try{
    if (command_arg.size() < 1)
    { 
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
    catch(std::exception &e)
    {
    }
}

size_t iscommand(std::string &str)
{
    std::string command[12] = {"BOT","JOIN", "KICK", "TOPIC", "PRIVMSG", "INVITE", "QUIT", "PART", "NICK", "MODE", "PASS", "USER"};
    size_t i = 0;
    while(i < 12)
    {
        if (str == command[i])
            return 1;
        i++;
    }
    return 0;
}

void Command::getcommand(std::string const &str, std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server)
{
    try
    {
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
        command_arg.push_back(command_args);
        size_arg++;
    }
    if (!command.empty())
    {  
        toupper(command);
        if (command == "PONG")
            return ;

        if (command == "JOIN")
        {
            joincommand();
            join(chan, cmd, client);
        }
        else if (command == "PASS")
        {
            std::string rpl = ERR_ALREADYREGISTERED(client.nickname);
            if (send(client.c_fd, rpl.c_str(), rpl.size(), 0) == -1)
                perror("send");
            return ;
        }
        else if (command == "USER")
        {
            std::string rpl = ERR_ALREADYREGISTERED(client.nickname);
            if (send(client.c_fd, rpl.c_str(), rpl.size(), 0) == -1)
                perror("send");
            return ;
        }
        else if (command == "BOT")
        {
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
        else
        {
            std::string unknowncommand = ERR_UNKNOWNCOMMAND(client.nickname, command);
            if (send(client.c_fd, unknowncommand.c_str(), unknowncommand.size(), 0) == -1)
                perror("send");
            return ;
        }
    }
    }
    catch(std::exception &e)
    {
        
    }
}

void Command::privmsg()
{
    if (command_arg.size() > 0)
    {
        std::stringstream split(command_arg.at(0));
        std::string token;
        while (std::getline(split, token, ','))
                privmsg_list.push_back(token);
        if (command_arg.size() > 1)
        {
            size_t i = 1;
            while(i < command_arg.size())
            {
                if (command_arg[i][0] == ':')
                {
                    size_t pos = mainstring.find(command_arg.at(i)) + 1;
                    if (mainstring[pos] == '\n')
                        privmessage = "";
                    else
                    {
                        mainstring = mainstring.substr(pos, mainstring.length() - pos);
                        pos = mainstring.find_last_not_of("\n");
                        if (pos != std::string::npos && pos < mainstring.length())
                            mainstring.erase(pos + 1);
                        privmessage = mainstring;
                    }
                    return ;
                }
                ++i;
            }
            privmessage = command_arg.at(command_arg.size() - 1);
        }
}
}

void Command::topiccommand()
{
    try
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
    catch(std::exception &e)
    {
    }
}

void Command::invitecommand()
{
    if (command_arg.size() > 2)
    {
        if(command_arg[1][0] == ':')
            command_arg[1].erase(0, 1);
    }   
    if (command_arg.size() > 0)
            nickname = trim__(command_arg[0]);
    if (command_arg.size() > 1)
        channel.push_back(std::make_pair(trim__(command_arg[1]), ""));
}

void Command::nickcommand()
{
    if (command_arg.size() < 1)
    { 
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
            comment = comment.erase(comment.size());
        }
        else
            comment = command_arg[1];
    }
}

void Command::quitcommand()
{
    if (command_arg.size() > 0)
    {
        if (command_arg[0][0] == ':')
        {
            comment = mainstring.substr(mainstring.find(':'));
            comment = comment.erase(comment.size());
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
    if (command_arg.size() == 0 || command_arg.size() > 1)
    {
        bot_arg = "";
        return ;
    }
    bot_arg = command_arg[0];
}




