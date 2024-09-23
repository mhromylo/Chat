#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <vector> //-> for vector
#include <sstream>
#include <ctime>
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include "Channel.hpp"
#include "errors.hpp"
//-------------------------------------------------------//
#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color
//-------------------------------------------------------//
class Channel;
class Client;

class Server
{
private:
	
	
	int port; //-> server port
	int serSocketFd; //-> server socket file descriptor
	static bool signal; //-> static boolean for signal
	std::string password;
	std::vector<Client> clients; //-> vector of clients
	std::vector<Channel> channels;
	std::vector<struct pollfd> fds; //-> vector of pollfd
	struct sockaddr_in add;
	struct sockaddr_in cliadd;
	struct pollfd new_cli;
public:
	Server(); //-> default constructor
	~Server();
	Server(Server const &src);
	Server &operator=(Server const &src);
	int GetFd();
	int GetPort();
	std::string GetPassword();
	Client *GetClient(int fd);
	Client *GetClientNick(std::string nickname);
	Channel *GetChannel(std::string name);
	void SetFd(int server_fdsocket);
	void SetPort(int port);
	void SetPassword(std::string password);
	void AddClient(Client newClient);
	void AddChannel(Channel newChannel);
	void AddFds(pollfd newFd);
	void set_username(std::string& username, int fd);
	void set_nickname(std::string cmd, int fd);
	void RemoveClient(int fd);
	void RemoveChannel(std::string name);
	void RemoveFds(int fd);
	void RmChannels(int fd);
	void senderror(int code, std::string clientname, int fd, std::string msg);
	void senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg);
	void sendResponse(std::string response, int fd);
	void ServerInit(); //-> server initialization
	void init(int port, std::string pass);
	void SerSocket(); //-> server socket creation
	void accept_new_client(); //-> accept new client
	void ReceiveNewData(int fd); //-> receive new data from a registered client

	static void signalHandler(int signum); //-> signal handler

	void set_sever_socket();
 
	void CloseFds(); //-> close file descriptors
	void ClearClients(int fd); //-> clear clients

	std::vector<std::string> split_recivedBuffer(std::string str);
	std::vector<std::string> split_cmd(std::string &str);
	void parse_exec_cmd(std::string &cmd, int fd);
	//---------------//Authentification Methods
	bool notregistered(int fd);
	bool nickNameInUse(std::string& nickname);
	bool is_validNickname(std::string& nickname);
	void client_authen(int fd, std::string pass);
	//---------------------------//JOIN CMD
	void	JOIN(std::string cmd, int fd);
	int		SplitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd);
	void	ExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
	void	NotExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int fd);
	int		SearchForClients(std::string nickname);
	//---------------------------//PART CMD
	void	PART(std::string cmd, int fd);
	int		SplitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd);
	//---------------------------//CKIK CMD
	void	KICK(std::string cmd, int fd);
	std::string SplitCmdKick(std::string cmd, std::string &args, std::vector<std::string> &users, int fd);
	//---------------------------//PRIVMSG CMD
	void	PRIVMSG(std::string cmd, int fd);
	void	CheckForChannels_Clients(std::vector<std::string> &tmp, int fd);
	//---------------------------//QUITE CMD
	void	QUIT(std::string cmd, int fd);
	//---------------------------//MODE CMD
	void 		mode_command(std::string& cmd, int fd);
	std::string invite_only(Channel *channel, char opera, std::string chain);
	std::string topic_restriction(Channel *channel ,char opera, std::string chain);
	std::string password_mode(std::vector<std::string> splited, Channel *channel, char opera, int fd, std::string chain, std::string& arguments);
	std::string operator_privilege(std::vector<std::string> splited, Channel *channel, int fd, char opera, std::string chain, std::string& arguments);
	std::string channel_limit(std::vector<std::string> splited, Channel *channel, char opera, int fd, std::string chain, std::string& arguments);
	bool		isvalid_limit(std::string& limit);
	std::string mode_toAppend(std::string chain, char opera, char mode);
	std::vector<std::string> splitParams(std::string params);
	void getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params);
	//---------------------------//TOPIC CMD
	std::string tTopic();
	void Topic(std::string &cmd, int &fd);
	void Invite(std::string &cmd, int &fd);
	std::string gettopic(std::string& input);
	int getpos(std::string &cmd);
	void handleWHO(std::string &cmd, int fd);
};

//-------------------------------------------------------//
#endif