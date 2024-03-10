#include"Commands.hpp"
#include"Channel.hpp"
#include"server.hpp"


int main(int ac, char **av)
{
    if (ac != 3)
        return 1;
        // limits ports
    if (std::atoi(av[1]) <= 6000 || std::atoi(av[1]) >= 7000)
    {
        std::cout<<"Usage : ./irc PORT[6000 - 7000] password"<<std::endl;
        return 2;
    }
    Server server(av[1], av[2]);
    server.runServer();
    return 0;
}
