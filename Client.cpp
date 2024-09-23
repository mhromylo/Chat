#include "Client.hpp"

Client::Client()
{
	this->nickname = "";
	this->username = "";
	this->hostname = "";
	this->servername = "";
	this->realname = "";
	this->buff = "";
	this->fd = -1;
	this->oper= false;
	this->reg = false;
	this->ipadd = "";
	this->log = false;
}
Client::Client(std::string nickname, std::string username, std::string hostname, std::string servername, std::string realname, int fd) :fd(fd), nickname(nickname), username(username), hostname(hostname), servername(servername), realname(realname){}
Client::~Client(){}
Client::Client(Client const &src){*this = src;}
Client &Client::operator=(Client const &src){
	if (this != &src){
		this->nickname = src.nickname;
		this->username = src.username;
		this->hostname = src.hostname;
		this->servername = src.servername;
		this->realname = src.realname;
		this->fd = src.fd;
		this->ChannelsInvite = src.ChannelsInvite;
		this->buff = src.buff;
		this->reg = src.reg;
		this->ipadd = src.ipadd;
		this->log = src.log;
	}
	return *this;
}
int Client::Getfd(){return this->fd;}
bool Client::getRegistered(){return this->reg;}
bool Client::GetInviteChannel(std::string &ChName){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == ChName)
			return true;
	}
	return false;
}
std::string Client::GetNickName(){return this->nickname;}
bool Client::GetLog(){return this->log;}
std::string Client::GetUserName(){return this->username;}
std::string Client::GetHostName(){return this->hostname;}
std::string Client::GetServerName(){return this->servername;}
std::string Client::GetRealName(){return this->realname;}
std::string Client::getBuff(){return this->buff;}
std::string Client::getIpAdd(){return ipadd;}
std::string Client::getHostname(){
	std::string hostname = this->GetNickName() + "!" + this->GetUserName();
	return hostname;
}
bool Client::IsOper(){return this->oper;}
void Client::Setfd(int fd){this->fd = fd;}
void Client::SetNickname(std::string& nickName){this->nickname = nickName;}
void Client::setLog(bool value){this->log = value;}
void Client::SetUsername(std::string& username){this->username = username;}
void Client::SetHostName(std::string& hostname){this->hostname = hostname;}
void Client::SetServerName(std::string& servername){this->servername = servername;}
void Client::SetRealName(std::string& realname){this->realname = realname;}
void Client::setBuff(std::string recived){buff += recived;}
void Client::setReg(bool value){this->reg = value;}
void Client::setIpAdd(std::string ipadd){this->ipadd = ipadd;}
void Client::AddChannelInvite(std::string &chname){
	ChannelsInvite.push_back(chname);
}
void Client::RmChannelInvite(std::string &chname){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == chname)
			{this->ChannelsInvite.erase(this->ChannelsInvite.begin() + i); return;}
	}
}
void Client::clearBuff(){this->buff.clear();}