#include"Commands.hpp"
#include"Channel.hpp"
#include"server.hpp"


int main(int ac, char **av)
{
    if (ac != 3)
        return 1;
    // if (std::atoi(av[2]) )
    Server server(av[1], av[2]);
    server.runServer();
    return 0;
}
