#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<list>
#include<map>
#include <arpa/inet.h>
#include <curl/curl.h>

#include "server.hpp"
std::string my_hostname();
#define Hostname ":" + my_hostname()
//replies
#define RPL_CREATIONTIME(client, channel, creation_time) Hostname + " 329 " + (client) + " " + (channel) + " " + (creation_time) + "\r\n"
#define RPL_NAMERPLY(client, channel, users_list) Hostname + " 353 " + (client) + " = " + (channel) + " :" + (users_list) + "\r\n"
#define RPL_ENDOFNAMES(client, channel)  Hostname + " 366 " + (client) + " " + (channel) + " :End of /NAMES list\r\n"
#define RPL_CHANNELMODEIS(client, channel, modestring) Hostname + " 324 " + (client) + " "+ (channel) + " " +(modestring) + " \r\n"
#define RPL_ENDOFINVITELIST(client) Hostname + " 337 " + " :End of /INVITE list\r\n"
#define RPL_TOPIC(client, channel, topic) Hostname + " 332 " + (client) + " " + (channel) +  " :" + (topic) + "\r\n"
#define RPL_INVITING(client, nick, channel)  Hostname + " 341 " + (client) + " " + (nick) + " " + (channel) + "\r\n"
#define RPL_TOPICWHOTIME(client, channel, nick, setat) Hostname + " 333 " + (client) + " " + (channel) + " " + (nick) + " " + (setat) + "\r\n"
#define RPL_NOTOPIC(client, channel)  Hostname + " 331 " + (client) + " " + (channel) + " :No topic is set\r\n"
#define RPL_ISUPPORT(client, values)  Hostname + " 005 " + (client) + " " + (values) + " :are supported by this server\r\n"
#define RPL_WELCOME(client) Hostname +  " 001 " + (client) + " :Welcome to the IRC Network, " + (client) + "\r\n"
#define RPL_YOURHOST(client) Hostname + " 002 " + (client) + " :Your host is " + (Hostname) + "\r\n"
#define RPL_CREATED(client) Hostname + " 003 " + (client) + " :This server was created Wed, 28 feb 2024 22:56:48 UTC" + "\r\n"
#define RPL_MYINFO(client) Hostname + " 004 " + "irc" + "Version 0.0.01" + " it" + " klo" + "\r\n"
//errors 
#define ERR_ALREADYREGISTERED(client) Hostname + " 462 " +  (client) + " " + " :You may not reregister\r\n"
#define ERR_NOTREGISTERED(client) Hostname + " 451 " +  (client) + " " + " :You have not registered\r\n"
#define ERR_ERRONEUSNICKNAME(client, command) Hostname + " 432 " +  (client) + " " + command + " ::Erroneus nickname\r\n"
#define ERR_UNKNOWNCOMMAND(client, command) Hostname + " 421 " +  (client) + " " + (command) + " :Unknown command\r\n"
#define ERR_NOSUCHNICK(client1, client2) Hostname + " 401 " + (client1) + " " + (client2) + " :No such nick\r\n"
#define ERR_NEEDMOREPARAMS(client, command) Hostname +  " 461 " + (client) + " " + (command) + " :Not enough parameters\r\n"
#define ERR_BADCHANNELKEY(client, channel)   Hostname + " 475 " + (client) + " " + (channel) + " :Cannot join channel (+k)\r\n"
#define ERR_NOSUCHCHANNEL(client, channel)  Hostname + " 403 " + (client) + " " + (channel) + " :No such channel\r\n"
#define ERR_NOTONCHANNEL(client, channel)  Hostname + " 442 " + (client) + " " + (channel) + " :You're not on that channel\r\n" 
#define ERR_PASSWDMISTACH(client) Hostname + " 464 " + (client) + " :Password incorrect\r\n"
#define ERR_NOPASSGIVEN(client) Hostname  + " * :No password  given\r\n"
#define ERR_CHANOPRIVSNEEDED(client, channel) Hostname +  " 482 " + (client) + " " + (channel) + " :You're not channel operator\r\n"
#define ERR_USERONCHANNEL(client, nick, channel) Hostname + " 443 " + (client) + " " + (nick) + " " + (channel) + " :is already on channel\r\n"
#define ERR_BADCHANMASK(channel)  Hostname + " 476 " + (channel) + " :Bad Channel Mask\r\n"
#define ERR_UNKNOWNMODE(client, modechar) Hostname + " 472 " + (client) + " " + (modechar) + " :is unknown mode char to me\r\n"
#define ERR_INVITEONLYCHAN(client, channel) Hostname + " 473 " + (client) + " " + (channel) + " :Cannot join channel (+i)\r\n"
#define ERR_TOOMANYCHANNELS(client, channel) Hostname +  " 405 " + (client) + " "  + (channel) + " :You have joined too many channels\r\n"
#define ERR_KEYALREADYSET(client, channel) Hostname + " 467 " + (client) + " " + (channel) + " :Channel key already set\r\n"
#define ERR_NORECIPENT(client, command) Hostname + " 411 " + (client) + " :No recipient given " + (command) + "\r\n"
#define ERR_CHANNELISFULL(client, channel)  Hostname + " 471 " + (client) + " " + (channel) + " :Cannot join channel (+l)\r\n"
#define ERR_NONICKNAMEGIVEN(client)  Hostname + " 431 " + (client) + " :No nickname given\r\n"
#define ERR_NICKINUSE(client, usednick) Hostname  + " 433 " + (client) +  "  :Nickname is already in use.\r\n"
#define ERR_ONEOUS(client, wrongnick) Hostname  +" 432 " + (client) + " " + (wrongnick) + " :Erroneous Nickname\r\n"
#define ERROR_MSG(reason) Hostname + " " + (reason) + "\r\n"
#define ERR_NOTEXTTOSEND(client) Hostname + " 412 " + (client) + " :No text to send\r\n"

