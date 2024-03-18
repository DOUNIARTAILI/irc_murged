/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drtaili <drtaili@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/17 23:29:41 by drtaili           #+#    #+#             */
/*   Updated: 2024/03/18 01:22:53 by drtaili          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Commands.hpp"
#include "server.hpp"

class Clientx;

std::string old_arg; //GLOBAL !!

void broadcast(std::vector<Clientx *> &clients, std::string msg)
{
    size_t i = 0;
    while (i < clients.size())
    {
        if (send(clients[i]->c_fd, msg.c_str(), msg.size(), 0) == -1)
        {
            perror("send");
        }
        i++;
    }
}

void broadcast2(std::list<Clientx> &clients, std::string msg)
{
    std::list<Clientx>::iterator i = clients.begin();
    while (i != clients.end())
    {
        if (send(i->c_fd, msg.c_str(), msg.size(), 0) == -1)
        {
            perror("send");
        }
        ++i;
    }
}

void broadcast3(std::list<Clientx> &clients, std::string msg, Server &server,std::vector<Channel> &chan)
{
    (void)server;
    (void)chan;
    std::list<Clientx>::iterator i = clients.begin();
    while (i != clients.end())
    {
        if (send(i->c_fd, msg.c_str(), msg.size(), 0) == -1)
        {
            perror("send 1");
        }
        ++i;
    }
}

std::string my_hostname()
{
    std::string host_name;
    char hostname[256];

    if (!gethostname(hostname, sizeof(hostname)))
        host_name = hostname;
    return host_name;
}

std::string usersonchan(Channel &channel)
{
    size_t i = 0;
    std::string users;
    while (i < channel.user_list.size())
    {
        if (channel.is_operator(channel.user_list[i]->nickname))
            users += '@';
        users += channel.user_list[i]->nickname;
        if (i < channel.user_list.size() - 1)
            users += " ";
        i++;
    }
    return users;
}

void join(std::vector<Channel> &chan, Command &cmd, Clientx &client)
{
    size_t i = 0;

    if (cmd.command_arg.size() > 0)
    {
        while (i < cmd.channel.size())
        { 
            if (cmd.channel[i].first[0] != '#')
                cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
            if (it != chan.end())
            {
                if (it->user_list.size() >= it->max_users && it->mode.find('l') != std::string::npos)
                {
                    std::string maxusers = ERR_CHANNELISFULL(client.nickname, cmd.channel[i].first);
                    if (send(client.c_fd, maxusers.c_str(), maxusers.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                if (it->mode.find('i') != std::string::npos && it->is_invite(client.nickname) == false)
                {
                    std::string inviteonly = ERR_INVITEONLYCHAN(client.nickname, cmd.channel[i].first);
                    if (send(client.c_fd, inviteonly.c_str(), inviteonly.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                if (it->mode.find('k') != std::string::npos && it->pwd != cmd.channel[i].second)
                {
                    if (!it->is_user(client.nickname))
                    {
                        std::string badpassword = ERR_BADCHANNELKEY(client.nickname, cmd.channel[i].first);
                        if (send(client.c_fd, badpassword.c_str(), badpassword.size(), 0) == -1)
                        {
                            perror("send");
                        }
                        // broadcast(it->user_list, badpassword);
                        return;
                    }
                }
                if (!it->is_user(client.nickname))
                {
                    it->add_user(client);
                    std::string j_rpl = JOIN_SUCC(client.nickname, client.username, client.ip, cmd.channel[i].first);
                    broadcast(it->user_list, j_rpl);
                    j_rpl = RPL_NAMERPLY(client.nickname, cmd.channel[i].first, usersonchan(*it));
                    if (send(client.c_fd, j_rpl.c_str(), j_rpl.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    j_rpl = RPL_ENDOFNAMES(client.nickname, cmd.channel[i].first);
                    if (send(client.c_fd, j_rpl.c_str(), j_rpl.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else
            {
                if (cmd.channel[i].first[0] != '#')
                {
                    std::string notonchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
                    if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
                else if (cmd.channel[i].first[0] == '#' && !cmd.channel[i].first[1])
                {
                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "JOIN #");
                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
                else
                {
                    Channel tmp(cmd.channel[i].first);
                    chan.push_back(tmp);
                    chan[chan.size() - 1].add_user(client);
                    chan[chan.size() - 1].add_operator(client);
                    std::string j_rpl = JOIN_SUCC(client.nickname, client.username, client.ip, cmd.channel[i].first);
                    if (send(client.c_fd, j_rpl.c_str(), j_rpl.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    j_rpl = RPL_NAMERPLY(client.nickname, cmd.channel[i].first, usersonchan(chan[chan.size() - 1]));
                    if (send(client.c_fd, j_rpl.c_str(), j_rpl.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    j_rpl = RPL_ENDOFNAMES(client.nickname, cmd.channel[i].first);
                    if (send(client.c_fd, j_rpl.c_str(), j_rpl.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            i++;
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "JOIN");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

void broadcast_kick(std::vector<Channel>::iterator it, Clientx &user, std::string &cli, std::string &reason)
{
    for (size_t i = 0; i < it->user_list.size(); ++i)
    {
        std::string syn = KICK_MSG(user.nickname, user.ip, it->name, cli, reason);
        if (send(it->user_list[i]->c_fd, syn.c_str(), syn.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

void kick(std::vector<Channel> &chan, Command &cmd, Clientx &client)
{
    size_t i = 0;
    if (cmd.command_arg.size() > 1)
    {
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
        if (it != chan.end())
        {
            if (it->is_operator(client.nickname))
            {
                while (i < cmd.users.size())
                {
                    if (it->is_user(cmd.users[i]))
                    {
                        broadcast_kick(it, client, cmd.users[i], cmd.comment);
                        it->remove_operator(cmd.users[i]);
                        it->remove_user(cmd.users[i]);
                    }
                    else
                    {
                        std::string nosuchnick = ERR_NOSUCHNICK(client.nickname, cmd.users[i]);
                        if (send(client.c_fd, nosuchnick.c_str(), nosuchnick.size(), 0) == -1)
                        {
                            perror("send");
                        }
                    }
                    i++;
                }
            }
            else
            {
                std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[i].first);
                if (send(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size(), 0) == -1)
                {
                    perror("send");
                }
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
            if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "KICK");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

void part(std::vector<Channel> &chan, Command &cmd, Clientx &client)
{
    if (cmd.command_arg.size() < 1)
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "PART");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
    size_t i = 0;
    while (i < cmd.channel.size())
    {
        if (cmd.channel[i].first[0] != '#')
            cmd.channel[i].first = '#' + cmd.channel[i].first;
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
        if (it != chan.end())
        {
            if (it->is_user(client.nickname))
            {
                std::string partmsg = PART_MSG(client.nickname, client.username, client.ip, cmd.channel[i].first, cmd.comment);
                broadcast(it->user_list, partmsg);
                it->remove_operator(client.nickname);
                it->remove_user(client.nickname);
                if (it->user_list.size() == 0)
                    chan.erase(it);
            }
            else
            {
                std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[i].first);
                if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                {
                    perror("send");
                }
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
            if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
            {
                perror("send");
            }
        }
        i++;
    }
}


void invite(std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    if (cmd.command_arg.size() > 1)
    {
        if (cmd.channel[0].first[0] != '#')
            cmd.channel[0].first = '#' + cmd.channel[0].first;
        std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.command_arg[0]));
        if (it != clients.end())
        {
            std::vector<Channel>::iterator it2 = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
            if (it2 != chan.end())
            {
                if (!it2->is_user(client.nickname))
                {
                    std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[0].first);
                    if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                if (it2->is_operator(client.nickname))
                {
                    std::string sendinv = INVITE_MSG(client.nickname, client.username, client.ip, cmd.command_arg[0], cmd.channel[0].first);
                    if (send(it->c_fd, sendinv.c_str(), sendinv.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    it2->inv_list.push_back(&*it);
                }
                else
                {
                    std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[0].first);
                    if (send(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else
            {
                std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
                if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
                {
                    perror("send");
                }
            }
        }
        else
        {
            std::string nosuchnick = ERR_NOSUCHNICK(client.nickname, cmd.command_arg[0]);
            if (send(client.c_fd, nosuchnick.c_str(), nosuchnick.size(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "INVITE");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

// void Server::fdHandler(int i){
//     if (i >= 0 && i < clientsList.size()) {
//         int client_fd = clientsList[i].c_fd;
//         close(client_fd);

//         // Erase the client from clientsList
//         clientsList.erase(clientsList.begin() + i);

//         // Check if clientsList is not empty before calling del_from_pfds
//         if (!clientsList.empty()) {
//             del_from_pfds(client_fd);
//         }
//     }
// }

// Function to quit a client from a specific channel
// void quit_channel(Channel& channel, int fd)
// {
//     // std::vector<Clientx*>::iterator itu = std::find(channel.user_list.begin(), channel.user_list.end(), fd);
//     // std::vector<Clientx*>::iterator ito = std::find(channel.op_list.begin(), channel.op_list.end(), fd);
//     // std::vector<Clientx*>::iterator iti = std::find(channel.inv_list.begin(), channel.inv_list.end(), fd);

//     // if (itu != channel.user_list.end()) {
//     //     channel.user_list.erase(itu);
//     // }
//     // if (ito != channel.op_list.end()) {
//     //     channel.op_list.erase(ito);
//     // }
//     // if (iti != channel.inv_list.end()) {
//     //     channel.inv_list.erase(iti);
//     // }
//     // std::vector<Clientx*>::iterator itu = channel.begin();
//     // for (; itu != channel.user_list.end(); itu++){
//     //     if (itu.)
//     // }

// removing client from channels
// size_t i = 0;
// while (i < chan.size()){
//     quit_channel(chan[i], client.c_fd);
//     i++;
// }

void quit(std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server)
{
    std::vector<Channel>::iterator iter = chan.begin();

    while (iter != chan.end())
    {
        if (iter->is_user(client.nickname))
        {
            iter->remove_user(client.nickname);
            iter->remove_operator(client.nickname);
            iter->remove_invite(client);

            if (iter->user_list.empty())
                iter = chan.erase(iter); // Update the iterator after erasing
            else
                ++iter;
        }
        else
            ++iter;
    }
    // removing client from clientList
    std::list<Clientx>::iterator itl = std::find(clients.begin(), clients.end(), client);
    if (itl != clients.end())
    {
        clients.erase(itl);
    }
    std::string quitmsg = QUIT_MSG(client.nickname, client.username, client.ip, cmd.comment);
    broadcast3(clients, quitmsg, server,chan);
    close(client.c_fd);
    if (!clients.empty())
    {
        server.del_from_pfds(client.c_fd);
    }
}

std::string trim_(const std::string &str)
{
    size_t leftPos = str.find_first_not_of(" \t\n\r\v\f");
    size_t rightPos = str.find_last_not_of(" \t\n\r\v\f");

    if (leftPos == std::string::npos || rightPos == std::string::npos)
        return std::string("");  // Empty string if all characters are whitespace

    return str.substr(leftPos, rightPos - leftPos + 1);
}

std::vector<std::string> splitt_(const std::string &str, char del)
{
    std::istringstream iss(str);
    std::string arg;
    std::vector<std::string> v;

    while (std::getline(iss, arg, del))
    {
        if (del == ' ')
            arg = trim_(arg);
        if (!arg.empty())
            v.push_back(arg);
    }
    return v;
}

int_fast16_t checkCHANTYPES_(std::string &str){
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

int validate_Nick(std::string &str, Clientx &user){
    std::vector<std::string> splited = splitt_(str, ' ');
    if (!checkCHANTYPES_(str)){
        std::string rp = ERR_ERRONEUSNICKNAME(user.ip, "NICK");
        if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
        return 1;
    }else{
        if (splited[0].size() > 15){
            std::string rp = ERR_ERRONEUSNICKNAME(user.ip, "NICK");
            if (send(user.c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
            return 1;
        }
        else{
                user.nickname = splited[0];
                return 0;
        }
    }
    return 0;
}

void nick(std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    (void)chan;
    if (cmd.command_arg.size() > 0)
    {
        std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.nickname));
        if (it == clients.end())
        {
            if (validate_Nick(cmd.nickname, client))
            {
                return;
            }
            std::string nickmsg = NICK_MSG(client.nickname, client.username, client.ip, cmd.nickname);
            broadcast2(clients, nickmsg);
        }
        else
        {
            std::string nickinuse = ERR_NICKINUSE(client.nickname, cmd.nickname);
            if (send(client.c_fd, nickinuse.c_str(), nickinuse.size(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    else
    {
        std::string moreparams = ERR_NONICKNAMEGIVEN(client.nickname);
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

void prv(std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    size_t i = 0;
    size_t j;
    if (cmd.command_arg.size() > 0)
    {
        while (i < cmd.privmsg_list.size())
        {
            if (cmd.privmsg_list[i][0] == '#')
            {
                std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.privmsg_list[i]));
                if (it != chan.end())
                {
                    j = 0;
                    while (j < it->user_list.size())
                    {
                        if (it->user_list[j]->nickname != client.nickname)
                        {
                            std::string prvmsg = PRIVMSG(client.nickname, client.username, client.ip, it->name, cmd.privmessage);
                            if (send(it->user_list[j]->c_fd, prvmsg.c_str(), prvmsg.size(), 0) == -1)
                            {
                                perror("send");
                            }
                        }
                        j++;
                    }
                }
                else
                {
                    std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.privmsg_list[i]);
                    if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            else
            {
                std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.privmsg_list[i]));
                if (it != clients.end())
                {
                    std::string privmsg = PRIVMSG(client.nickname, client.username, client.ip, it->nickname, cmd.privmessage);
                    if (send(it->c_fd, privmsg.c_str(), privmsg.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
                else
                {
                    std::string nosuchnick = ERR_NOSUCHNICK(client.nickname, cmd.privmsg_list[i]);
                    if (send(client.c_fd, nosuchnick.c_str(), nosuchnick.size(), 0) == -1)
                    {
                        perror("send");
                    }
                }
            }
            i++;
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "PRIVMSG");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

// void printallnicknames(std::vector<Clientx> &clients)
// {
//     size_t i = 0;
//     while (i < clients.size())
//     {
//         std::cout << clients[i].nickname << std::endl;
//         i++;
//     }
// }

// void printallchannels(std::vector<Channel> &chan)
// {
//     size_t i = 0;
//     while (i < chan.size())
//     {
//         std::cout << chan[i].name << std::endl;
//         i++;
//     }
// }

std::string tostr(int n)
{
    std::string str;
    std::stringstream ss;
    ss << n;
    ss >> str;
    return str;
}

void topicf(std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    (void)clients;

    if (cmd.command_arg.size() > 0)
    {
            // if (cmd.channel[i].first[0] != '#')
            //     cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
            if (it != chan.end())
            {
                if (!it->is_user(client.nickname))
                {
                    std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, it->name);
                    // write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                    if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                std::cout << "|" << cmd.topic << "|" << std::endl;
                if (cmd.command_arg.size() == 1 && !it->topic.empty())
                {
                    std::cout<<"dkhel cond 1"<<std::endl;
                    std::string topicmsg = RPL_TOPIC(client.nickname, it->name, it->topic);
                    // write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                    if (send(client.c_fd, topicmsg.c_str(), topicmsg.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    std::string topictime = RPL_TOPICWHOTIME(client.nickname, it->name, it->topicsetter, tostr(it->topic_time));
                    // write(client.c_fd, topictime.c_str(), topictime.size());
                    if (send(client.c_fd, topictime.c_str(), topictime.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                else if (cmd.command_arg.size() == 1 && it->topic.empty())
                {
                    std::cout<<"dkhel cond 2"<<std::endl;
                    std::string topicmsg = RPL_NOTOPIC(client.nickname, it->name);
                    // write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                    if (send(client.c_fd, topicmsg.c_str(), topicmsg.size(), 0) == -1)
                    {
                        perror("send");
                    }
                    return;
                }
                if (cmd.command_arg.size() > 1)
                {
                    if (it->mode.find('t') != std::string::npos)
                    {
                        if (it->is_operator(client.nickname))
                        {
                            std::string topicmsg = RPL_TOPIC(client.nickname, it->name, cmd.topic);
                            // write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                            broadcast(it->user_list, topicmsg);
                            it->topic = cmd.topic;
                            it->topicsetter = client.nickname;
                            it->topic_time = time(NULL);
                            return;
                        }
                        else if (!it->is_operator(client.nickname) && it->is_user(client.nickname))
                        {
                            std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, it->name);
                            // write(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size());
                            if (send(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size(), 0) == -1)
                            {
                                perror("send");
                            }
                            return;
                        }
                    }
                    if (it->is_user(client.nickname))
                    {
                        std::string topicmsg = RPL_TOPIC(client.nickname, it->name, cmd.topic);
                        // write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                        broadcast(it->user_list, topicmsg);
                        it->topic = cmd.topic;
                        it->topic_time = time(NULL);
                        return;
                    }
                    else
                    {
                        std::string notonchannel = ERR_CHANOPRIVSNEEDED(client.nickname, it->name);
                        // write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                        if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                        {
                            perror("send");
                        }
                    }
                }
            }
            else
            {
                std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, it->name);
                // write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
                if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
                {
                    perror("send");
                }
            }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "TOPIC");
        // write(client.c_fd, moreparams.c_str(), moreparams.size());
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
    }
}

std::string removeDuplicates(std::string &str)
{
    bool used[256];
    for (size_t i = 0; i < 256; ++i)
    {
        used[i] = false;
    }
    std::string res;
    for (size_t i = 0; i < str.size(); i++)
    {
        int c = str[i];
        if (!used[c])
        {
            res += c;
            used[c] = true;
        }
    }
    if (str[0] == '+' && !str[1])
        res = "";
    return res;
}

// std::string modeupdate(std::string &new_, std::string &old, std::vector<std::string> &param)
// {
//     (void)param;
//     std::string res;
//     size_t i = 0;
//     while(old[i])
//     {
//         if (new_.find(old[i]) == std::string::npos && old[i] != '+' && old[i] != '-') // not found
//         {
//             res += "-";
//             res += old[i];
//         }
//         i++;
//     }
//     i = 0;
//     while (new_[i])
//     {
//         if (old.find(new_[i]) == std::string::npos && new_[i] != '+' && new_[i] != '-')
//         {
//             res += "+";
//             res += new_[i];
//         }
//         i++;
//     }
//     // i = 0;
//     // while(i < param.size())
//     // {
//     //     if (res.find('+') != std::string::npos && res.find('o') != std::string::npos)
//     //         res += ' ' + param[i];
//     //     if (res.find('-' != std::string::npos) && res.find('o') != std::string::npos)
//     //         res += ' ' + param[i];
//     //     if (res.find('+') != std::string::npos && res.find('l') != std::string::npos)
//     //         res += ' ' + param[i];
//     //     if (res.find('-' != std::string::npos) && res.find('l') != std::string::npos)
//     //         res += ' ' + param[i];
//     //     i++;
//     // }
//     return res;
// }

std::string print_update(std::string const &oldstring, std::string &newstring, std::string cmd_arg);

void modef(std::vector<Channel> &chan, Command &cmd, Clientx &client)
{
    bool removeOp = 0;
    bool flag_err = 0;
    if (cmd.command_arg.size() == 0)
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE");
        if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
        {
            perror("send");
        }
        return;
    }
    if (cmd.command_arg.size() == 1)
    {
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
        if (it != chan.end())
        {
            it->old_mode = it->mode;
            if (it->is_user(client.nickname))
            {
                std::string mode = RPL_CHANNELMODEIS(client.nickname, cmd.channel[0].first, it->mode);
                if (send(client.c_fd, mode.c_str(), mode.size(), 0) == -1)
                {
                    perror("send");
                }
                return;
            }
            else
            {
                std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[0].first);
                if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                {
                    perror("send");
                }
                return;
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
            if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
            {
                perror("send");
            }
        }
    }
    if (cmd.command_arg.size() > 1)
    {
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
        if (it != chan.end())
        {
            it->old_mode = it->mode;
            if (!it->is_user(client.nickname))
            {
                std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[0].first);
                if (send(client.c_fd, notonchannel.c_str(), notonchannel.size(), 0) == -1)
                {
                    perror("send");
                }
                return;
            }
            size_t i = 0;
            size_t x = 0;
            if (it->is_operator(client.nickname))
            {
                while (i < cmd.mode.size())
                {
                    if (cmd.mode[i].second != 's' && cmd.mode[i].second != 'n')
                    {
                        if (cmd.mode[i].second == 'i') // first +- second char
                        {
                            if (cmd.mode[i].first == '+')
                                it->mode += 'i';
                            else if (cmd.mode[i].first == '-')
                            {
                                size_t pos = it->mode.find('i');
                                if (pos != std::string::npos)
                                    it->mode.erase(it->mode.find('i'));
                            }
                        }
                        if (cmd.mode[i].second == 't')
                        {
                            if (cmd.mode[i].first == '+')
                                it->mode += 't';
                            else if (cmd.mode[i].first == '-')
                            {
                                size_t pos = it->mode.find('t');
                                if (pos != std::string::npos)
                                    it->mode.erase(it->mode.find('t'), 1);
                            }
                        }
                        if (cmd.mode[i].second == 'l')
                        {
                            if (cmd.mode[i].first == '+')
                            {
                                if (cmd.mode_args.size() > x)
                                {
                                    int max_users = atoi(cmd.mode_args[x].c_str());
                                    // bool b = false;
                                    if (max_users > 0)
                                    {
                                        it->mode += 'l';
                                        it->max_users = atoi(cmd.mode_args[x].c_str());
                                        it->mode_param += cmd.mode_args[x] + ' ';
                                    }
                                }
                                else
                                {
                                    flag_err = 1;
                                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +l");
                                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                                x++;
                            }
                            else if (cmd.mode[i].first == '-')
                            {
                                size_t pos = it->mode.find(cmd.mode[i].second);
                                if (pos != std::string::npos)
                                    it->mode.erase(it->mode.find(cmd.mode[i].second));
                            }
                        }
                        if (cmd.mode[i].second == 'o')
                        {
                            if (cmd.mode[i].first == '+')
                            {
                                if (cmd.mode_args.size() > x)
                                {
                                    std::vector<Clientx *>::iterator it2 = std::find(it->user_list.begin(), it->user_list.end(), it->nickcmp(cmd.mode_args[x]));
                                    if (it2 != it->user_list.end())
                                    {
                                        if (!it->is_operator((*it2)->nickname))
                                        {
                                            it->mode += cmd.mode[i].second;
                                            it->mode_param += cmd.mode_args[x] + ' ';
                                            it->add_operator(*(*it2));
                                        }
                                    }
                                    else
                                    {
                                        flag_err = 1;
                                        std::string user_not_found = ERR_NOSUCHNICK(client.nickname, cmd.mode_args[x]);
                                        if (send(client.c_fd, user_not_found.c_str(), user_not_found.size(), 0) == -1)
                                        {
                                            perror("send");
                                        }
                                    }
                                }
                                else
                                {
                                    flag_err = 1;
                                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +o");
                                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                            else if (cmd.mode[i].first == '-')
                            {
                                if (cmd.mode_args.size() > x)
                                {
                                    std::vector<Clientx *>::iterator it2 = std::find(it->user_list.begin(), it->user_list.end(), it->nickcmp(cmd.mode_args[x]));
                                    if (it2 != it->user_list.end())
                                    {
                                        // it->mode += cmd.mode[i].second;
                                        //     it->add_operator(*(*it2));
                                        // size_t found = it->mode.find('o');
                                        // if ()
                                        // it->mode += cmd.mode[i].second;


                                        it->mode_param += cmd.mode_args[x] + ' ';
                                        it->remove_operator(cmd.mode_args[x]);
                                        removeOp = 1;
                                        // std::string modeup = MODE_MSG(client.nickname, client.username, client.ip, it->name, "-o", it->mode_param);

                                        // size_t found = it->mode.find(cmd.mode_args[x]);
                                        // if (found != std::string::npos)
                                        // {
                                        //     it->mode.erase(found, cmd.mode_args[x].length());
                                        //     it->mode_param += cmd.mode_args[x] + ' ';
                                        //     it->remove_operator(cmd.mode_args[x]);
                                        //     std::cout<<"MODE param -o =====> "<<it->mode_param<<std::endl;
                                        // }
                                    }
                                    else
                                    {
                                        flag_err = 1;
                                        std::string user_not_found = ERR_NOSUCHNICK(client.nickname, cmd.mode_args[x]);
                                        if (send(client.c_fd, user_not_found.c_str(), user_not_found.size(), 0) == -1)
                                        {
                                            perror("send");
                                        }
                                    }
                                }
                                else
                                {
                                    flag_err = 1;
                                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE -o");
                                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                            x++;
                        }
                        if (cmd.mode[i].second == 'k')
                        {
                            if (cmd.mode[i].first == '+')
                            {
                                if (cmd.mode_args.size() > x)
                                {
                                    if (!it->k_flag)
                                    {
                                        it->k_flag = true;
                                        it->mode_param += cmd.mode_args[x] + ' ';
                                        it->mode += cmd.mode[i].second;
                                        it->pwd = cmd.mode_args[x];
                                    }
                                    else
                                    {
                                        flag_err = 1;
                                        std::string alreadyset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                        if (send(client.c_fd, alreadyset.c_str(), alreadyset.size(), 0) == -1)
                                        {
                                            perror("send");
                                        }
                                    }
                                }
                                else
                                {
                                    flag_err = 1;
                                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +k");
                                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                            else if (cmd.mode[i].first == '-')
                            {
                                if (!it->k_flag)
                                {
                                    flag_err = 1;
                                    std::string keynotset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                    if (send(client.c_fd, keynotset.c_str(), keynotset.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                                else if (cmd.mode_args.size() > x)
                                {
                                    // size_t j = 0;
                                    // for (; j < cmd.mode_args.size(); j++)
                                    // {
                                    //     std::cout<<"cmd.mode_args[i] = "<<cmd.mode_args[i]<<std::endl;
                                    //     if (cmd.mode_args[i][j] == ' ')
                                    //         break ;
                                    // }
                                    // std::cout<<"j ==>"<<j<<std::endl;
                                    size_t pos = it->mode.find(cmd.mode[i].second);
                                    size_t pos2 = it->mode_param.find(cmd.mode_args[x]);
                                    if (pos != std::string::npos)
                                    {
                                        it->pwd.clear();
                                        it->pwd = "";
                                        it->k_flag = false;
                                        it->mode.erase(it->mode.find(cmd.mode[i].second));
                                    }
                                    else
                                    {
                                        flag_err = 1;
                                        std::string keynotset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                        if (send(client.c_fd, keynotset.c_str(), keynotset.size(), 0) == -1)
                                        {
                                            perror("send");
                                        }
                                    }
                                    if (it->pwd == cmd.mode_args[x])
                                    {
                                        it->pwd.clear();
                                        it->pwd = "";
                                        if (pos2 != std::string::npos)
                                        {
                                            it->mode_param.erase(it->mode_param.find(cmd.mode_args[x]), cmd.mode_args[x].size());
                                            it->k_flag = false;
                                        }
                                    }
                                }
                                else
                                {
                                    flag_err = 1;
                                    std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE -k");
                                    if (send(client.c_fd, moreparams.c_str(), moreparams.size(), 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                            x++;
                        }
                        if (cmd.mode[i].second != 'i' && cmd.mode[i].second != 't' && cmd.mode[i].second != 'l' && cmd.mode[i].second != 'o' && cmd.mode[i].second != 'k')
                        {
                            flag_err = 1;
                            std::string unknownmode = ERR_UNKNOWNMODE(client.nickname, cmd.mode[i].second);
                            if (send(client.c_fd, unknownmode.c_str(), unknownmode.size(), 0) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                    i++;
                }
                it->mode = removeDuplicates(it->mode);
                std::string newmode = print_update(it->old_mode, it->mode, cmd.command_arg[1]) + ' ';
                for(size_t i = 0; i < cmd.mode.size(); i++)
                if(removeOp)
                {
                    if(old_arg.compare(cmd.command_arg[1]) == 0)
                    {
                        removeOp = 0;
                        old_arg = cmd.command_arg[1];
                        it->mode_param.clear();
                        it->mode_param = "";
                        return;
                    }
                    removeOp = 0;
                    std::string op = "-";
                    for(size_t i = 0; i < cmd.mode.size(); i++)
                        op += "o";
                    op += " ";
                    std::string modeup = MODE_MSG(client.nickname, client.username, client.ip, it->name, op, it->mode_param);
                    broadcast(it->user_list, modeup);
                    it->mode_param.clear();
                    it->mode_param = "";
                }
                else if ((newmode.find("o") != std::string::npos || newmode.find("i") != std::string::npos || newmode.find("t") != std::string::npos || newmode.find("l") != std::string::npos || newmode.find("k") != std::string::npos) && !flag_err)
                {
                    std::string param = "";
                    newmode = trim_(newmode);
                    param = newmode + " ";
                    std::string modeup = MODE_MSG(client.nickname, client.username, client.ip, it->name, param, it->mode_param);
                    broadcast(it->user_list, modeup);
                    it->mode_param.clear();
                    it->mode_param = "";

                    flag_err = 0;
                    old_arg = cmd.command_arg[1];
                    return;
                }
                else
                {
                    old_arg = cmd.command_arg[1];
                    it->mode_param.clear();
                    it->mode_param = "";
                    return;
                }
                old_arg = cmd.command_arg[1];
            }
            else
            {
                std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[0].first);
                if (send(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size(), 0) == -1)
                {
                    perror("send");
                }
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
            if (send(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size(), 0) == -1)
            {
                perror("send");
            }
        }
    }
}

std::string print_update(std::string const &oldstring, std::string &newstring, std::string cmd_arg)
{
    std::string result;
    bool sign = false;
    newstring = trim_(newstring);

    if(old_arg.find(cmd_arg) != std::string::npos && old_arg.size() == cmd_arg.size())
        return "";
    if(oldstring.compare(newstring) == 0  && newstring.compare("+t") == 0 )
        return newstring;
    else if(oldstring.compare(newstring) == 0 && cmd_arg.find("l") != std::string::npos)
        return "+l";
    else if(cmd_arg.find("+o") != std::string::npos)
    {
        return cmd_arg;
    }
    else if(oldstring.compare(newstring) == 0)
        return "";

    for (size_t i = 0; i < oldstring.length(); ++i)
    {
        char c = oldstring[i];
        if (c == ' ')
            break;
        if ((c != '+') && newstring.find(c) == std::string::npos)
        {
            if (!sign)
            {
                sign = true;
                result += "-";
            }
            result += c;
        }
    }

    sign = false;
    for (size_t i = 0; i < newstring.length(); ++i)
    {
        char c = newstring[i];
        if (c == ' ')
            break;
        if ((c != '+') && oldstring.find(c) == std::string::npos)
        {
            if (!sign)
            {
                sign = true;
                result += "+";
            }
            result += c;
        }
    }

    return result;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append(contents, totalSize);
    return totalSize;
}

size_t findNthOccurrence(const std::string &str, char ch, size_t n)
{
    size_t count = 0;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == ch)
        {
            ++count;
            if (count == n)
            {
                return i;
            }
        }
    }
    // Return -1 if the character is not found n times
    return -1;
}

void bot(std::vector<Channel> &chan, Command &cmd, Clientx &client)
{
    (void)chan;
    if (cmd.bot_arg.empty())
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "BOT");
        if (send(client.c_fd, moreparams.c_str(), moreparams.length(), 0) == -1)
        {
            perror("send");
        }
        return;
    }   
    std::string arg[3] = {"time", "quote", "fact"};
    int i = 0;
    while (i < 3)
    {
        if (arg[i] == cmd.bot_arg)
            break;
        i++;
    }

    switch (i)
    {
    case 0:
    {
        std::time_t currentTime = std::time(0);
        std::string timeString = std::ctime(&currentTime);
        std::string welcomeMsg = "BOT: Current time: " + timeString;
        std::string motod = RPL_MOTD(client.nickname, welcomeMsg);
        if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
        {
            perror("send");
        }
        break;
    }
    case 1:
    {
        CURL *curl; // Create a CURL pointer
        std::string data; // Create a string to store the data
        curl = curl_easy_init(); 
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.quotable.io/random");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
            CURLcode res = curl_easy_perform(curl); 
            if (res != CURLE_OK)
            {
                // Handle error
            }
            curl_easy_cleanup(curl);
        }
        size_t spos = findNthOccurrence(data, '"', 7);
        size_t epos = findNthOccurrence(data, '"', 8);
        data = data.substr(spos + 1, (epos - spos) - 1);
        data += '\n';
        std::string motod = RPL_MOTD(client.nickname, data);
        if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
        {
            perror("send");
        }
        break;
    }
    case 2:
    {
        CURL *curl;
        std::string data;
        curl = curl_easy_init();
        if (curl)
        {
            // URL for fetching a random fact
            curl_easy_setopt(curl, CURLOPT_URL, "https://uselessfacts.jsph.pl/random");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                // Handle error
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            curl_easy_cleanup(curl);
        }
        std::string start_tag = "<blockquote>";
        std::string end_tag = "</blockquote>";

        // Find the starting position of the quote
        size_t start_pos = data.find(start_tag);
        if (start_pos == std::string::npos)
        {
            std::cerr << "Error: Could not find starting quote tag." << std::endl;
            return;
        }

        // Find the ending position of the quote
        size_t end_pos = data.find(end_tag, start_pos + start_tag.length());
        if (end_pos == std::string::npos)
        {
            std::cerr << "Error: Could not find ending quote tag." << std::endl;
            return;
        }

        // Extract the quote text
        std::string quote = data.substr(start_pos + start_tag.length(), end_pos - (start_pos + start_tag.length()));

        // Remove any leading/trailing whitespace
        quote.erase(0, quote.find_first_not_of(" \t"));
        quote.erase(quote.find_last_not_of(" \t") + 1);

        quote += '\n';
        std::string motod = RPL_MOTD(client.nickname, quote);
        if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
        {
            perror("send");
        }
        break;
    }
    default:
        std::string rp = ERR_UNKNOWNCOMMAND(client.ip, cmd.bot_arg);
        if (send(client.c_fd, rp.c_str(), rp.length(), 0) == -1)
        {
            perror("send");
        }
    }
}