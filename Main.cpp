#include "Server.hpp"

bool isPortValid(std::string port){
	return (port.find_first_not_of("0123456789") == std::string::npos && \
	std::atoi(port.c_str()) >= 1024 && std::atoi(port.c_str()) <= 65535);
}

int main(int ac, char **av)
{
	Server ser;
	if (ac != 3)
		{std::cout << "Usage: " << av[0] << " <port number> <password>" << std::endl; return 1;}
	std::cout << "---- SERVER ----" << std::endl;
	try
	{
		signal(SIGINT, Server::signalHandler);
		signal(SIGQUIT, Server::signalHandler);
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    		perror("signal");
    		return -1;
		}
		if(!isPortValid(av[1]) || !*av[2] || std::strlen(av[2]) > 20)
			{std::cout << "invalid Port number / Password!" << std::endl; return 1;}
		ser.init(std::atoi(av[1]), av[2]);
	}
	catch(const std::exception& e)
	{
		ser.CloseFds();
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}