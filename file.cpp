#include"Channel.hpp"
#include"Commands.hpp"
#include"server.hpp"

class Clientx;


// void kick_broadcast(std::vector<Channel>::iterator it ,Clientx  &clients,std::string kicked, std::string reason)
// {
//     size_t i = 0;
//     std::string kickstr;
//     while (i < it->user_list.size())
//     {
//         std::string kickmsg = KICK_MSG(clients.nickname, clients.ip, it->name, kicked, reason);
//         write(it->user_list[i]->c_fd, kickstr.c_str(), kickstr.size());
//         i++;
//     }
// }

void broadcast(std::vector<Clientx *> &clients, std::string msg)
{
    size_t i = 0;
    while (i < clients.size())
    {
        write(clients[i]->c_fd, msg.c_str(), msg.size());
        i++;
    }
}

void broadcast2(std::vector<Clientx > &clients, std::string msg)
{
    size_t i = 0;
    while (i < clients.size())
    {
        write(clients[i].c_fd, msg.c_str(), msg.size());
        i++;
    }
}

// void broadcast(std::vector<Clientx *> &clients, std::string msg)
// {
//     size_t i = 0;
//     while (i < clients.size())
//     {
//         write(clients[i]->c_fd, msg.c_str(), msg.size());
//         i++;
//     }
// }

std::vector<Clientx>::iterator find_client(std::vector<Clientx> &clients,const Clientx &client);

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
    while(i < channel.user_list.size())
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

void join(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    size_t i = 0;
    if (cmd.command_arg.size() > 0)
    {
        while(i < cmd.channel.size())
        {
            if (cmd.channel[i].first[0] != '#')
                cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
            if (it != chan.end())
            {
                if (it->user_list.size() > it->max_users && it->mode.find('l') != std::string::npos)
                {
                    std::string maxusers = ERR_CHANNELISFULL(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, maxusers.c_str(), maxusers.size());
                    return ;
                }
                if (it->mode.find('i') != std::string::npos && it->is_invite(client.nickname) == false)
                {
                    std::string inviteonly = ERR_INVITEONLYCHAN(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, inviteonly.c_str(), inviteonly.size());
                    return ;
                }
                if (it->mode.find('k') != std::string::npos && it->pwd != cmd.channel[i].second)
                {
                    std::string badpassword = ERR_BADCHANNELKEY(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, badpassword.c_str(), badpassword.size());
                    return ;
                }
                if (!it->is_user(client.nickname))
                {
                    it->add_user(client);
                    std::string j_rpl = JOIN_SUCC(client.nickname, client.username, client.ip, cmd.channel[i].first);
                    broadcast(it->user_list, j_rpl);
                    j_rpl = RPL_NAMERPLY(client.nickname, cmd.channel[i].first, usersonchan(*it));
                    write(client.c_fd, j_rpl.c_str(), j_rpl.size());
                    j_rpl = RPL_ENDOFNAMES(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, j_rpl.c_str(), j_rpl.size());
                }
            }
            else
            {
                //broadcast !!
                Channel tmp(cmd.channel[i].first);
                chan.push_back(tmp);
                chan[chan.size() - 1].add_user(client);
                chan[chan.size() - 1].add_operator(client);
                std::string j_rpl = JOIN_SUCC(client.nickname, client.username, client.ip, cmd.channel[i].first);
                write(client.c_fd, j_rpl.c_str(), j_rpl.size());
                j_rpl = RPL_NAMERPLY(client.nickname, cmd.channel[i].first, usersonchan(chan[chan.size() - 1]));
                write(client.c_fd, j_rpl.c_str(), j_rpl.size());
                j_rpl = RPL_ENDOFNAMES(client.nickname, cmd.channel[i].first);
                write(client.c_fd, j_rpl.c_str(), j_rpl.size());
            }
            i++;
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "JOIN");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
}



void    broadcast_kick(std::vector<Channel>::iterator it, Clientx &user, std::string &cli, std::string &reason)
{
    for (size_t i = 0; i < it->user_list.size(); ++i)
    {
        std::string syn = KICK_MSG(user.nickname, user.ip, it->name, cli, reason);
        write(it->user_list[i]->c_fd, syn.c_str(), syn.size());
    }
}

void kick(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    size_t i = 0;
    // size_t x = 0;
    size_t j = 0;
    if (cmd.command_arg.size() > 0)
    {
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
            if (it != chan.end())
            {
                if (it->is_operator(client.nickname))
                {
                    while(i < cmd.users.size())
                    {
                        std::cout<<it->is_user(cmd.users[i])<<std::endl;
                        std::cout<<cmd.users[i]<<std::endl;
                        if (it->is_user(cmd.users[i]))
                        {
                            broadcast_kick(it, client, cmd.users[i], cmd.comment);
                            // write(client.c_fd, kickmsg.c_str(), kickmsg.size());
                            // kick_broadcast(it, client, cmd.users[0], cmd.comment);
                            it->remove_operator(cmd.users[i]);
                            it->remove_user(cmd.users[i]);
                        }
                        else
                        {
                            std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[0].first);
                            write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                        }
                        i++;
                    }
                }
                else
                {
                    std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size());
                }
            }
            else
            {
                std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
                write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
            }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "KICK");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
}

