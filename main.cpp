#include"Commands.hpp"
#include"Channel.hpp"
#include"server.hpp"


int main(int ac, char **av)
{
    Server server(av[1], av[2]);
    server.runServer();
    return 0;
}
