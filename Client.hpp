#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
//-------------------------------------------------------//
#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//-------------------------------------------------------//
class Client
{
private:
	int fd; //-> client file descriptor
	bool oper;
	bool reg;
	bool log;
	std::string nickname;
	std::string buff;
	std::string username;
	std::string hostname;
	std::string servername;
	std::string realname;
	std::string ipadd; //-> client ip address
	std::vector<std::string> ChannelsInvite;
public:
	Client();
	Client(std::string nickname, std::string username, std::string hostname, std::string servername, std::string realname, int fd);
	~Client();
	Client(Client const &src);
	Client &operator=(Client const &src);
	int Getfd();
	bool getRegistered();
	bool GetInviteChannel(std::string &ChName);
	bool GetLog();
	bool IsOper();
	std::string GetNickName();
	std::string GetUserName();
	std::string GetHostName();
	std::string GetServerName();
	std::string GetRealName();
	std::string getIpAdd();
	std::string getBuff();
	std::string getHostname();
	void setLog(bool value);
	void setBuff(std::string recived);
	void setReg(bool value);
	void Setfd(int fd);
	void setIpAdd(std::string ipadd);
	void SetNickname(std::string& nickName);
	void SetUsername(std::string& username);
	void SetHostName(std::string& hostname);
	void SetServerName(std::string& servername);
	void SetRealName(std::string& realname);
	void AddChannelInvite(std::string &chname);
	void RmChannelInvite(std::string &chname);
	void clearBuff();
};