void part(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    if (cmd.command_arg.size() < 1)
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "PART");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
    size_t i = 0;
    while(i < cmd.channel.size())
    {
        if (cmd.channel[i].first[0] != '#')
             cmd.channel[i].first = '#' + cmd.channel[i].first;
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
        if (it != chan.end())
        {
            if (it->is_user(client.nickname))
            {
                std::string partmsg = PART_MSG(client.nickname, client.username, client.ip, cmd.channel[i].first, cmd.comment);
                broadcast(it->user_list ,partmsg);
                it->remove_operator(client.nickname);
                it->remove_user(client.nickname);
                if (it->user_list.size() == 0)
                    chan.erase(it);
            }
            else
            {
                std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[i].first);
                write(client.c_fd, notonchannel.c_str(), notonchannel.size());
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
            write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
        }
        i++;
    }
}



// void clearchannel(std::vector<Channel>&chan, Clientx &client)
// {
//     size_t i = 0;
//     while(i < chan.size())
//     {
//         if (chan[i].is_user(client.nickname))
//         {
//             if (chan[i].user_list.size() == 0)
//                 chan.erase(chan.begin() + i);
//         }
//         i++;
//     }
// }

void privmsg(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    size_t i = 0;
    if (cmd.command_arg.size() > 0)
    {
        while(i < cmd.channel.size())
        {
            if (cmd.channel[i].first[0] != '#')
                cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), cmd.channel[i].first);
            if (it != chan.end())
            {
                if (it->is_user(client.nickname))
                {
                    std::cout<<client.nickname<<" sent a message to "<<it->name<<" : "<<cmd.command_arg[0]<<std::endl;
                }
                else
                    std::cout<<client.nickname<<" not found on channel!"<<std::endl;
            }
            else
                std::cout<<"Channel not found!"<<std::endl;
            i++;
        }
    }
}

void invite(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::vector<Clientx> &clients)
{
    // size_t i = 0;
    if (cmd.command_arg.size() > 0)
    {
        if (cmd.channel[0].first[0] != '#')
            cmd.channel[0].first = '#' + cmd.channel[0].first;
        std::vector<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.command_arg[0]));
        if (it != clients.end())
        {
            std::vector<Channel>::iterator it2 = std::find(chan.begin(), chan.end(), cmd.channel[0].first);
            if (it2 != chan.end())
            {
                if (it2->is_operator(client.nickname))
                {
                    std::string sendinv = INVITE_MSG(client.nickname, client.username, client.ip, cmd.command_arg[0], cmd.channel[0].first);
                    write(it->c_fd, sendinv.c_str(), sendinv.size());
                    it2->inv_list.push_back(&*it);
                }
                else
                {
                    std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[0].first);
                    write(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size());
                }
            }
            else
            {
                std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
                write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
            }
        }
    }
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "INVITE");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
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

void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    (void)chan;
    std::string quitmsg = QUIT_MSG(client.nickname, client.username, client.ip, cmd.comment);
    write(client.c_fd, quitmsg.c_str(), quitmsg.size());
    close(client.c_fd);
    // server.clients.erase(std::find(server.clients.begin(), server.clients.end(), client));
}


