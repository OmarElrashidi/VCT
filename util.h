#pragma once
#include "framework.h"

inline std::vector<std::string> serversList;

namespace util
{
    static void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr)
    {
        size_t pos = data.find(toSearch);
        while (pos != std::string::npos)
        {
            data.replace(pos, toSearch.size(), replaceStr);
            pos = data.find(toSearch, pos + replaceStr.size());
        }
    }

    static auto ping(const char *host, int timeout = 1000)
    {
        long p = -1;

        wchar_t buf[MAX_PATH + 1] = {};
        HANDLE hIcmpFile = IcmpCreateFile();
        DWORD ip = inet_addr(host);
        DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(buf);

        void *replayBuffer = static_cast<void *>(malloc(replySize));

        if (hIcmpFile != INVALID_HANDLE_VALUE || replayBuffer)
        {
            IcmpSendEcho(hIcmpFile, ip, buf, sizeof(buf), nullptr, replayBuffer, replySize, timeout);

            auto echoReply = static_cast<PICMP_ECHO_REPLY>(replayBuffer);
            if (echoReply->Status == IP_SUCCESS)
            {
                p = echoReply->RoundTripTime;
            }

            free(replayBuffer);
        }

        return p;
    };

    static void parseServers(const std::string &data)
    {
        std::stringstream ss(data);
        std::istream is(ss.flush().rdbuf());

        io::CSVReader<7> in("servers", is);
        in.read_header(io::ignore_extra_column, "HostName", "IP", "Score", "Ping", "Speed", "CountryLong", "CountryShort");

        std::string HostName;
        std::string IP;
        long long int Score;
        int Ping;
        long long int Speed;
        std::string CountryLong;
        std::string CountryShort;

        while (in.read_row(HostName, IP, Score, Ping, Speed, CountryLong, CountryShort))
        {
            printf("IP: %s - Round network trip: %i\n", IP.c_str(), Ping);
            serversList.push_back(IP);
        }
    }

    static void getServersList()
    {
        httplib::Client cli("http://www.vpngate.net");

        if (auto res = cli.Get("/api/iphone/"))
        {
            if (res->status == 200)
            {
                //the csv here is not valid, but it's the only way to get the data so we fix it 
                res->body.erase(0, 15);                 // removes (*vpn_servers\n#)
                res->body.resize(res->body.size() - 3); // removes (*)

                return parseServers(res->body);
            }
        }

        MessageBoxA(nullptr, "An error occured while getting servers list.\nPlease make sure you have a stable internet connection and relaunch the app.", "Caution!", MB_OK | MB_ICONWARNING);

        exit(1);
    }

}