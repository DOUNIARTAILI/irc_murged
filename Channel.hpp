#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"server.hpp"
#include <map>
#include <utility>
// #define timeout 77

class Clientx;

class Channel
{
    public:
    time_t topic_time;
    time_t creation_time;
    std::string name;
    std::string pwd;
    std::string topic;
    std::string mode;
    std::string old_mode;
    std::string mode_param;
    std::string topicsetter;
    bool k_flag;
    size_t  max_users;
    Channel();
    ~Channel();
    Channel(const std::string &name);
    Channel& operator=(const Channel& other);

    std::vector<Clientx *> user_list;
    std::vector<Clientx *> op_list;
    std::vector<Clientx *> inv_list;
    //addition
    void add_user(Clientx &client);
    void add_operator(Clientx &client);
    void add_invite(Clientx &client);
    //remove
    void remove_user(const std::string &client);
    void remove_operator(const std::string &client);
    void remove_invite(Clientx &client);
    //if exist
    bool is_user(std::string const &user);
    bool is_operator(std::string const &user);
    bool is_invite(std::string const &user);
    //----------
    bool operator==(const Channel& other);  //overload == operator for iterator comparison
    void listalloperators(); 
    void listallusers();
    void listallchannels();
    void get_users_list(std::vector<std::string> &users, std::string &users_list);
    Clientx    *nickcmp(const std::string &n);
};
