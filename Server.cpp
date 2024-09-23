#include "Server.hpp"
bool Server::signal = false; //-> initialize the static boolean

Server::Server(){this->serSocketFd = -1;} //-> default constructor
Server::~Server(){}
Server::Server(Server const &src){*this = src;}
Server &Server::operator=(Server const &src){
	if (this != &src){
	
		this->port = src.port;
		this->serSocketFd = src.serSocketFd;
		this->password = src.password;
		this->clients = src.clients;
		this->channels = src.channels;
		this->fds = src.fds;
	}
	return *this;
}
int Server::GetPort(){return this->port;}

int Server::GetFd(){return this->serSocketFd;}

Client *Server::GetClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].Getfd() == fd)
			return &this->clients[i];
	}
	return NULL;
}
Client *Server::GetClientNick(std::string nickname){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetNickName() == nickname)
			return &this->clients[i];
	}
	return NULL;
}

Channel *Server::GetChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			return &channels[i];
	}
	return NULL;
}
void Server::SetFd(int fd){this->serSocketFd = fd;}
void Server::SetPort(int port){this->port = port;}
void Server::SetPassword(std::string password){this->password = password;}
std::string Server::GetPassword(){return this->password;}
void Server::AddClient(Client newClient){this->clients.push_back(newClient);}
void Server::AddChannel(Channel newChannel){this->channels.push_back(newChannel);}
void Server::AddFds(pollfd newFd){this->fds.push_back(newFd);}

void Server::signalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "signal Received!" << std::endl;
	Server::signal = true; //-> set the static boolean to true to stop the server
}
void Server::RemoveClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].Getfd() == fd)
			{this->clients.erase(this->clients.begin() + i); return;}
	}
}
void Server::RemoveChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			{this->channels.erase(this->channels.begin() + i); return;}
	}
}
void Server::RemoveFds(int fd){
	for (size_t i = 0; i < this->fds.size(); i++){
		if (this->fds[i].fd == fd)
			{this->fds.erase(this->fds.begin() + i); return;}
	}
}
void	Server::RmChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;
		if (channels[i].get_client(fd))
			{channels[i].remove_client(fd); flag = 1;}
		else if (channels[i].get_admin(fd))
			{channels[i].remove_admin(fd); flag = 1;}
		if (channels[i].GetClientsNumber() == 0)
			{channels.erase(channels.begin() + i); i--; continue;}
		if (flag){
			std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendTo_all(rpl);
		}
	}
}
void Server::senderror(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() faild" << std::endl;
}

void Server::sendResponse(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() faild" << std::endl;
}
void Server::CloseFds(){
	for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << clients[i].Getfd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].Getfd());
	}
	if (serSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << serSocketFd << "> Disconnected" << WHI << std::endl;
		close(serSocketFd);
	}
}

void Server::ClearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < fds.size(); i++){ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
			{fds.erase(fds.begin() + i); break;}
	}
	for(size_t i = 0; i < clients.size(); i++){ //-> remove the client from the vector of clients
		if (clients[i].Getfd() == fd)
			{clients.erase(clients.begin() + i); break;}
	}

}
void Server::init(int port, std::string pass)
{
	this->password = pass;
	this->port = port;
	this->set_sever_socket();

	std::cout << GRE << "Server <" << serSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (Server::signal == false)
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::signal == false)
			throw(std::runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == serSocketFd)
					this->accept_new_client();
				else
					this->ReceiveNewData(fds[i].fd);
			}
		}
	}
	CloseFds();
}

void Server::set_sever_socket() //initializing the server socket and setting up the server to handle incoming connections and data
{
	int en = 1;
	add.sin_family = AF_INET; //This sets the address family to IPv4
	add.sin_addr.s_addr = INADDR_ANY; // This specifies that the socket will bind to all available network interfaces on the machine
	add.sin_port = htons(port); //The port number is converted from host byte order to network byte order using htons() (which stands for "host to network short"). This ensures the port number is stored in big-endian format, which is required by the network protocol.
	serSocketFd = socket(AF_INET, SOCK_STREAM, 0); // Specifies the use of a TCP socket, which is connection-oriented and reliable. The protocol is set to 0, allowing the system to select the appropriate protocol for the given socket type (in this case, TCP).
	if(serSocketFd == -1)
		throw(std::runtime_error("faild to create socket"));
	if(setsockopt(serSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) // This option allows the socket to bind to an address that is already in use. This is useful during development or when restarting the server frequently, as it avoids the "address already in use" error.
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(serSocketFd, F_SETFL, O_NONBLOCK) == -1) //This option makes the socket non-blocking, meaning that operations on the socket will return immediately rather than waiting for an event (e.g., data being available to read).
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(serSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //This function binds the socket to the specified address and port (add structure)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(serSocketFd, SOMAXCONN) == -1) // making it a passive socket that waits for incoming connection requests.
		throw(std::runtime_error("listen() faild"));
	new_cli.fd = serSocketFd;
	new_cli.events = POLLIN; //the socket is ready for reading (i.e., it can receive incoming connections or data).
	new_cli.revents = 0;
	fds.push_back(new_cli);
}

void Server::accept_new_client()
{
	Client cli;
	memset(&cliadd, 0, sizeof(cliadd)); //structure, which will store the address information of the incoming client. 
	socklen_t len = sizeof(cliadd);
	int incofd = accept(serSocketFd, (sockaddr *)&(cliadd), &len); //The file descriptor for the new client socket is returned by accept(). This is used to communicate with the newly connected client.
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) // sets the new client socket to non-blocking mode.
		{std::cout << "fcntl() failed" << std::endl; return;}
	new_cli.fd = incofd;
	new_cli.events = POLLIN;
	new_cli.revents = 0;
	cli.Setfd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients.push_back(cli);
	fds.push_back(new_cli);
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
	sendResponse("type PLEASE command pass and password\n", incofd);
}

