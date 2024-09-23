#include "Server.hpp"

std::string Server::mode_toAppend(std::string chain, char opera, char mode)
{
	std::stringstream ss;

	ss.clear();
	char last = '\0';
	for(size_t i = 0; i < chain.size(); i++)
	{
		if(chain[i] == '+' || chain[i] == '-')
			last = chain[i];
	}
	if(last != opera)
		ss << opera << mode;
	else
		ss << mode;
	return ss.str();
}

void Server::getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params)
{
	std::istringstream stm(cmd);
	if (!std::getline(stm, name, ' '))
		name = "";
	if (!std::getline(stm, modeset, ' '))
		modeset = "";
	if (!std::getline(stm, params, ' '))
		params = "";
}


std::vector<std::string> Server::splitParams(std::string params)
{
	std::vector<std::string> tokens;
	std::string param;
	std::istringstream stm(params);
	while (std::getline(stm, param, ','))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}

void Server::mode_command(std::string& cmd, int fd)
{
	std::string channelName;
	std::string params;
	std::string modeset;
	std::stringstream mode_chain;
	std::string arguments = ":";
	Channel *channel;
	char opera = '\0';

	arguments.clear();
	mode_chain.clear();
	Client *cli = GetClient(fd);
	size_t found = cmd.find_first_not_of("MODEmode \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	else
	{
		sendResponse(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd); 
		return ;
	}
	getCmdArgs(cmd ,channelName, modeset ,params);
	std::vector<std::string> tokens = splitParams(params);
	std::vector<std::string> channels = splitParams(channelName);
		if(channels[0][0] != '#' || !(channel = GetChannel(channels[0].substr(1))))
		{
			sendResponse(ERR_CHANNELNOTFOUND(cli->GetUserName(),channel->GetName()), fd);
			return ;
		}
		else if (!channel->get_client(fd) && !channel->get_admin(fd))
		{
			senderror(442, GetClient(fd)->GetNickName(), channelName, GetClient(fd)->Getfd(), " :You're not on that channel\r\n"); return;
		}
		else if (modeset.empty() || modeset.size() > 2 || (modeset[0] != '+' && modeset[0] != '-')) // response with the channel modes (MODE #channel)
		{
			sendResponse(RPL_CHANNELMODES(cli->GetNickName(), channel->GetName(), channel->getModes()) + \
			RPL_CREATIONTIME(cli->GetNickName(), channel->GetName(),channel->get_creationtime()),fd);
			return ;
		}
		else if (!channel->get_admin(fd)) // client is not channel operator
		{
			sendResponse(ERR_NOTOPERATOR(channel->GetName()), fd);
			return ;
		}
		else if(channel)
		{
			opera = modeset[0];
			if(modeset[1] == 'i')//invite mode
				mode_chain << invite_only(channel , opera, mode_chain.str());
			else if (modeset[1] == 't') //topic restriction mode
				mode_chain << topic_restriction(channel, opera, mode_chain.str());
			else if (modeset[1] == 'k') //password set/remove
				mode_chain <<  password_mode(tokens, channel, opera, fd, mode_chain.str(), arguments);
			else if (modeset[1] == 'o') //set/remove user operator privilege
					mode_chain << operator_privilege(tokens, channel, fd, opera, mode_chain.str(), arguments);
			else if (modeset[1] == 'l') //set/remove channel limits
				mode_chain << channel_limit(tokens, channel, opera, fd, mode_chain.str(), arguments);
			else
				sendResponse(ERR_UNKNOWNMODE(cli->GetNickName(), channel->GetName(),modeset[1]),fd);
	std::string chain = mode_chain.str();
	if(chain.empty())
		return ;
 	channel->sendTo_all(RPL_CHANGEMODE(cli->getHostname(), channel->GetName(), mode_chain.str(), arguments));
	}
}

std::string Server::invite_only(Channel *channel, char opera, std::string chain)
{
	std::string param;
	param.clear();
	if(opera == '+' && !channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, true);
		channel->SetInvOnly(1);
		param =  mode_toAppend(chain, opera, 'i');
	}
	else if (opera == '-' && channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, false);
		channel->SetInvOnly(0);
		param =  mode_toAppend(chain, opera, 'i');
	}
	return param;
}

