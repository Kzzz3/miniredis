#include "server.h"
#include <iostream>

std::string GenerateReply(std::vector<std::string>& result)
{
    int size = result.size();
    std::string temp = std::to_string(size);

    std::string reply = "*" + temp + "\r\n";

    for (auto& sds : result)
    {
        temp = std::to_string(sds.length());
        reply = reply + "$" + temp + "\r\n";
        reply = reply + sds + "\r\n";
    }

    return reply;
}

int main()
{

    std::thread([]() { server.start(); }).detach();

    sleep(1);

    asio::io_context io_context;
    asio::ip::tcp::socket socket(io_context);
    asio::ip::tcp::resolver resolver(io_context);

    try
    {
        asio::connect(socket, resolver.resolve("127.0.0.1", "10087"));
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    std::thread thd(
        [&]()
        {
            while (true)
            {
                sleep(1);
                char buffer[1024];
                size_t n = socket.read_some(asio::buffer(buffer, 1024));
                if (n != 0)
                    std::cout << std::string_view(buffer, n) << std::endl;
                else
                {
                    std::cout << "server close" << std::endl;
                    break;
                }
            }
        });
    thd.detach();

    while (true)
    {
        std::string cmd;
        std::getline(std::cin, cmd); // 读取一行命令

        std::istringstream iss(cmd); // 将命令行输入流传递给istringstream
        std::vector<std::string> words;
        std::string word;

        // 使用空格分割输入的命令
        while (iss >> word)
        {
            words.push_back(word);
        }

        cmd = GenerateReply(words);
        socket.send(asio::const_buffer(cmd.c_str(), cmd.length()));
    }
    return 0;
}