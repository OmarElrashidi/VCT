#pragma once
#include "framework.h"
#include "util.h"

/*
 KNOWN ISSUE(S) :
    - If the adapter was connected before this was launched, it doesn't disconnect even it returns true 
*/

constexpr const char *VPN_CONNECTION_NAME = "VCT";
const std::vector<std::string> VOIP_ROUTES = {"3.0.0.0", "18.0.0.0", "34.0.0.0", "35.0.0.0", "50.0.0.0", "52.0.0.0", "54.0.0.0"};

std::string CONNECTION_DATA =
    R"([PLACEHOLDER]
Encoding=1
PBVersion=6
Type=2
AutoLogon=0
UseRasCredentials=1
LowDateTime=1298341856
HighDateTime=30923514
DialParamsUID=11566937
Guid=CA45014DD3BD1E43A8F69521B48FAA48
VpnStrategy=6
ExcludedProtocols=0
LcpExtensions=1
DataEncryption=8
SwCompression=0
NegotiateMultilinkAlways=0
SkipDoubleDialDialog=0
DialMode=0
OverridePref=15
RedialAttempts=3
RedialSeconds=60
IdleDisconnectSeconds=0
RedialOnLinkFailure=1
CallbackMode=0
CustomDialDll=
CustomDialFunc=
CustomRasDialDll=
ForceSecureCompartment=0
DisableIKENameEkuCheck=0
AuthenticateServer=0
ShareMsFilePrint=1
BindMsNetClient=1
SharedPhoneNumbers=0
GlobalDeviceSettings=0
PrerequisiteEntry=
PrerequisitePbk=
PreferredPort=VPN2-0
PreferredDevice=WAN Miniport (IKEv2)
PreferredBps=0
PreferredHwFlow=1
PreferredProtocol=1
PreferredCompression=1
PreferredSpeaker=1
PreferredMdmProtocol=0
PreviewUserPw=1
PreviewDomain=1
PreviewPhoneNumber=0
ShowDialingProgress=1
ShowMonitorIconInTaskBar=0
CustomAuthKey=0
AuthRestrictions=512
IpPrioritizeRemote=0
IpInterfaceMetric=0
IpHeaderCompression=0
IpAddress=0.0.0.0
IpDnsAddress=0.0.0.0
IpDns2Address=0.0.0.0
IpWinsAddress=0.0.0.0
IpWins2Address=0.0.0.0
IpAssign=1
IpNameAssign=1
IpDnsFlags=0
IpNBTFlags=1
TcpWindowSize=0
UseFlags=2
IpSecFlags=0
IpDnsSuffix=
Ipv6Assign=1
Ipv6Address=::
Ipv6PrefixLength=0
Ipv6PrioritizeRemote=1
Ipv6InterfaceMetric=0
Ipv6NameAssign=1
Ipv6DnsAddress=::
Ipv6Dns2Address=::
Ipv6Prefix=0000000000000000
Ipv6InterfaceId=0000000000000000
DisableClassBasedDefaultRoute=0
DisableMobility=0
NetworkOutageTime=0
IDI=
IDR=
ImsConfig=0
IdiType=0
IdrType=0
ProvisionType=0
PreSharedKey=
CacheCredentials=0
NumCustomPolicy=0
NumEku=0
UseMachineRootCert=0
Disable_IKEv2_Fragmentation=0
PlumbIKEv2TSAsRoutes=0
NumServers=0
RouteVersion=1
NumRoutes=0
NumNrptRules=0
AutoTiggerCapable=0
NumAppIds=0
NumClassicAppIds=0
SecurityDescriptor=
ApnInfoProviderId=
ApnInfoUsername=
ApnInfoPassword=
ApnInfoAccessPoint=
ApnInfoAuthentication=1
ApnInfoCompression=0
DeviceComplianceEnabled=0
DeviceComplianceSsoEnabled=0
DeviceComplianceSsoEku=
DeviceComplianceSsoIssuer=
WebAuthEnabled=0
WebAuthClientId=
FlagsSet=0
Options=0
DisableDefaultDnsSuffixes=0
NumTrustedNetworks=0
NumDnsSearchSuffixes=0
PowershellCreatedProfile=0
ProxyFlags=0
ProxySettingsModified=0
ProvisioningAuthority=
AuthTypeOTP=0
GREKeyDefined=0
NumPerAppTrafficFilters=0
AlwaysOnCapable=0
DeviceTunnel=0
PrivateNetwork=0

NETCOMPONENTS=
ms_msclient=1
ms_server=1