std::string Server::topic_restriction(Channel *channel ,char opera, std::string chain)
{
	std::string param;
	param.clear();
	if(opera == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->set_topicRestriction(true);
		param =  mode_toAppend(chain, opera, 't');
	}
	else if (opera == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->set_topicRestriction(false);
		param =  mode_toAppend(chain, opera, 't');
	}	
	return param;
}

bool validPassword(std::string password)
{
	if(password.empty())
		return false;
	for(size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}
std::string Server::password_mode(std::vector<std::string> tokens, Channel *channel, char opera, int fd, std::string chain, std::string &arguments)
{
	std::string param;
	std::string pass;

	param.clear();
	pass.clear();
	if(tokens.size() != 0)
	{
		pass = tokens[0];
	if(!validPassword(pass))
	{
		sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	if(opera == '+')
	{
		channel->setModeAtindex(2, true);
		channel->SetPassword(pass);
		if(!arguments.empty())
			arguments += " ";
		arguments += pass;
		param = mode_toAppend(chain,opera, 'k');
	}
	else if (opera == '-' && channel->getModeAtindex(2))
	{
		if(pass == channel->GetPassword())
		{		
			channel->setModeAtindex(2, false);
			channel->SetPassword("");
			param = mode_toAppend(chain,opera, 'k');
		}
		else
			sendResponse(ERR_KEYSET(channel->GetName()),fd);
	}
	}
	else
	{
		sendResponse(ERR_NEEDMODEPARM(channel->GetName(),std::string("(k)")),fd);
		return param;
	}
	return param;
}

std::string Server::operator_privilege(std::vector<std::string> tokens, Channel *channel, int fd, char opera, std::string chain, std::string& arguments)
{
	std::string user;
	std::string param;

	param.clear();
	user.clear();
	if(tokens.size() != 0)
		user = tokens[0];
	else
	{
		sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(o)"),fd);
		return param;
	}
	if(!channel->clientInChannel(user))
	{
		sendResponse(ERR_NOSUCHNICK(channel->GetName(), user),fd);
		return param;
	}
	if(opera == '+')
	{
		channel->setModeAtindex(3,true);
		if(channel->change_clientToAdmin(user))
		{
			param = mode_toAppend(chain, opera, 'o');
			if(!arguments.empty())
				arguments += " ";
			arguments += user;
		}
	}
	else if (opera == '-')
	{
		channel->setModeAtindex(3,false);
		if(channel->change_adminToClient(user))
		{
			param = mode_toAppend(chain, opera, 'o');
				if(!arguments.empty())
					arguments += " ";
			arguments += user;

		}
	}
	return param;
}

bool Server::isvalid_limit(std::string& limit)
{
	return (!(limit.find_first_not_of("0123456789")!= std::string::npos) && std::atoi(limit.c_str()) > 0);
}

std::string Server::channel_limit(std::vector<std::string> tokens,  Channel *channel, char opera, int fd, std::string chain, std::string& arguments)
{
	std::string limit;
	std::string param;

	param.clear();
	limit.clear();
	if(opera == '+')
	{
		if(tokens.size() != 0 )
		{
			limit = tokens[0];
			if(!isvalid_limit(limit))
			{
				sendResponse(ERR_INVALIDMODEPARM(channel->GetName(),"(l)"), fd);
			}
			else
			{
				channel->setModeAtindex(4, true);
				channel->SetLimit(std::atoi(limit.c_str()));
				if(!arguments.empty())
					arguments += " ";
				arguments += limit;
				param =  mode_toAppend(chain, opera, 'l');
			}
		}
		else
			sendResponse(ERR_NEEDMODEPARM(channel->GetName(),"(l)"),fd);
	}
	else if (opera == '-' && channel->getModeAtindex(4))
	{
		channel->setModeAtindex(4, false);
		channel->SetLimit(0);
		param  = mode_toAppend(chain, opera, 'l');
	}
	return param;
}