void Server::ReceiveNewData(int fd)
{
	std::vector<std::string> cmd;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *cli = GetClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		RmChannels(fd);
		RemoveClient(fd);
		RemoveFds(fd);
		close(fd);
	}
	else
	{ 
		cli->setBuff(buff);
		if(cli->getBuff().find_first_of("\r\n") == std::string::npos)
			return;
		cmd = split_recivedBuffer(cli->getBuff());
		for(size_t i = 0; i < cmd.size(); i++)
			this->parse_exec_cmd(cmd[i], fd);
		if(GetClient(fd))
			GetClient(fd)->clearBuff();
	}
}

std::vector<std::string> Server::split_recivedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}

std::vector<std::string> Server::split_cmd(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

bool Server::notregistered(int fd)
{
	if (!GetClient(fd) || GetClient(fd)->GetNickName().empty() || GetClient(fd)->GetUserName().empty() || GetClient(fd)->GetNickName() == "*"  || !GetClient(fd)->GetLog())
		return false;
	return true;
}

void Server::parse_exec_cmd(std::string &cmd, int fd)
{
	if(cmd.empty() || cmd == "CAP LS 302")
		return ;
	std::vector<std::string> splited_cmd = split_cmd(cmd);
	size_t found = cmd.find_first_not_of(" \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	if(splited_cmd.size() && (splited_cmd[0] == "PASS" || splited_cmd[0] == "pass"))
		client_authen(fd, cmd);
	else if (splited_cmd.size() && (splited_cmd[0] == "NICK" || splited_cmd[0] == "nick"))
		set_nickname(cmd,fd);
	else if(splited_cmd.size() && (splited_cmd[0] == "USER" || splited_cmd[0] == "user"))
		set_username(cmd, fd);
	else if (splited_cmd.size() && (splited_cmd[0] == "QUIT" || splited_cmd[0] == "quit"))
		QUIT(cmd,fd);
	else if(notregistered(fd))
	{
		if (splited_cmd.size() && (splited_cmd[0] == "KICK" || splited_cmd[0] == "kick"))
			KICK(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "WHO" || splited_cmd[0] == "who"))
			handleWHO(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "JOIN" || splited_cmd[0] == "join"))
			JOIN(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "TOPIC" || splited_cmd[0] == "topic"))
			Topic(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "MODE" || splited_cmd[0] == "mode"))
			mode_command(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PART" || splited_cmd[0] == "part"))
			PART(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PRIVMSG" || splited_cmd[0] == "privmsg"))
			PRIVMSG(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "INVITE" || splited_cmd[0] == "invite"))
			Invite(cmd,fd);
		else if (splited_cmd.size())
			sendResponse(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
			
	}
	else if (!notregistered(fd))
		sendResponse(ERR_NOTREGISTERED(std::string("*")),fd);
}

void Server::handleWHO(std::string &cmd, int fd)
{
	std::vector<std::string> scmd = split_cmd(cmd);
		
	// Ensure correct number of parameters
	if (scmd.size() < 2) {
		sendResponse(ERR_NEEDMODEPARM(GetClient(fd)->GetNickName(), "WHO"), fd);
		return;
	}
		
	std::string target = scmd[1]; // Target channel or user

	// Check if target is a channel (starts with '#')
	if (target[0] == '#') {
		Channel *channel = GetChannel(target.substr(1)); // Extract channel name without the '#'
		
		// Check if channel exists
		if (!channel) {
			sendResponse(ERR_CHANNELNOTFOUND(GetClient(fd)->GetNickName(), target), fd);
			return;
		}
		
		// Iterate through the clients in the channel
		std::vector<Client> clients = channel->getAllClients();
		for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			Client client = *it;
			std::string response = ":server 352 " + GetClient(fd)->GetNickName() + " " + 
								   target + " " + client.GetUserName() + " " + client.GetHostName() + 
								   " " + client.GetServerName() + " " + client.GetNickName() + 
								   (client.IsOper() ? " @" : " +") + " :0 " + client.GetRealName() + "\r\n";
			sendResponse(response, fd);
		}

		// End of WHO list
		sendResponse(":server 315 " + GetClient(fd)->GetNickName() + " " + target + " :End of /WHO list.\r\n", fd);
	}
	else {
		// WHO for a specific user can be handled here
		Client *client = GetClientNick(target);

		if (!client) {
			sendResponse(ERR_NOSUCHNICK(GetClient(fd)->GetNickName(), target), fd);
			return;
		}

		std::string response = ":server 352 " + GetClient(fd)->GetNickName() + " * " + 
							   client->GetUserName() + " " + client->GetHostName() + 
							   " " + client->GetServerName() + " " + client->GetNickName() + 
							   (client->IsOper() ? " @" : " +") + " :0 " + client->GetRealName() + "\r\n";
		sendResponse(response, fd);

		// End of WHO list
		sendResponse(":server 315 " + GetClient(fd)->GetNickName() + " * :End of /WHO list.\r\n", fd);
	}
}