void nick(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::vector<Clientx> &clients)
{
    // size_t i = 0;
    (void)chan;
    if (cmd.command_arg.size() > 0)
    {
        std::vector<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.nickname));
        // if (it != clients.end() && it->c_fd == client.c_fd)
        // {
        //     puts("ignore change nick");
        //     return ;
        // }
        if (it == clients.end())
        {
            std::string nickmsg = NICK_MSG(client.nickname, client.username, client.ip, cmd.nickname);
            broadcast2(clients, nickmsg);
            // write(client.c_fd, nickmsg.c_str(), nickmsg.size());
            client.nickname = cmd.nickname;
        }
        else
        {
            std::string nickinuse = ERR_NICKINUSE(client.nickname, cmd.nickname);
            // write(client.c_fd, nickinuse.c_str(), nickinuse.size());
            broadcast2(clients,nickinuse);
        }
    }
    else
    {
        std::string moreparams = ERR_NONICKNAMEGIVEN(client.nickname);
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
}

void prv(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::vector<Clientx> &clients)
{
    size_t i = 0;
    size_t j;
    if (cmd.command_arg.size() > 0)
    {
        while(i < cmd.privmsg_list.size())
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
                            write(it->user_list[j]->c_fd, prvmsg.c_str(), prvmsg.size());
                        }
                        j++;
                    }
                }
                else
                {
                    std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.privmsg_list[i]);
                    write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
                }
            }
            else
            {   
                std::vector<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.privmsg_list[i]));
                if (it != clients.end())
                {
                    std::string privmsg = PRIVMSG(client.nickname, client.username, client.ip, cmd.privmsg_list[i], cmd.privmessage);
                    write(it->c_fd, privmsg.c_str(), privmsg.size());
                }
                else
                {
                    std::string nosuchnick = ERR_NOSUCHNICK(client.nickname, cmd.privmsg_list[i]);
                    write(client.c_fd, nosuchnick.c_str(), nosuchnick.size());
                }
            }
            i++;
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "PRIVMSG");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
}

void printallnicknames(std::vector<Clientx> &clients)
{
    size_t i = 0;
    while (i < clients.size())
    {
        std::cout<<clients[i].nickname<<std::endl;
        i++;
    }
}

void printallchannels(std::vector<Channel> &chan)
{
    size_t i = 0;
    while (i < chan.size())
    {
        std::cout<<chan[i].name<<std::endl;
        i++;
    }
}

std::string tostr(int n)
{
    std::string str;
    std::stringstream ss;
    ss << n;
    ss >> str;
    return str;
}

void topicf(std::vector<Channel>&chan, Command &cmd, Clientx &client , std::vector<Clientx>&clients)
{
    (void)clients;
    size_t i = 0;
    if (cmd.command_arg.size() > 0)
    {
        while(i < cmd.channel.size())
        {
            if (cmd.channel[i].first[0] != '#')
                cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
            if (it != chan.end())
            {
                std::cout<<cmd.topic<<std::endl;
                if (cmd.command_arg.size() == 1 && chan[i].topic.empty())
                {
                    std::string topicmsg = RPL_NOTOPIC(client.nickname, cmd.channel[0].first);
                    write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                    return ;
                }
                if (cmd.command_arg.size() == 1 && !chan[i].topic.empty())
                {
                    std::string topicmsg = RPL_TOPIC(client.nickname, cmd.channel[0].first, it->topic);
                    write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                    std::string topictime = RPL_TOPICWHOTIME(client.nickname, it->name, it->topicsetter,tostr(it->topic_time));
                    write(client.c_fd, topictime.c_str(), topictime.size());
                    return ;
                }
                if (cmd.command_arg.size() > 1)
                {
                    if (it->mode.find('t') != std::string::npos)
                    {
                        if (it->is_operator(client.nickname))
                        {
                            std::string topicmsg = RPL_TOPIC(client.nickname, cmd.channel[i].first, cmd.topic);
                            write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                            it->topic = cmd.topic;
                            it->topicsetter = client.nickname;
                            it->topic_time = time(NULL);
                            return ;
                        }
                        else if (!it->is_operator(client.nickname) && it->is_user(client.nickname))
                        {
                            std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[i].first);
                            write(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size());
                            return ;
                        }
                    }
                    if (it->is_user(client.nickname))
                    {
                        std::string topicmsg = RPL_TOPIC(client.nickname, cmd.channel[i].first, cmd.topic);
                        write(client.c_fd, topicmsg.c_str(), topicmsg.size());
                        it->topic = cmd.topic;
                        it->topic_time = time(NULL);
                        return ;
                    }
                    else
                    {
                        std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[i].first);
                        write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                    }
                }
            }
            else
            {
                std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[i].first);
                write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
            }
            i++;
        }
    }
    else
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "TOPIC");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
    }
}

