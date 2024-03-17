#include"Commands.hpp"
#include"Channel.hpp"
#include"server.hpp"

std::string trim(const std::string &str)
{
    size_t leftPos = str.find_first_not_of(" \t\n\r\v\f");
    size_t rightPos = str.find_last_not_of(" \t\n\r\v\f");

    if (leftPos == std::string::npos || rightPos == std::string::npos)
        return std::string("");  // Empty string if all characters are whitespace

    return str.substr(leftPos, rightPos - leftPos + 1);
}

// "  " "aa aa" "" "    fer dfewr    "
std::string check_pass(std::string pass){
    std::string str = trim(pass);
    return str;
}


int main(int ac, char **av)
{
    (void)ac;
    try
    {
        if (ac != 3)
        {
            std::cerr<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
            return 1;
        }
            // limits ports
        // if (av[1] == NULL || !av[2][0])
        // {
        //     std::cerr<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
        //     return 2;
        // }
        // ports 6660–6669, 7000
        if (std::atoi(av[1]) <= 6000 || std::atoi(av[1]) >= 7000)
        {
            std::cerr<<"Usage : ./ircserver PORT[6000 - 7000] password"<<std::endl;
            return 2;
        }
        // signal(SIGINT, &handler);
        //signal(SIGTERM, SIG_IGN);
        //signal(SIGKILL, SIG_IGN);
        std::string password = check_pass(av[2]);
        std::string port = trim(av[1]);
        std::cout << "pass = " << password << std::endl;
        if (password.empty() == 0){
            Server server(port, password);
            server.runServer();
        }
        else
            std::cerr << "enter a valid password !" << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}
