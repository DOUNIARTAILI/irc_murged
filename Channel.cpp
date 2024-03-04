#include"Channel.hpp"
#include"server.hpp"

class Clientx;

Channel::Channel()
{
}

Channel::~Channel()
{
}

Channel::Channel(const std::string &name)
{
    this->name = name;
    this->mode = "+t";
    this->k_flag = false;
}

Channel& Channel::operator=(const Channel& other)
{
    name = other.name;
    pwd = other.pwd;
    topic = other.topic;
    mode = other.mode;
    user_list = other.user_list;
    // operator_list = other.operator_list;
    // invite_list = other.invite_list;
    return *this;
}

void Channel::add_user(Clientx &client)
{
    std::vector<Clientx *>::iterator it = std::find(user_list.begin(), user_list.end(), nickcmp(client.nickname));
    if (it == user_list.end())
    {
        user_list.push_back(&client);
    }
}

void Channel::add_operator(Clientx &client)
{
    std::vector<Clientx *>::iterator it = std::find(op_list.begin(), op_list.end(), nickcmp(client.nickname));
    if (it == op_list.end())
    {
        op_list.push_back(&client);
    }
}

void Channel::add_invite(Clientx &client)
{
    std::vector<Clientx *>::iterator it = std::find(inv_list.begin(), inv_list.end(), nickcmp(client.nickname));
    if (it == inv_list.end())
    {
        inv_list.push_back(&client);
    }
}

void Channel::remove_user(const std::string &client)
{
    std::vector<Clientx *>::iterator it = std::find(user_list.begin(), user_list.end(), nickcmp(client));
    if (it != user_list.end())
    {
        user_list.erase(it);
    }
}

void Channel::remove_operator(const std::string &client)
{
    std::vector<Clientx *>::iterator it = std::find(op_list.begin(), op_list.end(), nickcmp(client));
    if (it != op_list.end())
    {
        std::cout << "t7ied op\n";
        op_list.erase(it);
    }
}

void Channel::remove_invite(Clientx &client)
{
    std::vector<Clientx *>::iterator it = std::find(inv_list.begin(), inv_list.end(), nickcmp(client.nickname));
    if (it != inv_list.end())
    {
        // std::cout << ""
        inv_list.erase(it);
    }
}

bool Channel::operator==(const Channel& other)
{
    if (name == other.name)
        return true;
    return false;
}

bool Channel::is_user(std::string const &user)
{
    if (nickcmp(user) != NULL)
        return true;
    return false;
}

bool Channel::is_operator(std::string const &user)
{
    for (size_t i = 0; i < op_list.size(); ++i)
    {
        if (op_list[i]->nickname == user)
        {
            std::cout << "l9a nickname!\n";
             return true;
        }
    }
    return false;
}

bool Channel::is_invite(std::string const &user)
{
    for (size_t i = 0; i < inv_list.size(); ++i)
    {
        if (inv_list[i]->nickname == user)
             return true;
    }
    return false;
}

void Channel::listalloperators()
{
    size_t i = 0;
    std::cout<<"operators list :"<<std::endl;
    while(i < op_list.size())
    {
        std::cout<<op_list[i]<<std::endl;
        i++;
    }
}

void Channel::get_users_list(std::vector<std::string> &users, std::string &users_list)
{
    size_t i = 0;
    while (i < users.size())
    {
        users_list += users[i];
        if (i < users.size() - 1)
            users_list += " ";
        i++;
    }
}

 Clientx    *Channel::nickcmp(const std::string &nick)
{
    for (size_t i = 0; i < user_list.size(); ++i)
    {
        if (user_list[i]->nickname == nick)
             return user_list[i];
    }
    return NULL;
}