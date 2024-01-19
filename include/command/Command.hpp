#pragma once 

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include "TFile.hpp"
#include "Errors.hpp"
#include "channel.hpp"
#include <utility>

class Server;

// handle the buffer that's comming from server and parse it into vector<string>
std :: vector<std :: string> HandleIncomingMsg(std :: vector<std :: string> & commands,std :: string msg);

// compare first string in vector too see if is a valid command
void                        execute_commmand(Server *sev,std :: vector<std :: string> & commands,int id);

void                        send_file(Server *sev,std :: vector<std :: string> & commands,Client cl);

void                        get_file(Server *srv,std :: vector<std :: string> command,Client cl);

int                         search_a_file(Client clt,std :: string sender);

void                        creat_file(Client clt,std :: string sender,std :: string filename);

void                        prv_msg(Server *srv,std::vector<std :: string>command,Client clt);

const char *                getDownMsg(void);

void                        check_targets(Server *srv,std::vector<std::string>commmand,Client clt,size_t position);

int                        search_in_channels(Server * srv,std::string name,Client clt);

int                        search_client_inChannel(Client clt,channel channel);

void                       sendPrvmsg(Client sender,std::string msg,Client recv);
#include "Server.hpp"