std::string removeDuplicates(std::string &str)
{
    bool used[256];
    for (size_t i = 0; i < 256; ++i) {
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


std::string print_update(std::string const &oldstring, std::string const &newstring);

void modef(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    if (cmd.command_arg.size() == 0)
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE");
        write(client.c_fd, moreparams.c_str(), moreparams.size());
        return ;
    }
    if (cmd.command_arg.size()  == 1)
    {
        std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[0].first));
        if (it != chan.end())
        {
            it->old_mode = it->mode;
            if (it->is_user(client.nickname))
            {
                std::string mode = RPL_CHANNELMODEIS(client.nickname, cmd.channel[0].first, it->mode);
                write(client.c_fd, mode.c_str(), mode.size());
            }
            else
            {
                std::string notonchannel = ERR_NOTONCHANNEL(client.nickname, cmd.channel[0].first);
                write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                return ;
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
            write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
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
                write(client.c_fd, notonchannel.c_str(), notonchannel.size());
                return ;
            }
            size_t i = 0;
            size_t x = 0;
            if (it->is_operator(client.nickname))
            {
                while (i < cmd.mode.size())
                {
                    if (cmd.mode[i].second != 's' && cmd.mode[i].second != 'n')
                    {
                    if (cmd.mode[i].second == 'i')
                    {
                        if (cmd.mode[i].first == '+')
                            it->mode += cmd.mode[i].second;
                        else if (cmd.mode[i].first == '-')
                        {
                            std::cout<<"value = "<<it->mode.find(cmd.mode[i].second)<<std::endl;
                            size_t pos = it->mode.find(cmd.mode[i].second);
                            if (pos != std::string::npos)
                                it->mode.erase(it->mode.find(cmd.mode[i].second));
                        }
                    }
                    if (cmd.mode[i].second == 't')
                    {
                        std::cout<<cmd.mode[i].first<<std::endl;
                        if (cmd.mode[i].first == '+')
                            it->mode += cmd.mode[i].second;
                        else if (cmd.mode[i].first == '-')
                        {
                            size_t pos = it->mode.find(cmd.mode[i].second);
                            if (pos != std::string::npos)
                                it->mode.erase(it->mode.find(cmd.mode[i].second), 1);
                        }
                    }
                    if (cmd.mode[i].second == 'l')
                    {
                        if (cmd.mode[i].first == '+')
                        {
                            if (cmd.mode_args.size() > x)
                            {
                                it->mode += cmd.mode[i].second;
                                it->max_users = atoi(cmd.mode_args[x].c_str());
                                it->mode_param += cmd.mode_args[x] + ' ';
                            }
                            else
                            {
                                std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +l");
                                write(client.c_fd, moreparams.c_str(), moreparams.size());
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
                                    it->mode += cmd.mode[i].second;
                                    it->mode_param += cmd.mode_args[x] + ' ';
                                    it->add_operator(*(*it2));
                                }
                                else
                                {
                                    std::string user_not_found = ERR_NOSUCHNICK(client.nickname, cmd.mode_args[x]);
                                    write(client.c_fd, user_not_found.c_str(), user_not_found.size());
                                }
                            }
                            else
                            {
                                std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +o");
                                write(client.c_fd, moreparams.c_str(), moreparams.size());
                            }
                        }
                        else if (cmd.mode[i].first == '-')
                        {
                            size_t pos = it->mode.find(cmd.mode[i].second);
                            if (pos != std::string::npos)
                                it->mode.erase(it->mode.find(cmd.mode[i].second));
                            if (cmd.mode_args.size() > x)
                            {
                                std::vector<Clientx *>::iterator it2 = std::find(it->user_list.begin(), it->user_list.end(), it->nickcmp(cmd.mode_args[x]));
                                if (it2 != it->user_list.end())
                                {
                                    it->mode_param.erase(it->mode_param.find(cmd.mode_args[x]), cmd.mode_args[x].size());
                                    it->remove_operator(cmd.mode_args[x]);
                                }
                                else
                                {
                                    std::string user_not_found = ERR_NOSUCHNICK(client.nickname, cmd.mode_args[x]);
                                    write(client.c_fd, user_not_found.c_str(), user_not_found.size());
                                }
                            }
                            else
                            {
                                std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE -o");
                                write(client.c_fd, moreparams.c_str(), moreparams.size());
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
                                    std::string alreadyset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                    write(client.c_fd, alreadyset.c_str(), alreadyset.size());
                                }
                            }
                            else
                            {
                                std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE +k");
                                write(client.c_fd, moreparams.c_str(), moreparams.size());
                            }
                        }
                        else if (cmd.mode[i].first == '-')
                        {
                            if (!it->k_flag)
                            {
                                std::string keynotset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                write(client.c_fd, keynotset.c_str(), keynotset.size());
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
                                if (pos != std::string::npos)
                                {
                                    it->pwd = cmd.mode_args[x];
                                    // std::cout << "password is :" << it->pwd << std::endl;
                                    it->mode.erase(it->mode.find(cmd.mode[i].second));
                                }
                                else
                                {
                                    std::string keynotset = ERR_KEYALREADYSET(client.nickname, cmd.channel[0].first);
                                    write(client.c_fd, keynotset.c_str(), keynotset.size());
                                }
                                if (it->pwd == cmd.mode_args[x])
                                {
                                    it->pwd = cmd.mode_args[x];
                                    it->mode_param.erase(it->mode_param.find(cmd.mode_args[x]), cmd.mode_args[x].size());
                                    it->k_flag = false;
                                }
                            }
                            else
                            {
                                std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "MODE -k");
                                write(client.c_fd, moreparams.c_str(), moreparams.size());
                            }
                        }
                        x++;
                    }
                    if (cmd.mode[i].second != 'i' && cmd.mode[i].second != 't' && cmd.mode[i].second != 'l' && cmd.mode[i].second != 'o' && cmd.mode[i].second != 'k')
                    {
                        std::string unknownmode = ERR_UNKNOWNMODE(client.nickname, cmd.mode[i].second);
                        write(client.c_fd, unknownmode.c_str(), unknownmode.size());
                    }
                    }
                    i++;
                }
                it->mode = removeDuplicates(it->mode);
                // std::string res = modeupdate(it->mode, tmp , cmd.mode_args);
                // std::cout<<res<<std::endl;
                std::cout<<"new ==>"<<it->mode<<std::endl;
                std::cout<<"old ==>"<<it->old_mode<<std::endl;
                std::cout<<"update ===> "<<print_update(it->old_mode, it->mode) <<std::endl;
                std::string newmode = print_update(it->old_mode, it->mode ) + " " + it->mode_param;
                if (!newmode.find("oitlk"))
                {
                    std::string mode = RPL_CHANNELMODEIS(client.nickname, cmd.channel[0].first, newmode);
                    broadcast(it->user_list, mode);
                }
            }
            else
            {
                std::string chanoprivsneeded = ERR_CHANOPRIVSNEEDED(client.nickname, cmd.channel[0].first);
                write(client.c_fd, chanoprivsneeded.c_str(), chanoprivsneeded.size());
            }
        }
        else
        {
            std::string nosuchchannel = ERR_NOSUCHCHANNEL(client.nickname, cmd.channel[0].first);
            write(client.c_fd, nosuchchannel.c_str(), nosuchchannel.size());
        }
    }
}


std::string print_update(std::string const &oldstring, std::string const &newstring)
{
    std::string result;
    bool sign = false;

    // Check for characters in oldstring but not in newstring
    for (char c : oldstring) {
        if (c == ' ')
            break ;
        if (c != '+' && newstring.find(c) == std::string::npos) {
            if (!sign) {
                sign = true;
                result += "-";
            }
            result += c;
        }
    }

    sign = false;

    // Check for characters in newstring but not in oldstring
    for (char c : newstring)
    {
        if (c == ' ')
            break ;
        if (c != '+' && oldstring.find(c) == std::string::npos)
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