MEDIA=rastapi
Port=VPN2-0
Device=WAN Miniport (IKEv2)

DEVICE=vpn
PhoneNumber=N/A
AreaCode=
CountryCode=0
CountryID=0
UseDialingRules=0
Comment=
FriendlyName=
LastSelectedPhone=0
PromoteAlternates=0
TryNextAlternateOnFail=1
)";

class VPN
{
    HRASCONN connection = nullptr;
    PIP_ADAPTER_INFO _interface = nullptr;

public:
    std::string currentServer = "N/A";
    VPN()
    {
        printf("[VPN] Init, Getting servers list...\n");
        util::getServersList();
    }
    ~VPN()
    {
    }

    static void error(DWORD n)
    {
        char szBuf[256];
        if (RasGetErrorStringA((UINT)n, (LPSTR)szBuf, 256) != ERROR_SUCCESS)
        {
            sprintf((LPSTR)szBuf, "Undefined RAS Dial Error (%ld).", n);
        }

        MessageBoxA(NULL, (LPSTR)szBuf, "Error", MB_OK | MB_ICONSTOP);
    }

    bool setupVPN(const std::string &phonebookDir)
    {
        printf("[setupVPN] Setting up VPN.\n");

        util::findAndReplaceAll(CONNECTION_DATA, "PLACEHOLDER", VPN_CONNECTION_NAME);

        //Append the data to the phonebook.
        std::ofstream f;

        f.open(phonebookDir, std::ios_base::app);
        f << CONNECTION_DATA;
        f.close();

        ini::IniFile phonebook;
        phonebook.load(phonebookDir);

        auto vpnStrategy = phonebook[VPN_CONNECTION_NAME]["VpnStrategy"].as<int>();

        return (vpnStrategy == 6);
    }

    bool validateVPN()
    {
        //i could use RasValidateEntryNameA but WDC
        auto rasphoneDir = std::string(getenv("USERPROFILE")) + "\\AppData\\Roaming\\Microsoft\\Network\\Connections\\PBK\\rasphone.pbk";

        ini::IniFile phonebook;
        phonebook.load(rasphoneDir);

        auto vpnStrategy = phonebook[VPN_CONNECTION_NAME]["VpnStrategy"].as<int>();

        //in case our connection exists VpnStrategy will be 6
        if (vpnStrategy == 6)
        {
            printf("[validateVPN] VPN is already setup.\n");
            currentServer = phonebook[VPN_CONNECTION_NAME]["PhoneNumber"].as<std::string>();
            return true;
        }

        return setupVPN(rasphoneDir);
    }

    bool changeServer(std::string &server)
    {

        if (validateVPN())
        {
            printf("[changeServer] Changing server to %s\n", server.c_str());
            auto rasphoneDir = std::string(getenv("USERPROFILE")) + "\\AppData\\Roaming\\Microsoft\\Network\\Connections\\PBK\\rasphone.pbk";
            auto tmp_file_name = "temp.pbk";

            {
                std::ifstream original_file(rasphoneDir);
                std::ofstream temp_file(tmp_file_name);
                std::string line;
                while (std::getline(original_file, line))
                {
                    util::findAndReplaceAll(line, currentServer, server);
                    temp_file << line << "\n";
                }
                currentServer = server;
            }

            // overwrite the original file with the temporary file
            {
                std::ifstream temp_file(tmp_file_name);
                std::ofstream original_file(rasphoneDir);
                original_file << temp_file.rdbuf();
            }

            // and delete the temporary file
            std::remove(tmp_file_name);

            return true;
        }

        return false;
    }

