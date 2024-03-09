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
                if (it->user_list.size() >= it->max_users && it->mode.find('l') != std::string::npos)
                {
                    std::string maxusers = ERR_CHANNELISFULL(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, maxusers.c_str(), maxusers.size());
                    // broadcast(it->user_list, maxusers);
                    return ;
                }
                if (it->mode.find('i') != std::string::npos && it->is_invite(client.nickname) == false)
                {
                    std::string inviteonly = ERR_INVITEONLYCHAN(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, inviteonly.c_str(), inviteonly.size());
                    // broadcast(it->user_list, inviteonly);
                    return ;
                }
                if (it->mode.find('k') != std::string::npos && it->pwd != cmd.channel[i].second)
                {
                    std::string badpassword = ERR_BADCHANNELKEY(client.nickname, cmd.channel[i].first);
                    write(client.c_fd, badpassword.c_str(), badpassword.size());
                    // broadcast(it->user_list, badpassword);
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
    if (cmd.command_arg.size() > 1)
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
    else
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

// Function to quit a client from a specific channel
// void quit_channel(Channel& channel, int fd)
// {
//     std::vector<Clientx*>::iterator itu = std::find(channel.user_list.begin(), channel.user_list.end(), fd);
//     std::vector<Clientx*>::iterator ito = std::find(channel.op_list.begin(), channel.op_list.end(), fd);
//     std::vector<Clientx*>::iterator iti = std::find(channel.inv_list.begin(), channel.inv_list.end(), fd);

//     if (itu != channel.user_list.end()) {
//         channel.user_list.erase(itu);
//     }
//     if (ito != channel.op_list.end()) {
//         channel.op_list.erase(ito);
//     }
//     if (iti != channel.inv_list.end()) {
//         channel.inv_list.erase(iti);
//     }
// }

// void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::vector<Clientx> &clients);
// {
//     // removing client from channels
//     size_t i = 0;
//     while (i < chan.size()){
//         quit_channel(chan[i], client.c_fd); 
//         i++;
//     }
//     // removing client from clientList
//     std::vector<Clientx>::iterator it = std::find(clients.begin(), clients.end(), client.c_fd);
//     if (it != clients.end()) {
//         clients.erase(it);
//     }
//     close(client.c_fd);
//     if (!clients.empty()) {
//         del_from_pfds(client.c_fd);
//     }
//     std::string quitmsg = QUIT_MSG(client.nickname, client.username, client.ip, cmd.comment);
//     broadcast2(clients, quitmsg);
// }

//bot
//motivational quote
//movie idea
// #include <curl/curl.h>
// void test()
// {
//     CURL *ptr;
//     ptr = curl_easy_init();
// }
// void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client)
// {
//     (void)chan;
//     std::string quitmsg = QUIT_MSG(client.nickname, client.username, client.ip, cmd.comment);
//     write(client.c_fd, quitmsg.c_str(), quitmsg.size());
//     close(client.c_fd);
//     // server.clients.erase(std::find(server.clients.begin(), server.clients.end(), client));
// }

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
            write(client.c_fd, nickinuse.c_str(), nickinuse.size());
            // broadcast2(clients,nickinuse);
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
                return ;
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
                // if (!newmode.find("oitlk"))
                // {
                //     std::string mode = RPL_CHANNELMODEIS(client.nickname, cmd.channel[0].first, newmode);
                //     broadcast(it->user_list, mode);
                // }
                if (newmode.find("o") != std::string::npos || newmode.find("i") != std::string::npos || newmode.find("t") != std::string::npos || newmode.find("l") != std::string::npos || newmode.find("k") != std::string::npos)
                {
                    std::string modeup = MODE_MSG(client.nickname, client.username, client.ip, cmd.channel[0].first, newmode, it->name);
                    broadcast(it->user_list, modeup);
                    return ;
                }
                else
                    return ;
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

size_t WriteCallback(char *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append(contents, totalSize);
    return totalSize;
}

size_t findNthOccurrence(const std::string& str, char ch, size_t n) {
    size_t count = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == ch) {
            ++count;
            if (count == n) {
                return i;
            }
        }
    }
    // Return -1 if the character is not found n times
    return -1;
}


