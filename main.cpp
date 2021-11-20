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
        Screen *screen = new Screen(Vector2i(600, 800), VPN_CONNECTION_NAME, true);

        bool enabled = true;
        FormHelper *gui = new FormHelper(screen);
        ref<Window> window = gui->addWindow(Eigen::Vector2i(0, 0), VPN_CONNECTION_NAME);
        window->setLayout(new GroupLayout());


        /* themeing */
        screen->setBackground(Color(Eigen::Vector4i(0, 0, 0, 255)));
        window->theme()->mTextColor = Color(Eigen::Vector4i(106, 211, 25, 255));
        gui->setLabelFontSize(24);

        auto serversCobo = new ComboBox(window, serversList);
        serversCobo->setFontSize(16);
        serversCobo->setFixedSize(Vector2i(100, 20));
        gui->addWidget("Server: ", serversCobo);

        std::string pingStr = "N/A";
        auto ping = gui->addVariable("Ping: ", pingStr, false);
        ping->setFixedSize(Vector2i(200, 30));

        auto pingStrLamb = [&](int idx)
        {
            auto p = util::ping(serversList[idx].c_str());
            std::string s = "N/A (Don't use!)";

            if (p != -1)
            {
                s = std::to_string(p) + " ms";
            }

            ping->setValue(s);
        };

        pingStrLamb(0);
        serversCobo->setCallback(pingStrLamb);

        PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
        gui->addWidget("", imagePanelBtn);
        imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
        auto popup = imagePanelBtn->popup();

        nanogui::VScrollPanel* scroll_panel = new nanogui::VScrollPanel(popup);
        scroll_panel->setFixedSize(nanogui::Vector2i(125, 200));
        nanogui::Widget* checkboxes = new nanogui::Widget(scroll_panel);
        checkboxes->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Minimum, 0, 0));
        for (int i = 0; i < 100; i++)
        {
            auto thing = new Button(checkboxes, "CheckBox " + std::to_string(i));
        }


        std::string connStatusStr = "Not Connected";

        auto connectionStatus = gui->addVariable("Status: ", connStatusStr, false);
        connectionStatus->setFixedSize(Vector2i(200, 30));

        auto connectBtn = gui->addButton("Connect", [&]() {});

        connectBtn->setFixedSize(Vector2i(300, 100));
        connectBtn->setIcon(ENTYPO_ICON_SIGNAL);

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
        window->center();

        nanogui::mainloop();
    }

    std::atexit(onExit);

    nanogui::shutdown();
    return 0;
}
