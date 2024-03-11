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

void broadcast2(std::list<Clientx> &clients, std::string msg)
{
    std::list<Clientx>::iterator i = clients.begin();
    while (i != clients.end())
    {
        write(i->c_fd, msg.c_str(), msg.size());
        ++i;
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
    // std::cout<<"channel size => "<<cmd.channel.size()<<std::endl;
    if (cmd.command_arg.size() > 0)
    {
        while(i < cmd.channel.size())
        {
            if (cmd.channel[i].first[0] != '#')
                cmd.channel[i].first = '#' + cmd.channel[i].first;
            std::vector<Channel>::iterator it = std::find(chan.begin(), chan.end(), Channel(cmd.channel[i].first));
            if (it != chan.end())
            {
                // std::cout<<"chnanel >> "<<it->name<<std::endl;
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
    // size_t j = 0;
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

void invite(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    // size_t i = 0;
    if (cmd.command_arg.size() > 1)
    {
        if (cmd.channel[0].first[0] != '#')
            cmd.channel[0].first = '#' + cmd.channel[0].first;
        std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.command_arg[0]));
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

void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server)
{

    std::vector<Channel>::iterator iter = chan.begin();
    


    while(iter != chan.end())
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
    if (itl != clients.end()) {
        clients.erase(itl);
    }
    close(client.c_fd);
    if (!clients.empty()) {
        server.del_from_pfds(client.c_fd);
    }
    std::string quitmsg = QUIT_MSG(client.nickname, client.username, client.ip, cmd.comment);
    broadcast2(clients, quitmsg);
}

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

void nick(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
{
    // size_t i = 0;
    (void)chan;
    if (cmd.command_arg.size() > 0)
    {
        std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.nickname));
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

void prv(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients)
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
                std::list<Clientx>::iterator it = std::find(clients.begin(), clients.end(), Clientx(cmd.privmsg_list[i]));
                if (it != clients.end())
                {
                    std::string privmsg = PRIVMSG(client.nickname, client.username, client.ip, it->nickname, cmd.privmessage);
                    // write(it->c_fd, privmsg.c_str(), privmsg.size());
                    send(it->c_fd, privmsg.c_str(), privmsg.size(), 0);
                }
                else
                {
                    std::string nosuchnick = ERR_NOSUCHNICK(client.nickname, cmd.privmsg_list[i]);
                    // write(client.c_fd, nosuchnick.c_str(), nosuchnick.size());
                    send(client.c_fd, nosuchnick.c_str(), nosuchnick.size(), 0);
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

void topicf(std::vector<Channel>&chan, Command &cmd, Clientx &client , std::list<Clientx>&clients)
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
                        if (cmd.mode[i].second == 'i')//first +- second char
                        {
                            if (cmd.mode[i].first == '+')
                                it->mode += 'i';
                            else if (cmd.mode[i].first == '-')
                            {
                                std::cout<<"value = "<<it->mode.find(cmd.mode[i].second)<<std::endl;
                                size_t pos = it->mode.find('i');
                                if (pos != std::string::npos)
                                    it->mode.erase(it->mode.find('i'));
                            }
                        }
                        if (cmd.mode[i].second == 't')
                        {
                            std::cout<<cmd.mode[i].first<<std::endl;
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
                                    bool b = false;
                                    if (max_users > 0 && b == false)
                                    {
                                        it->mode += 'l';
                                        it->max_users = atoi(cmd.mode_args[x].c_str());
                                        std::cout << "argument dial l = "<< cmd.mode_args[x] << std::endl;
                                        it->mode_param += cmd.mode_args[x] + ' ';
                                        b = true;
                                    }
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
                                        if (!it->is_operator((*it2)->nickname))
                                        {  
                                            it->mode += cmd.mode[i].second;
                                            it->mode_param += cmd.mode_args[x] + ' ';
                                            it->add_operator(*(*it2));
                                        }
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
                                        // it->mode += cmd.mode[i].second;
                                        //     it->add_operator(*(*it2));
                                        // size_t found = it->mode.find('o');
                                        // if ()
                                        // it->mode += cmd.mode[i].second;

                                            it->mode_param += cmd.mode_args[x] + ' ';
                                            it->remove_operator(cmd.mode_args[x]);
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
                                    size_t pos2 = it->mode_param.find(cmd.mode_args[x]);
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
                                        if (pos2 != std::string::npos)
                                        {
                                            it->mode_param.erase(it->mode_param.find(cmd.mode_args[x]), cmd.mode_args[x].size());
                                            std::cout<<"------------------->"<<it->mode_param<<std::endl;
                                            it->k_flag = false;
                                        }
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
                std::string newmode = print_update(it->old_mode, it->mode ) + ' ';
                // if (!newmode.find("oitlk"))
                // {
                //     std::string mode = RPL_CHANNELMODEIS(client.nickname, cmd.channel[0].first, newmode);
                //     broadcast(it->user_list, mode);
                // }
                if (newmode.find("o") != std::string::npos || newmode.find("i") != std::string::npos || newmode.find("t") != std::string::npos || newmode.find("l") != std::string::npos || newmode.find("k") != std::string::npos)
                {
                    std::cout<<"mode param "<<it->mode_param<<std::endl;
                    std::string modeup = MODE_MSG(client.nickname, client.username, client.ip, it->name, newmode, it->mode_param);
                    broadcast(it->user_list, modeup);
                     it->mode_param.clear();
                    return ;
                }
                else
                {
                     it->mode_param.clear();
                    return ;
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
    (void)chan;
    if (cmd.bot_arg.empty())
    {
        std::string moreparams = ERR_NEEDMOREPARAMS(client.nickname, "BOT");
        if (send(client.c_fd, moreparams.c_str(), moreparams.length(), 0) == -1)
        {
            perror("send");
        }
        return ;
    }
    std::string arg[6]= {"time","quote","fact", "dad_joke", "dirty_joke", "help"};
    int i = 0;
    while(i < 6)
    {
        if (arg[i] == cmd.bot_arg)
            break;
        i++;
    }

    switch(i)
    {
        case 0:
        {
            std::time_t currentTime = std::time(0);
            std::string timeString = std::ctime(&currentTime);
            std::string welcomeMsg = "BOT: Current time: " + timeString;
            std::string motod = RPL_MOTD(client.nickname, welcomeMsg);
            if (send(client.c_fd, motod.c_str(), motod .length(), 0) == -1)
            {
                perror("send");
            }
            break;
        }
        case 1:
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
                std::string motod = RPL_MOTD(client.nickname, quote);
                if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
                {
                    perror("send");
                }
                break;
            }
        case 3:
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
                std::string motod = RPL_MOTD(client.nickname, quote);
                if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
                {
                    perror("send");
                }
                break;
            }
        case 4:
            {
                CURL *curl;
                std::string data;
                curl = curl_easy_init();
                if (curl)
                {
                    curl_easy_setopt(curl, CURLOPT_URL, "https://v2.jokeapi.dev/joke/Any?format=txt&type=single");
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
                    CURLcode res = curl_easy_perform(curl);
                    if (res != CURLE_OK)
                    {
                        // Handle error
                    }
                    curl_easy_cleanup(curl);
                }
                // size_t spos = findNthOccurrence(data, '"', 7);
                // size_t epos = findNthOccurrence(data, '"', 8);
                // data = data.substr(spos + 1, (epos - spos) - 1);
                data += '\n';
                std::string motod = RPL_MOTD(client.nickname, data);
                if (send(client.c_fd, motod.c_str(), motod.length(), 0) == -1)
                {
                    perror("send");
                }
                break;
            }
        case 5:
            {
                // struct botcmd {
                //     public:
                //     std::string name;
                //     std::string description;
                //     std::string usage;
                // };

                std::vector<botcmd> commands;

                    // Define commands and their details here
                commands.push_back((botcmd){"USER", "This command is used to set the user's username, hostname, server name, and real name. It's typically sent by the client to the server upon connection to identify the user.", "Command: USER\nParameters: <username> 0 * <realname>"});
                commands.push_back((botcmd){"NICK", "Changes your nickname on the IRC network. It's used to set or change your nickname, which is how you are identified on the network.", "Command: NICK\nParameters: <nickname>"});
                commands.push_back((botcmd){"PASS", "Sends a password to the server. This is used for server authentication, allowing users to connect to password-protected servers.", "Command: PASS\nParameters: <password>"});
                commands.push_back((botcmd){"JOIN", "Joins a specified channel. Users can join channels to participate in discussions or meetings.", "Command: JOIN\nParameters: <channel>{,<channel>} [<key>{,<key>}]"});
                commands.push_back((botcmd){"PART", "Leaves a specified channel. This command is used to leave a channel you are currently in.", "Command: PART\nParameters: <channel>{,<channel>} [<reason>]"});
                commands.push_back((botcmd){"PRIVMSG", "Sends a private message to a user or a channel. It's used for direct communication with other users or for sending messages to a channel.", "Command: PRIVMSG\nParameters: <target>{,<target>} <text to be sent>"});
                commands.push_back((botcmd){"NOTICE", "Sends a notice to a user or a channel. Notices are similar to private messages but are typically used for system messages or notices from the server.", "Command: NOTICE\nParameters: <target>{,<target>} <text to be sent>"});
                commands.push_back((botcmd){"QUIT", "Disconnects you from the IRC network. This command is used to leave the IRC network and disconnect from the server.", "Command: QUIT\nParameters: [<reason>]"});
                commands.push_back((botcmd){"INVITE", "Invites a user to a channel. This command is used to invite another user to join a channel you are currently in.", "Command: INVITE\nParameters: <nickname> <channel>"});
                commands.push_back((botcmd){"TOPIC", "Changes the topic of a channel. The topic is a short description or subject of the channel's discussion.", "Command: TOPIC\nParameters: <channel> [<topic>]"});
                commands.push_back((botcmd){"KICK", "Forcefully remove a user from a channel. This command is used by channel operators to remove users from a channel.", "Command: KICK\nParameters: <channel> <user> *( \",\" <user> ) [<comment>]"});

                std::stringstream ss;
                ss << "List of Available IRC Commands:\n\n";
                for (std::vector<botcmd>::const_iterator it = commands.begin(); it != commands.end(); ++it) {
                    const botcmd& command = *it;
                    ss << command.name << ":\n"
                    << "Description: " << command.description << "\n"
                    << "Usage: " << command.usage << "\n\n";
                }

                std::string line = ss.str(); // Convert the stringstream to a std::string

                // Assuming client.c_fd is the socket file descriptor for the client
                if (send(client.c_fd, line.c_str(), line.length(), 0) == -1) {
                    perror("send");
                }
                break;
            }
        default :
            std::string rp = ERR_UNKNOWNCOMMAND(client.ip, cmd.bot_arg);
            if (send(client.c_fd, rp.c_str(), rp.length(), 0) == -1)
            {
                perror("send");
            }
    }
}