    static PIP_ADAPTER_INFO getInterface()
    {
        PIP_ADAPTER_INFO pAdapterInfo;
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
        ULONG buflen = sizeof(IP_ADAPTER_INFO);

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW)
        {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO *)malloc(buflen);
        }

        if (GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR)
        {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            while (pAdapter)
            {
                //It will not find our adapter if it is not connected
                if (std::string(pAdapter->Description) == VPN_CONNECTION_NAME)
                {
                    printf("[getInterface] Found the interface!\n");

                    return pAdapter;
                }

                pAdapter = pAdapter->Next;
            }
        }
        else
        {
            printf("[getInterface] Failed to call GetAdaptersInfo!\n");
        }

        return NULL;
    }

    bool addRouting()
    {
        if (!_interface)
        {
            _interface = getInterface();
        }

        if (_interface)
        {
            printf("[addRouting] Adding routing to [%s]...\n", _interface->IpAddressList.IpAddress.String);

            for (auto &&ip : VOIP_ROUTES)
            {
                MIB_IPFORWARDROW ipForwardRow;
                memset(&ipForwardRow, 0, sizeof(ipForwardRow));

                ipForwardRow.dwForwardDest = inet_addr(ip.c_str());
                ipForwardRow.dwForwardMask = inet_addr("255.0.0.0");
                ipForwardRow.dwForwardNextHop = inet_addr(_interface->IpAddressList.IpAddress.String);
                ipForwardRow.dwForwardPolicy = 0;
                ipForwardRow.dwForwardProto = MIB_IPPROTO_NETMGMT;
                ipForwardRow.dwForwardType = MIB_IPROUTE_TYPE_DIRECT;
                ipForwardRow.dwForwardIfIndex = _interface->Index;
                ipForwardRow.dwForwardNextHopAS = 0;
                ipForwardRow.dwForwardMetric1 = 36;
                ipForwardRow.dwForwardMetric2 = -1;
                ipForwardRow.dwForwardMetric3 = -1;
                ipForwardRow.dwForwardMetric4 = -1;
                ipForwardRow.dwForwardMetric5 = -1;

                auto dwRet = CreateIpForwardEntry(&ipForwardRow);
                if (dwRet != NO_ERROR)
                {
                    error(dwRet);
                    printf("[addRouting] Failed to add routing!\n");
                }
            }

            return true;
        }
        else
        {
            printf("[addRouting] Failed to get interface!\n");
        }

        return false;
    }

    bool Connect()
    {
        if (validateVPN())
        {
            RASDIALPARAMSA params;
            SecureZeroMemory(&params, sizeof(params));

            params.dwSize = sizeof(RASDIALPARAMSA);
            params.szPhoneNumber[0] = '\0';
            params.szCallbackNumber[0] = '\0';
            params.szDomain[0] = '\0';

            lstrcpyA(params.szEntryName, VPN_CONNECTION_NAME);
            lstrcpyA(params.szUserName, "vpn");
            lstrcpyA(params.szPassword, "vpn");

            printf("[Connect] Connecting...\n");

            DWORD dwRet = RasDialA(NULL, NULL, &params, 0L, NULL, &connection);

            if (dwRet == ERROR_SUCCESS)
            {
                if (connection)
                {
                    printf("[Connect] Connected!\n");
                    if (addRouting())
                    {
                        //Everything is in place, what a great pog day!!
                        printf("[Connect] Routing was added, everything should be good, GLHF! :)\n");

                        return true;
                    }

                    printf("[Connect] Couldn't add the routing :(\n");
                }
            }

            printf("[Connect] Failed to connect!\n");

            error(dwRet);
        }
        else
        {
            printf("[Connect] Failed to validate VPN!\n");
        }

        return false;
    }

    bool isConnected()
    {
        DWORD dwRet;
        _interface = getInterface();
        if (_interface)
        {
            if (connection)
            {
                RASCONNSTATUSA RasConnStatus;
                RasConnStatus.dwSize = sizeof(RASCONNSTATUSA);

                RasGetConnectStatusA(connection, &RasConnStatus);

                if (RasConnStatus.rasconnstate == RASCS_Connected)
                {
                    return true;
                }
            }
            else
            {
                /*
                *   This is pretty goddamn annoying, 
                *   If you connected and somehow we didn't disconnect using THE SAME handle you can't hangup the connection
                *   Hell nah, not even windows rasdial itself or elevated CMD lmao, the ONLY way to do it is to disconnect it from the settings.
               */
                printf("[isConnected] Found the interface but we can't find it's handle.\n");

                MessageBoxA(nullptr, "You are already connected to a VCT Tunnel.\nPlease disconnect from it then press OK to try again.\n(TIP: Use the network icon in the taskbar, then press on VCT then press disconnect)", "Caution!", MB_OK | MB_ICONWARNING);

                return isConnected();
            }
        }
        else
        {
            printf("[isConnected] Couldn't find the interface.\n");
        }

        return false;
    }

    bool Disconnect()
    {
        if (isConnected())
        {
            printf("[Disconnect] Already connected!, Disconnecting...\n");

            auto dwRet = RasHangUpA(connection);
            if (dwRet == ERROR_SUCCESS)
            {
                printf("[Disconnect] Disconnected!\n");

                connection = nullptr;
                return true;
            }
            else
            {
                printf("[Disconnect] Failed to disconnect!\n");
            }

            error(dwRet);
        }

        return false;
    }
};
