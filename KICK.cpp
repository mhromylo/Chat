#include "Server.hpp"
#include <sstream>
#include <algorithm> // For std::remove

void FindK(std::string cmd, std::string tofind, std::string &str) {
	size_t i = 0;
	for (; i < cmd.size(); i++) {
		if (cmd[i] != ' ') {
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind) break;
			else tmp.clear();
		}
	}
	if (i < cmd.size()) str = cmd.substr(i);
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

std::string SplitCmdK(std::string &cmd, std::string &channel, std::vector<std::string> &tmp) {
	std::stringstream ss(cmd);
	std::string str, users, reason;
	std::cout << cmd << std::endl; 
	if (!(ss >> str)) return std::string("");
	std::cout << str << std::endl; 
	// Read the first two parts: users and channel
	if (!(ss >> channel)) return std::string("");
	std::cout << channel << std::endl; 
	if (!(ss >> users)) return std::string("");
	tmp.push_back(users); // This is the channel
	std::cout << users << std::endl; 
	// The remaining part is the reason
	if (std::getline(ss, reason)) {
		size_t pos = reason.find(':');
		if (pos != std::string::npos) {
			reason = reason.substr(pos + 1); // Extract everything after ":"
		}
	}

	// Trim leading and trailing spaces
	reason.erase(0, reason.find_first_not_of(' '));
	reason.erase(reason.find_last_not_of(' ') + 1);

	return reason;
}

std::string Server::SplitCmdKick(std::string cmd, std::string &channel, std::vector<std::string> &users, int fd) {
	
	
	std::string reason = SplitCmdK(cmd, channel, users);
	if (users.size() < 1) // Check if at least one user and one channel are provided
		return std::string("");
	
	std::cout << channel << std::endl; 
	std::string userStr = users[1]; 
	// Split the user string by commas to get individual users
	std::string tempUser;
	users.clear();
	std::stringstream userStream(userStr);
	while (std::getline(userStream, tempUser, ',')) {
		if (!tempUser.empty()) {
			users.push_back(tempUser);
		}
	}
	for (size_t i = 0; i < users.size(); i++)
	{
		std::cout << users[i] << std::endl;
	}
	
	// Remove leading ":" in the reason
	if (!reason.empty() && reason[0] == ':') 
		reason.erase(reason.begin());

	// Validate channel
	if (channel.empty() || channel[0] != '#') {
		senderror(403, GetClient(fd)->GetNickName(), channel, GetClient(fd)->Getfd(), " :No such channel\r\n");
		return std::string("");
	}
	else
		channel = channel.substr(1);

	return reason;
}

void Server::KICK(std::string cmd, int fd) {
	std::cout << cmd << std::endl;
	std::string channel;
	std::vector<std::string> users;
	std::string reason = SplitCmdKick(cmd, channel, users, fd);

	if (users.empty() || channel.empty()) {
		senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->Getfd(), " :Not enough parameters\r\n");
		return;
	}

	Channel *ch = GetChannel(channel);
	if (!ch) {
		senderror(403, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :No such channel\r\n");
		return;
	}

	if (!ch->get_client(fd) && !ch->get_admin(fd)) {
		senderror(442, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :You're not on that channel\r\n");
		return;
	}

	if (!ch->get_admin(fd)) {
		senderror(482, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :You're not channel operator\r\n");
		return;
	}

	// Use traditional for loop instead of range-based for loop
	for (std::vector<std::string>::iterator it = users.begin(); it != users.end(); ++it) {
		std::string &user = *it; // Dereference iterator to get user

		Client *clientToKick = ch->GetClientInChannel(user);
		if (clientToKick) {
		if (clientToKick->Getfd() == fd){
			sendResponse("for kick youself, use please command PASS", fd);
		}
			std::stringstream ss;
			ss << ":" << GetClient(fd)->GetNickName() << "!~@" << "localhost" << " KICK #" << channel << " " << user;
			if (!reason.empty())
				ss << " :" << reason << "\r\n";
			else
				ss << "\r\n";
			ch->sendTo_all(ss.str());

			if (ch->get_admin(clientToKick->Getfd())) {
				ch->remove_admin(clientToKick->Getfd());
			} else {
				ch->remove_client(clientToKick->Getfd());
			}
		} else {
			senderror(441, GetClient(fd)->GetNickName(), "#" + channel, GetClient(fd)->Getfd(), " :They aren't on that channel\r\n");
		}
	}
}
