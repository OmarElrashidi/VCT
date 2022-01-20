/*
  TODO:
    - Finish all connection flow (DONE)
    - Add a dynamic server list (DONE)
    - Redo the rasphone code to be more safe and less randuant
    - Backup the original file (SCRAPPED, WHO TF USES WINDOWS VPN LOL)

    - Impl config files
    - Add an experimental feature to only connect when a certain game is running
    - Add an option to reset everything
    - Add an option to auto reconnect on different servers (if the server is down)
    - Add start with windows
    - Add toast notifications
    - Add support for tray icon
    - Add credits
*/

#include "vpn.h"

using namespace nanogui;

inline auto vpn = new VPN();

void onExit()
{
    if (vpn)
        vpn->Disconnect();
}

// int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    nanogui::init();

    /* scoped variables */ {
        auto screen = new Screen(Vector2i(400, 400), VPN_CONNECTION_NAME, true);

        auto gui = new FormHelper(screen);
        auto window = gui->addWindow(Eigen::Vector2i(0, 0), VPN_CONNECTION_NAME);
        window->setFixedSize(Vector2i(400, 400));

        auto layout = new GroupLayout(50);
        window->setLayout(layout);

        /* themeing */
        screen->setBackground(Color(Eigen::Vector4i(0, 0, 0, 0)));
        window->theme()->mTextColor = Color(Eigen::Vector4i(106, 211, 25, 255));
        gui->setLabelFontSize(24);

        auto serversLabel = new Label(window, "Servers:", "sans", 24);
        auto serversBtn = new Button(window, "Select");

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
                    //nanogui::leave();
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

        auto credits = new Label(window, "Made by kemo", "sans", 24);

        // TODO
        /*
        ref<Window> logsWindow = gui->addWindow(Eigen::Vector2i(0, 0), "LOGS");
        logs = new TextBox(logsWindow, logString);
        logs->setFontSize(16);
        gui->addWidget("", logs);
        */

        auto windowServer = gui->addWindow(Eigen::Vector2i(0, 0), VPN_CONNECTION_NAME);
        windowServer->setLayout(new GridLayout(Orientation::Horizontal, 1, Alignment::Middle, 15, 5));

        windowServer->theme()->mWindowFillUnfocused = Color(Eigen::Vector4i(39, 39, 39, 255));
        windowServer->theme()->mWindowFillFocused = Color(Eigen::Vector4i(39, 39, 39, 255));
        windowServer->setFixedSize(Vector2i(400, 400));

        auto back = new Button(windowServer, "");
        // back->setIconPosition(Button::IconPosition::Left);
        back->setIcon(ENTYPO_ICON_CHEVRON_LEFT);
        back->setPosition(Vector2i(0, 0));
        back->setCallback([&]()
                          {
                              windowServer->setVisible(false);
                              window->setVisible(true); });

        serversBtn->setCallback([&]()
                                {
                                    windowServer->setVisible(true);
                                    window->setVisible(false); });

        auto vscroll = new VScrollPanel(windowServer);
        vscroll->setFixedSize({300, 250});

        auto wrapper = new Widget(vscroll);
        wrapper->setFixedSize({300, 250});
        wrapper->setLayout(new GridLayout(Orientation::Horizontal, 1, Alignment::Middle, 15, 5));

        detail::FormWidget<std::string> *ping;

        auto pingStrLamb = [&](const std::string &str)
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
            auto btn = new Button(wrapper, p.first);
            btn->setCallback([&]()
                             {
                                 serversBtn->setCaption(p.first);
                                 pingStrLamb(p.first);
                                 vpn->changeServer(p.second); });
        }

        std::string pingStr = "N/A";
        ping = gui->addVariable("Ping: ", pingStr, false);
        ping->setFixedSize(Vector2i(200, 30));

        screen->setVisible(true);
        screen->performLayout();

        nanogui::mainloop();
    }

    std::atexit(onExit);

    nanogui::shutdown();
    return 0;
}
