#include"Commands.hpp"
#include"Channel.hpp"
#include"server.hpp"

// void handler(int sig){
//     if (sig == SIGINT)
//         quit(this->channels,cmd, *it, this->clients_list, server);
// }

// void signalHandler(int sig){
//     if (sig == SIGINT || sig == SIGTERM)
//         std::cout << "maghayti7ch" << std::endl;
// }

int main(int ac, char **av)
{
    try
    {
        if (ac != 3)
        {
            std::cout<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
            return 1;
        }
            // limits ports
        if (av[1] == NULL || !av[2][0])
        {
            std::cout<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
            return 2;
        }
        if (std::atoi(av[1]) <= 6000 || std::atoi(av[1]) >= 7000)
        {
            std::cout<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
            return 2;
        }
        // signal(SIGINT, &handler);
        // signal(SIGTERM, SIG_IGN);
        // signal(SIGKILL, SIG_IGN);
        // signal(SIGPIPE, SIG_IGN);

        Server server(av[1], av[2]);
        server.runServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
