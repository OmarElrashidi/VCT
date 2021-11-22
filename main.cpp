/*
  TODO:
    - Finish all connection flow
    - Add a dynamic server list
    - Backup the original file
    
    - Add an experimental feature to only connect a certain game is running
    - Add an option to select prefered country
    - Add an option to reset everything
    - Add an option to auto reconnect on different servers (if the server is down)
    - Add start with windows
    - Add toast notifications
    - Add support for tray icon
    - Add credits
*/
/*
  Connection workflow:
    - Check if there's vc tunnel vpn to decide if we need to setup the vpn connection,
       and do so if needed.

    - Incase there was no setup, we add the vpn connection by appending to the
       rasphone.pbk file. 

    - Use rasdial to connect to the vpn and make sure it got enough attempts, if not we warn the user. 
      (has an option to auto reconnect, modify attempts or trying another servers automatically)

    - Start the temporary routing using iphlpapi.h and do all neccessary checks to make sure it's actually working.
*/

#include "vpn.h"

using namespace nanogui;

inline auto vpn = new VPN();

void onExit()
{
    if (vpn)
        vpn->Disconnect();
}

//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    nanogui::init();

    /* scoped variables */ {
        auto screen = new Screen(Vector2i(625, 420), VPN_CONNECTION_NAME, true);

        auto gui = new FormHelper(screen);
        auto window = gui->addWindow(Eigen::Vector2i(0, 0), VPN_CONNECTION_NAME);

        auto layout = new GroupLayout(50);
        window->setLayout(layout);

        /* themeing */
        screen->setBackground(Color(Eigen::Vector4i(0, 0, 0, 255)));
        window->theme()->mTextColor = Color(Eigen::Vector4i(86, 206, 202, 255));
        gui->setLabelFontSize(24);

        auto lol = new Label(window, "Servers:", "sans", 24);

        auto serversPanel = new PopupButton(window, "Pick a server");
        gui->addWidget("", serversPanel);
        serversPanel->setIcon(ENTYPO_ICON_SIGNAL);

        auto popup = serversPanel->popup();
        popup->setFixedSize(Vector2i(170, 300));

        auto scrollPanel = new VScrollPanel(popup);

        auto buttons = new Widget(scrollPanel);
        buttons->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Middle, 30, 20));

        detail::FormWidget<std::string>* ping;

        auto pingStrLamb = [&](const std::string& str)
        {
            auto p = util::ping(str.c_str());
            std::string s = "N/A (Don't use!)";

            if (p != -1)
            {
                s = std::to_string(p) + " ms";
            }

            ping->setValue(s);
        };

        for (auto &&p : serversList)
        {
            auto btn = new Button(buttons, p.first);
            btn->setCallback([&]()
                             {
                                 serversPanel->setCaption(p.first);
                                 pingStrLamb(p.first);
                                 vpn->changeServer(p.second);
                             });
        }

        std::string pingStr = "N/A";
        ping = gui->addVariable("Ping: ", pingStr, false);
        ping->setFixedSize(Vector2i(200, 30));

        std::string connStatusStr = "Not Connected";

        auto connectionStatus = gui->addVariable("Status: ", connStatusStr, false);
        connectionStatus->setFixedSize(Vector2i(200, 30));

        auto connectBtn = gui->addButton("Connect", [&]() {});

        connectBtn->setFixedSize(Vector2i(300, 100));
        connectBtn->setIcon(ENTYPO_ICON_PAPER_PLANE);

        auto buttonFn = [&]()
        {
            if (!vpn->isConnected())
            {
                connectionStatus->setValue("Connecting...");

                if (vpn->Connect())
                {
                    connectionStatus->setValue("Connected!");
                    connectBtn->setCaption("Disconnect");
                }
                else
                {
                    connectionStatus->setValue("Failed!");
                }
            }
            else
            {
                connectionStatus->setValue("Disconnecting...");

                if (vpn->Disconnect())
                {
                    connectionStatus->setValue("Disconnected!");
                    connectBtn->setCaption("Connect");
                }
                else
                {
                    connectionStatus->setValue("Failed!");
                }
            }
        };

        connectBtn->setCallback(buttonFn);

        //TODO
        /*
        ref<Window> logsWindow = gui->addWindow(Eigen::Vector2i(0, 0), "LOGS");
        logs = new TextBox(logsWindow, logString);
        logs->setFontSize(16);
        gui->addWidget("", logs);
        */

        screen->setVisible(true);
        screen->performLayout();

        nanogui::mainloop();
    }

    std::atexit(onExit);

    nanogui::shutdown();
    return 0;
}