void bot(std::vector<Channel>&chan, Command &cmd, Clientx &client)
{
    if (cmd.bot_arg.empty())
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "BOT");
        if (send(client.c_fd, moreparams.c_str(), moreparams.length(), 0) == -1)
        {
            perror("send");
        }
        return ;
    }
    if (cmd.bot_arg == "time")
    {
        std::time_t currentTime = std::time(0);
        std::string timeString = std::ctime(&currentTime);
        std::string welcomeMsg = "BOT: Current time: " + timeString;
        if (send(client.c_fd, welcomeMsg.c_str(), welcomeMsg.length(), 0) == -1)
        {
            perror("send");
        }
    }
    if (cmd.bot_arg == "quote")
    {
        CURL *curl;
        std::string data;
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
        if (send(client.c_fd, data.c_str(), data.length(), 0) == -1)
        {
            perror("send");
        }
    }
    if (cmd.bot_arg == "fact")
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
        if (start_pos == std::string::npos) {
            std::cerr << "Error: Could not find starting quote tag." << std::endl;
            return ;
        }

        // Find the ending position of the quote
        size_t end_pos = data.find(end_tag, start_pos + start_tag.length());
        if (end_pos == std::string::npos) {
            std::cerr << "Error: Could not find ending quote tag." << std::endl;
            return ;
        }

        // Extract the quote text
        std::string quote = data.substr(start_pos + start_tag.length(), end_pos - (start_pos + start_tag.length()));

        // Remove any leading/trailing whitespace
        quote.erase(0, quote.find_first_not_of(" \t"));
        quote.erase(quote.find_last_not_of(" \t") + 1);

        quote += '\n';
        if (send(client.c_fd, quote.c_str(), quote.length(), 0) == -1)
        {
            perror("send");
        }
    }
    if (cmd.bot_arg == "dad_joke")
    {
        CURL *curl;
        std::string data;
        curl = curl_easy_init();
        if (curl)
        {
            // Use the I Can Haz Dad Joke API endpoint for a random joke
            curl_easy_setopt(curl, CURLOPT_URL, "https://icanhazdadjoke.com/");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
            // Set the header to accept JSON response
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Accept: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                // Handle error
            }
            curl_slist_free_all(headers); // Don't forget to free the headers!
            curl_easy_cleanup(curl);
        }
        std::cout << "joke " << data << std::endl;
        // joke {"id":"0ozAXv4Mmjb","joke":"Why did the tomato blush? Because it saw the salad dressing.","status":200}

        // joke {"id":"NZDlb299Uf","joke":"Where do sheep go to get their hair cut? The baa-baa shop.","status":200}
        // Assuming the JSON response is well-formed and contains "joke" field
        // size_t spos = data.find("\"joke\":");
        // size_t epos = data.find("\"id\":");
        // if (spos != std::string::npos && epos != std::string::npos)
        // {
        //     // Extract the joke text
        //     data = data.substr(spos + 7, epos - spos - 7);
        //     // Trim leading and trailing whitespace and quotes
        //     data.erase(0, data.find_first_not_of(' '));
        //     data.erase(data.find_last_not_of(' ') + 1);
        //     data = data.substr(1, data.size() - 2); // Remove leading and trailing quotes
        std::string start_tag = "joke\":";
        std::string end_tag = ",\"status\":200";

        // Find the starting position of the quote
        size_t start_pos = data.find(start_tag);
        if (start_pos == std::string::npos) {
            std::cerr << "Error: Could not find starting quote tag." << std::endl;
            return ;
        }

        // Find the ending position of the quote
        size_t end_pos = data.find(end_tag, start_pos + start_tag.length());
        if (end_pos == std::string::npos) {
            std::cerr << "Error: Could not find ending quote tag." << std::endl;
            return ;
        }

        // Extract the quote text
        std::string quote = data.substr(start_pos + start_tag.length(), end_pos - (start_pos + start_tag.length()));

        // Remove any leading/trailing whitespace
        quote.erase(0, quote.find_first_not_of(" \t"));
        quote.erase(quote.find_last_not_of(" \t") + 1);

        quote += '\n';
        if (send(client.c_fd, quote.c_str(), quote.length(), 0) == -1)
        {
            perror("send");
        }
    }
    // if (cmd.bot_arg == "joke")
    // {
    //     CURL *curl;
    //     std::string data;
    //     curl = curl_easy_init();
    //     if (curl)
    //     {
    //         curl_easy_setopt(curl, CURLOPT_URL, "https://icanhazdadjoke.com/");
    //         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    //         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    //         CURLcode res = curl_easy_perform(curl);
    //         if (res != CURLE_OK)
    //         {
    //             // Handle error
    //         }
    //         curl_easy_cleanup(curl);
    //     }
    //     // size_t spos = findNthOccurrence(data, '"', 7);
    //     // size_t epos = findNthOccurrence(data, '"', 8);
    //     // data = data.substr(spos + 1, (epos - spos) - 1);
    //     data += '\n';
    //     if (send(client.c_fd, data.c_str(), data.length(), 0) == -1)
    //     {
    //         perror("send");
    //     }
    // }
    
   
    // if (args == "help" || args == "help -a")
    // {
    //     std::cout << "Available commands:\n";

    //     // Create a map to store command information
    //     std::map<std::string, botCommand> commandMap;

    //     // Register commands and their details
    //     commandMap["USER"] = (botCommand){"USER", "The USER command is used at the beginning of a connection to specify the username and realname of a new user.\nIt must be noted that <realname> must be the last parameter because it may contain SPACE (' ', 0x20) characters, and should be prefixed with a colon (:) if required.\nServers MAY use the Ident Protocol to look up the ‘real username’ of clients. If username lookups are enabled and a client does not have an Identity Server enabled, the username provided by the client SHOULD be prefixed by a tilde ('~', 0x7E) to show that this value is user-set.", "Command: USER\nParameters: <username> 0 * <realname>"};
    //     commandMap["NICK"] = (botCommand){"NICK", "The NICK command is used to give the client a nickname or change the previous one.\nIf the server receives a NICK command from a client where the desired nickname is already in use on the network, it should issue an ERR_NICKNAMEINUSE numeric and ignore the NICK command.\nIf the server does not accept the new nickname supplied by the client as valid (for instance, due to containing invalid characters), it should issue an ERR_ERRONEUSNICKNAME numeric and ignore the NICK command.\nIf the server does not receive the <nickname> parameter with the NICK command, it should issue an ERR_NONICKNAMEGIVEN numeric and ignore the NICK command.", "Command: NICK\nParameters: <nickname>"};
    //     commandMap["PASS"] = (botCommand){"PASS", "The PASS command is used to set a ‘connection password’. If set, the password must be set before any attempt to register the connection is made. This requires that clients send a PASS command before sending the NICK / USER combination.\nThe password supplied must match the one defined in the server configuration. It is possible to send multiple PASS commands before registering but only the last one sent is used for verification and it may not be changed once the client has been registered.", "Command: PASS\nParameters: <password>"};
    //     commandMap["JOIN"] = (botCommand){"JOIN", "The JOIN command indicates that the client wants to join the given channel(s), each channel using the given key for it. The server receiving the command checks whether or not the client can join the given channel, and processes the request. Servers MUST process the parameters of this command as lists on incoming commands from clients, with the first <key> being used for the first <channel>, the second <key> being used for the second <channel>, etc.", " Command: JOIN \nParameters: <channel>{,<channel>} [<key>{,<key>}]"};
    //     commandMap["PART"] = (botCommand){"PART", "The PART command removes the client from the given channel(s). On sending a successful PART command, the user will receive a PART message from the server for each channel they have been removed from. <reason> is the reason that the client has left the channel(s).\nFor each channel in the parameter of this command, if the channel exists and the client is not joined to it, they will receive an ERR_NOTONCHANNEL (442) reply and that channel will be ignored. If the channel does not exist, the client will receive an ERR_NOSUCHCHANNEL (403) reply and that channel will be ignored.\nThis message may be sent from a server to a client to notify the client that someone has been removed from a channel. In this case, the message <source> will be the client who is being removed, and <channel> will be the channel which that client has been removed from. Servers SHOULD NOT send multiple channels in this message to clients, and SHOULD distribute these multiple-channel PART messages as a series of messages with a single channel name on each. If a PART message is distributed in this way, <reason> (if it exists) should be on each of these messages.", "Command: PART\nParameters: <channel>{,<channel>} [<reason>]"};
    //     commandMap["PRIVMSG"] = (botCommand){"PRIVMSG", "The PRIVMSG command is used to send private messages between users, as well as to send messages to channels. <target> is the nickname of a client or the name of a channel.\nIf <target> is a channel name and the client is banned and not covered by a ban exception, the message will not be delivered and the command will silently fail. Channels with the moderated mode active may block messages from certain users. Other channel modes may affect the delivery of the message or cause the message to be modified before delivery, and these modes are defined by the server software and configuration being used.\nIf a message cannot be delivered to a channel, the server SHOULD respond with an ERR_CANNOTSENDTOCHAN (404) numeric to let the user know that this message could not be delivered.\nIf <target> is a channel name, it may be prefixed with one or more channel membership prefix character (@, +, etc) and the message will be delivered only to the members of that channel with the given or higher status in the channel. Servers that support this feature will list the prefixes which this is supported for in the STATUSMSG RPL_ISUPPORT parameter, and this SHOULD NOT be attempted by clients unless the prefix has been advertised in this token.\nIf <target> is a user and that user has been set as away, the server may reply with an RPL_AWAY (301) numeric and the command will continue.\nThe PRIVMSG message is sent from the server to client to deliver a message to that client. The <source> of the message represents the user or server that sent the message, and the <target> represents the target of that PRIVMSG (which may be the client, a channel, etc).\nWhen the PRIVMSG message is sent from a server to a client and <target> starts with a dollar character ('$', 0x24), the message is a broadcast sent to all clients on one or multiple servers.", "Command: PRIVMSG\nParameters: <target>{,<target>} <text to be sent>"};
    //     commandMap["NOTICE"] = (botCommand){"NOTICE", "The NOTICE message is used similarly to PRIVMSG.", "Command: NOTICE\nParameters: <target>{,<target>} <text to be sent>"};
    //     commandMap["QUIT"] = (botCommand){"QUIT", "The QUIT command is used to terminate a client’s connection to the server. The server acknowledges this by replying with an ERROR message and closing the connection to the client.\nThis message may also be sent from the server to a client to show that a client has exited from the network. This is typically only dispatched to clients that share a channel with the exiting user. When the QUIT message is sent to clients, <source> represents the client that has exited the network.\nWhen connections are terminated by a client-sent QUIT command, servers SHOULD prepend <reason> with the ASCII string \"Quit: \" when sending QUIT messages to other clients, to represent that this user terminated the connection themselves. This applies even if <reason> is empty, in which case the reason sent to other clients SHOULD be just this \"Quit: \" string. However, clients SHOULD NOT change behaviour based on the prefix of QUIT message reasons, as this is not required behaviour from servers.\nWhen a netsplit (the disconnecting of two servers) occurs, a QUIT message is generated for each client that has exited the network, distributed in the same way as ordinary QUIT messages. The <reason> on these QUIT messages SHOULD be composed of the names of the two servers involved, separated by a SPACE (' ', 0x20). The first name is that of the server which is still connected and the second name is that of the server which has become disconnected. If servers wish to hide or obscure the names of the servers involved, the <reason> on these messages MAY also be the literal ASCII string \"*.net *.split\" (i.e. the two server names are replaced with \"*.net\" and \"*.split\"). Software that implements the IRCv3 batch Extension should also look at the netsplit and netjoin batch types.\nIf a client connection is closed without the client issuing a QUIT command to the server, the server MUST distribute a QUIT message to other clients informing them of this, distributed in the same was an ordinary QUIT message. Servers MUST fill <reason> with a message reflecting the nature of the event which caused it to happen. For instance, \"Ping timeout: 120 seconds\", \"Excess Flood\", and \"Too many connections from this IP\" are examples of relevant reasons for closing or for a connection with a client to have been closed.", "Command: QUIT\nParameters: [<reason>]"};
    //     commandMap["INVITE"] = (botCommand){"INVITE", "The INVITE command is used to invite a user to a channel. The parameter <nickname> is the nickname of the person to be invited to the target channel <channel>.\nThe target channel SHOULD exist (at least one user is on it). Otherwise, the server SHOULD reject the command with the ERR_NOSUCHCHANNEL numeric.\nOnly members of the channel are allowed to invite other users. Otherwise, the server MUST reject the command with the ERR_NOTONCHANNEL numeric.\nServers MAY reject the command with the ERR_CHANOPRIVSNEEDED numeric. In particular, they SHOULD reject it when the channel has invite-only mode set, and the user is not a channel operator.\nIf the user is already on the target channel, the server MUST reject the command with the ERR_USERONCHANNEL numeric.\nWhen the invite is successful, the server MUST send a RPL_INVITING numeric to the command issuer, and an INVITE message, with the issuer as <source>, to the target user. Other channel members SHOULD NOT be notifie", "Command: INVITE\nParameters: <nickname> <channel>"};
    //     commandMap["TOPIC"] = (botCommand){"TOPIC", "The TOPIC command is used to change or view the topic of the given channel. If <topic> is not given, either RPL_TOPIC or RPL_NOTOPIC is returned specifying the current channel topic or lack of one. If <topic> is an empty string, the topic for the channel will be cleared.\nIf the client sending this command is not joined to the given channel, and tries to view its’ topic, the server MAY return the ERR_NOTONCHANNEL (442) numeric and have the command fail.\nIf RPL_TOPIC is returned to the client sending this command, RPL_TOPICWHOTIME SHOULD also be sent to that client.\nIf the protected topic mode is set on a channel, then clients MUST have appropriate channel permissions to modify the topic of that channel. If a client does not have appropriate channel permissions and tries to change the topic, the ERR_CHANOPRIVSNEEDED (482) numeric is returned and the command will fail.\nIf the topic of a channel is changed or cleared, every client in that channel (including the author of the topic change) will receive a TOPIC command with the new topic as argument (or an empty argument if the topic was cleared) alerting them to how the topic has changed.\nClients joining the channel in the future will receive a RPL_TOPIC numeric (or lack thereof) accordingly.", "Command: TOPIC\nParameters: <channel> [<topic>]"};
    //     commandMap["KICK"] = (botCommand){"KICK", "The KICK command can be used to request the forced removal of a user from a channel. It causes the <user> to be removed from the <channel> by force. If no comment is given, the server SHOULD use a default message instead.\nThe server MUST NOT send KICK messages with multiple users to clients. This is necessary to maintain backward compatibility with existing client software.\nServers MAY limit the number of target users per KICK command via the TARGMAX parameter of RPL_ISUPPORT, and silently drop targets if the number of targets exceeds the limit.", "Command: KICK\nParameters: <channel> <user> *( \",\" <user> ) [<comment>]"};

    //     // Add more commands and their details here...

    //     // Iterate over the command map and print the details
    //     std::map<std::string, botCommand>::const_iterator iter = commandMap.begin();
    //     for (; iter != commandMap.end(); ++iter)
    //     {
    //         // std::cout << iter->first << " - " << iter->first << "\n";
    //         // std::cout << "Usage: " << iter->second.usage << "\n\n";
    //         std::ostringstream oss;
    //         oss
    //             << iter->first << " - " << iter->first << "\n";
    //         if (args == "help -a")
    //             oss << "description: " << iter->second.description << "\n";

    //         oss << "Usage: " << iter->second.usage << "\n\n";
    //         user.send(oss.str());
    //     }
    // }
}
