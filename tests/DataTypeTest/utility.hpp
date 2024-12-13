#pragma once
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

// Helper function to generate random string
inline string GetRandomString(int length)
{
    static thread_local mt19937 rng{random_device{}()};
    string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    string newstr;
    uniform_int_distribution<> dist(0, str.size() - 2);
    while (newstr.size() != length)
    {
        int pos = dist(rng);
        newstr += str.substr(pos, 1);
    }
    return newstr;
}

// Helper function to convert commands to RESP format
inline string ConvertToResp(const vector<string>& args)
{
    stringstream ss;
    ss << "*" << args.size() << "\r\n";
    for (const auto& arg : args)
    {
        ss << "$" << arg.length() << "\r\n" << arg << "\r\n";
    }
    return ss.str();
}