//Some of them require the LOG template from cpp.cpp (or just remove/modify it)


void reboot()
{
    LOG("Rebooting...");
    ESP.restart();
}

void reboot_hard()
{
    LOG("Hard restart...");
    ESP.restart();
}

void status()
{
    String uptime = usToTime(esp_timer_get_time());
    LOG("Uptime:\t\t", uptime);

    int freq = getCpuFrequencyMhz();
    LOG("CPU freq:\t", freq, "MHz");

    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxHeap = ESP.getHeapSize();
    LOG("Free heap:\t", freeHeap*1.0/1024, " KiB (", (freeHeap*100)/maxHeap, "%)");

    int WiFiStatus = WiFi.status() == 255 ? -1 : WiFi.status();
    LOG("WiFi status:\t", WiFiStatus, " (", WiFi.SSID().length() ? WiFi.SSID() : "null" , ")");
    LOG("Mac address:\t", WiFi.macAddress());
    LOG("Local IP:\t", WiFi.localIP());
    LOG("Gateway IP:\t", WiFi.gatewayIP());

}

int connect(String SSID, String pass)
{
    LOG("Connecting to WiFi network ", SSID, " ...");
    WiFi.begin(SSID.c_str(), pass.c_str());
    WiFi.waitForConnectResult();

    int status =  WiFi.status();
    LOG("Status: ", status ==  WL_CONNECTED ? "Connected" : String(status) );

    return status;
}

int connect(String SSID, String pass)
{
    LOG("Connecting to WiFi network ", SSID, " ...");
    WiFi.begin(SSID.c_str(), pass.c_str());
    WiFi.waitForConnectResult();

    int status =  WiFi.status();
    LOG("Status: ", status ==  WL_CONNECTED ? "Connected" : String(status) );

    return status;
}

int disconnect()
{
    int code = WiFi.disconnect();
    LOG("Disconnected. Code: ", code);
    return code;
}

/*
 * Blocking function to get a string from serial (or one character)
 */
String getSerialInput(bool oneChar)
{
    String serialMessage;
    while(1)
    {
        if(Serial.peek()>0)
        {
            char c = Serial.read();
            if(oneChar) return String(c);
            if (c == '\n' || c == '\r')
            {
                Serial.flush();
                return serialMessage;
            }
            else serialMessage += c;
        }
        yield();
        delay(100);
    }
}

/*
 * Function to be called periodically to see if we received a serial command
 */
void handleSerialCommunication()
{
    static String serialMessage;
    while(Serial.peek()>0)
    {
        char c = Serial.read();
        if (c == '\n' || c == '\r')
        {
            Serial.flush();
            handleSerialMessage(serialMessage); // <-- implement this
            serialMessage = "";
        }
        else serialMessage += c;
    }
}

// Implement (or refactor) networks.list to be std::vector<WiFiParam> list;
// Implement (or refactor) struct WiFiParam { String SSID; uint8_t encryption; int32_t RSSI; };
// Requires terminal colors defines
// include algorithm, vector
// Also include lambdas and operator overloads below the function (preferably into the header file)
int scan()
{
    int n;

    LOG("Scanning for WiFi networks...");
    
    WiFi.scanDelete();
    networks.list.clear();

    n = WiFi.scanNetworks();

    for(int i=0; i<n; i++)
    { 
        WiFiParam network = {WiFi.SSID(i), WiFi.encryptionType(i), WiFi.RSSI(i)};
        networks.list.push_back(network);
    }

    //Dump all scanned data to serial
    LOG(T_GREEN, "Found ", n, " APs", T_CLEAR);
    LOG("---------------------------------------------------------");
    for(auto const& i : networks.list)
    {
        String color = i.RSSI >=-67 ? T_GREEN : (i.RSSI >=-70 ? T_YELLOW : T_RED);
        LOG(i.SSID, "\r\t\t\t (", Auth(i.encryption), ") \t", color, i.RSSI, " dBm", T_CLEAR);
    }
    LOG("---------------------------------------------------------");

    //Remove duplicate networks, keep the strongest (repeaters with same SSID)   
	std::sort(networks.list.begin(), networks.list.end(), less_by_name_signal);
	networks.list.erase(std::unique(networks.list.begin(), networks.list.end(), WiFi_eq_by_SSID), networks.list.end());
    LOG(T_GREEN, networks.list.size(), T_CLEAR, " unique SSIDs");

    //Sort by RSSI
    std::sort(networks.list.begin(), networks.list.end(), CompareByRSSI);

    return n;
}

auto less_by_name_signal = [](const WiFiParam& lhs, const WiFiParam& rhs)
{
	return std::tie(lhs.SSID, rhs.RSSI) < std::tie(rhs.SSID, lhs.RSSI);
};
auto WiFi_eq_by_SSID = [](const WiFiParam& lhs, const WiFiParam& rhs)
{
	return lhs.SSID == rhs.SSID;
};

inline bool operator==(const WiFiParam& lhs, const WiFiParam& rhs){ return !(lhs.SSID == rhs.SSID); }