//commands 
#define PRIVMSG(client, username, ipaddr, target, message) ":" + (client) + "!~" + (username) + "@" + (ipaddr) + " PRIVMSG " + (target) + " :" + (message) + "\r\n"
#define NICK_MSG(client, username,ipaddr, newnick)  ":" + (client) + "!~" + (username) + "@"+ (ipaddr) + " NICK :"+(newnick) + "\r\n"
#define KICK_MSG(client, ipaddr, channel, kicked, reason) ":" + (client) + "!~" + (ipaddr) + " KICK " + (channel) + " " + kicked + " " + reason + "\r\n"
#define MODE_MSG(client, username,ipaddr, channel,added_mode, target) ":" + (client) + "!~" + (username) +"@" + (ipaddr) + " MODE " + (channel) + " " +(added_mode) +  target + "\r\n"
#define JOIN_SUCC(nickname, username, client_ip, channel) ":" + (nickname) + "!~" + (username) + "@" + (client_ip) + " JOIN " + (channel) + "\r\n"
#define QUIT_MSG(nickname, username, ipaddr, reason) ":"+ (nickname) + "!~" + (username) + "@" + (ipaddr) + " QUIT :Client Quit" + (reason) + "\r\n"
#define NOTICE_MSG(client, username, ipaddr, target, message) ":" + (client) + "!~" + (username) + "@" + (ipaddr) + " NOTICE " + (target) + " :" + (message) + "\r\n"
#define PART_MSG(nickname, username, ipaddr, channel, reason) ":" + (nickname) + "!~" + (username) + "@" + ipaddr + " PART " + (channel) + " " + (reason) + "\r\n"
#define INVITE_MSG(client, username, ipaddr, target, channel) ":" + (client) + "!~" + (username) + "@" + (ipaddr) + " INVITE " + (target) + " " + (channel) + "\r\n"
#define RPL_MOTD(client, sentence) Hostname + " 372 " + (client) + " " + (sentence) + "\r\n"

class Channel;
class Clientx;

class Command
{
    public:
    std::string mainstring;
    time_t rpl_time;
    std::string command;
    std::vector<std::string> command_arg;
    std::vector<std::pair<std::string, std::string> > channel;
    std::vector<std::string> users;
    std::vector<std::string> privmsg_list;
    std::string comment;
    std::string topic;
    std::string nickname;
    std::string msgtarget;
    std::string privmessage;
    std::vector<std::pair<char , char> > mode;
    std::string mode_char;
    std::vector<std::string> mode_args;
    std::string bot_arg;

    Command();
    ~Command();
   void getcommand(std::string const &str, std::vector<Channel> &chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server);

    void joincommand();
    void kickcommand();
    void topiccommand();
    void invitecommand();
    void privmsg();
    void nickcommand();
    void quitcommand();
    void partcommand();
    void modecommand();
    void botCommand();
};

struct botcmd
{
    std::string name;
    std::string description;
    std::string usage;
    std::string parameters;
};

void join(std::vector<Channel>&chan, Command &cmd, Clientx &client);
void kick(std::vector<Channel>&chan, Command &cmd, Clientx &client);
void part(std::vector<Channel>&chan, Command &cmd, Clientx &client);
void quit(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients, Server &server);
void nick(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients);
void prv(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients);
void topicf(std::vector<Channel>&chan, Command &cmd, Clientx &client , std::list<Clientx>&clients);
void invite(std::vector<Channel>&chan, Command &cmd, Clientx &client, std::list<Clientx> &clients);
// void broadcast(std::vector<Clientx> &clients, std::string msg);
void modef(std::vector<Channel>&chan, Command &cmd, Clientx &client);
void broadcast(std::vector<Clientx *> &clients, std::string msg);
void bot(std::vector<Channel>&chan, Command &cmd, Clientx &client);
void broadcast2(std::list<Clientx> &clients, std::string msg);
