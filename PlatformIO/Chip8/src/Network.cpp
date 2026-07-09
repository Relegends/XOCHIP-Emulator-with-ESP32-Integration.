#include "Network.h"


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

Network::Network() :
    m_role(NETWORK_ROLE::NONE),
    m_playerID(0),
    m_connected(false),
    m_sequenceID(0),
    m_socket(-1),
    m_peerIPs({}),
    m_peerIDs({}),
    m_peerLastSeen({}),
    m_peerLastSeq({}),
    m_peerCount(0),
    m_networkQueue(nullptr),
    m_networkTaskHandle(nullptr),
    m_wifiEventGroup(nullptr),
    m_wifiChannel(0),
    m_isWifiDriverInit(false),
    m_apNetif(nullptr),
    m_staNetif(nullptr)
{
}

Network::~Network()
{
    Disconnect();
}

bool Network::Init(const std::string& deviceName, uint8_t wifiChannel)
{
    LOG("{Network} Initializing. Device: %s, Channel: %d", deviceName.c_str(), wifiChannel);

    m_deviceName  = deviceName;
    m_wifiChannel = wifiChannel;

    m_networkQueue = xQueueCreate(32, sizeof(NetworkPacket));
    if (!m_networkQueue) {
        LOGE("{Network} Failed to create RX queue");
        return false;
    }

    m_wifiEventGroup = xEventGroupCreate();
    if (!m_wifiEventGroup) {
        LOGE("{Network} Failed to create WiFi event group");
        return false;
    }

    // NVS is required by the WiFi driver
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        LOGE("{Network} NVS init failed: %d", err);
        return false;
    }

    err = esp_netif_init();
    if (err != ESP_OK) {
        LOGE("{Network} netif init failed: %d", err);
        return false;
    }

    // esp_event_loop_create_default() returns ESP_ERR_INVALID_STATE if already created
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        LOGE("{Network} Event loop create failed: %d", err);
        return false;
    }

    // WiFi driver must only be initialized once
    if (!m_isWifiDriverInit) {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&cfg);
        if (err != ESP_OK) {
            LOGE("{Network} WiFi driver init failed: %d", err);
            return false;
        }
        m_isWifiDriverInit = true;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Network::WiFiEventHandler, this, nullptr);
    if (err != ESP_OK) {
        LOGE("{Network} WiFi event handler register failed: %d", err);
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Network::WiFiEventHandler, this, nullptr);
    if (err != ESP_OK) {
        LOGE("{Network} IP event handler register failed: %d", err);
        return false;
    }

    LOG("{Network} Initialization complete");
    return true;
}

void Network::WiFiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    Network* net = static_cast<Network*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
        xEventGroupSetBits(net->m_wifiEventGroup, WIFI_FAIL_BIT);
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        xEventGroupSetBits(net->m_wifiEventGroup, WIFI_CONNECTED_BIT);
}

bool Network::Host(const std::string& ssid, const std::string& password)
{
    Disconnect();
    LOG("{Network} Starting AP: %s", ssid.c_str());

    if (!m_apNetif)
        m_apNetif = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, ssid.c_str(), sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char*)wifi_config.ap.password, password.c_str(), sizeof(wifi_config.ap.password) - 1);

    wifi_config.ap.ssid_len = static_cast<uint8_t>(ssid.size());
    wifi_config.ap.channel = m_wifiChannel;
    wifi_config.ap.max_connection = NET_MAX_PEERS;
    wifi_config.ap.authmode = password.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK) {
        LOGE("{Network} Host: set mode failed");
        return false;
    }
    if (esp_wifi_set_config(WIFI_IF_AP, &wifi_config) != ESP_OK) {
        LOGE("{Network} Host: set config failed");
        return false;
    }
    if (esp_wifi_start() != ESP_OK) {
        LOGE("{Network} Host: wifi start failed");
        return false;
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket < 0) {
        LOGE("{Network} Host: socket create failed");
        esp_wifi_stop();
        return false;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(NET_PORT);

    if (bind(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        LOGE("{Network} Host: bind failed");
        close(m_socket);
        m_socket = -1;
        esp_wifi_stop();
        return false;
    }

    // 500 ms receive timeout so NetworkLoop can periodically check peer timeouts
    struct timeval tv = { .tv_sec = 0, .tv_usec = 500000 };
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    m_playerID = 1;
    m_role = NETWORK_ROLE::HOST;
    m_connected = true;

    xTaskCreate(NetworkTask, "NetworkTask", 4096, this, 5, &m_networkTaskHandle);
    LOG("{Network} Hosting OK, player ID 1");
    return true;
}

bool Network::Join(const std::string& ssid, const std::string& password)
{
    Disconnect();
    LOG("{Network} Joining AP: %s", ssid.c_str());

    xEventGroupClearBits(m_wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    if (!m_staNetif)
        m_staNetif = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid,     ssid.c_str(),     sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password) - 1);

    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        LOGE("{Network} Join: set mode failed");
        return false;
    }
    if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) {
        LOGE("{Network} Join: set config failed");
        return false;
    }
    if (esp_wifi_start() != ESP_OK) {
        LOGE("{Network} Join: wifi start failed");
        return false;
    }
    // Retry connecting until the AP appears or 30 s elapses.
    // Each attempt waits up to 3 s; WIFI_FAIL_BIT fires immediately when
    // the AP is not found, so short per-attempt windows keep retry cadence tight.
    const int64_t deadline = esp_timer_get_time() + 30000000LL; // 30 s
    EventBits_t bits = 0;
    while (true)
    {
        xEventGroupClearBits(m_wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
        if (esp_wifi_connect() != ESP_OK) {
            LOGE("{Network} Join: connect call failed");
            esp_wifi_stop();
            return false;
        }
        bits = xEventGroupWaitBits(
            m_wifiEventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE,
            pdMS_TO_TICKS(3000)
        );
        if (bits & WIFI_CONNECTED_BIT) break;
        if (esp_timer_get_time() >= deadline)
        {
            LOGE("{Network} Join: timeout waiting for AP");
            esp_wifi_stop();
            return false;
        }
        LOG("{Network} Join: AP not found, retrying...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket < 0)
    {
        LOGE("{Network} Join: socket create failed");
        esp_wifi_stop();
        return false;
    }

    // Bind to NET_PORT so HOST can address subsequent packets to CLIENT_IP:NET_PORT.
    // Without this, the OS assigns a random ephemeral port; HOST always sends to
    // CLIENT_IP:NET_PORT, so those packets would be silently dropped.
    {
        struct sockaddr_in local_addr = {};
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_port = htons(NET_PORT);
        if (bind(m_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
        {
            LOGE("{Network} Join: bind failed");
            close(m_socket);
            m_socket = -1;
            esp_wifi_stop();
            return false;
        }
    }

    // Host is always the SoftAP gateway: 192.168.4.1
    struct sockaddr_in host_addr = {};
    host_addr.sin_family = AF_INET;
    host_addr.sin_addr.s_addr = inet_addr("192.168.4.1");
    host_addr.sin_port = htons(NET_PORT);

    // Send handshake
    NetworkPacket hello = {};
    hello.playerID = 0;
    memcpy(hello.payload, "HELLO", 5);
    hello.sequenceID = 0;
    sendto(m_socket, &hello, sizeof(hello), 0, (struct sockaddr*)&host_addr, sizeof(host_addr));

    // Wait up to 5 seconds for the host to reply with our assigned player ID
    struct timeval tv = {};
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in from = {};
    socklen_t fromLen = sizeof(from);
    NetworkPacket reply = {};
    int n = recvfrom(m_socket, &reply, sizeof(reply), 0, (struct sockaddr*)&from, &fromLen);

    // Switch from the 5 s handshake timeout to the permanent 500 ms loop timeout
    tv = { .tv_sec = 0, .tv_usec = 500000 };
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (n != (int)sizeof(NetworkPacket) || reply.playerID == 0)
    {
        LOGE("{Network} Join: handshake failed (n=%d, pid=%d)", n, reply.playerID);
        close(m_socket);
        m_socket = -1;
        esp_wifi_stop();
        return false;
    }

    m_playerID = reply.playerID;
    m_peerIPs[0] = inet_addr("192.168.4.1");
    m_peerIDs[0] = 1; // host is always player 1
    m_peerLastSeen[0] = esp_timer_get_time();
    m_peerCount = 1;
    m_role = NETWORK_ROLE::CLIENT;
    m_connected = true;

    xTaskCreate(NetworkTask, "NetworkTask", 4096, this, 5, &m_networkTaskHandle);
    LOG("{Network} Joined OK, player ID %d", m_playerID);
    return true;
}

void Network::Disconnect()
{
    if (m_role == NETWORK_ROLE::NONE && !m_connected)
        return;

    m_connected = false;

    // Close the socket first — this unblocks any pending recvfrom in NetworkTask,
    // which will then see m_connected=false, break its loop, and self-delete via
    // vTaskDelete(nullptr). We never call vTaskDelete here to avoid double-free
    // when NetworkTask already self-deleted (e.g. because CheckPeerTimeouts fired).
    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
        // Give NetworkTask time to notice the closed socket and self-delete
        // before Host()/Join() spawn a new one.
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    m_networkTaskHandle = nullptr;

    esp_wifi_stop();

    m_role = NETWORK_ROLE::NONE;
    m_playerID  = 0;
    m_peerCount = 0;
    m_peerIPs      = {};
    m_peerIDs      = {};
    m_peerLastSeen = {};
    m_peerLastSeq  = {};
    m_sequenceID   = 0;

    if (m_networkQueue)
        xQueueReset(m_networkQueue);

    if (m_wifiEventGroup)
        xEventGroupClearBits(m_wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    LOG("{Network} Disconnected");
}

bool Network::Send(const uint8_t* payload, size_t len)
{
    if (!m_connected || m_socket < 0)
        return false;

    NetworkPacket pkt = {};
    pkt.playerID = m_playerID;
    pkt.sequenceID = m_sequenceID++;
    memcpy(pkt.payload, payload, std::min(len, NET_PAYLOAD_SIZE));

    bool ok = true;

    if (m_role == NETWORK_ROLE::CLIENT) 
    {
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.4.1");
        addr.sin_port = htons(NET_PORT);
        ok = (sendto(m_socket, &pkt, sizeof(pkt), 0, (struct sockaddr*)&addr, sizeof(addr)) == (int)sizeof(pkt));
        return ok;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    for (uint8_t i = 0; i < m_peerCount; i++)
    {
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = m_peerIPs[i];
        addr.sin_port = htons(NET_PORT);
        ok &= (sendto(m_socket, &pkt, sizeof(pkt), 0, (struct sockaddr*)&addr, sizeof(addr)) == (int)sizeof(pkt));
    }

    return ok;
}

bool Network::Receive(NetworkPacket& dataReceived)
{
    if (!m_networkQueue)
        return false;
    return xQueueReceive(m_networkQueue, &dataReceived, 0) == pdTRUE;
}

bool Network::HasPending() const
{
    if (!m_networkQueue)
        return false;
    return uxQueueMessagesWaiting(m_networkQueue) > 0;
}

Network::NETWORK_ROLE Network::GetRole() const 
{ 
    return m_role; 
}

uint8_t Network::GetPlayerID() const 
{
    return m_playerID;
}

uint8_t Network::GetPeerCount() const
{
    return m_peerCount;
}

bool Network::IsConnected() const
{
    return m_connected;
}

uint32_t Network::GetPeerIP(uint8_t peerSlot) const
{
    if (peerSlot >= NET_MAX_PEERS)
        return 0;
    return m_peerIPs[peerSlot];
}

void Network::NetworkTask(void* arg)
{
    Network* net = static_cast<Network*>(arg);
    net->NetworkLoop();
    net->m_networkTaskHandle = nullptr;  // clear before self-delete so Disconnect() won't try to re-delete
    vTaskDelete(nullptr);
}

void Network::NetworkLoop()
{
    NetworkPacket packet;
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);

    while (true)
    {
        int n = recvfrom(m_socket, &packet, sizeof(packet), 0, (struct sockaddr*)&from, &fromLen);

        if (n != (int)sizeof(NetworkPacket))
        {
            if (!m_connected) break;
            CheckPeerTimeouts();
            if (!m_connected) break; // all peers gone
            continue;
        }

        uint32_t senderIP = from.sin_addr.s_addr;

        if (m_role == NETWORK_ROLE::CLIENT)
        {
            { std::lock_guard<std::mutex> lock(m_mutex); m_peerLastSeen[0] = esp_timer_get_time(); }
            xQueueSend(m_networkQueue, &packet, 0);
            continue;
        }

        // Data packet from a known peer
        if (packet.playerID != 0)
        {
            bool isDuplicate = false;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (uint8_t i = 0; i < m_peerCount; i++)
                {
                    if (m_peerIPs[i] != senderIP) continue;
                    m_peerLastSeen[i] = esp_timer_get_time();
                    if (packet.sequenceID == m_peerLastSeq[i])
                    {
                        LOG("{Network} Duplicate packet seq=%d from slot %d, dropped", packet.sequenceID, i);
                        isDuplicate = true;
                    }
                    else
                    {
                        m_peerLastSeq[i] = packet.sequenceID;
                    }
                    break;
                }
            }
            if (!isDuplicate)
            {
                RelayToOtherClients(packet, senderIP);
                xQueueSend(m_networkQueue, &packet, 0);
            }
            continue; // never fall through to handshake
        }

        // playerID == 0: HELLO handshake from client
        // Re-handshake: client already registered, re-send its player ID
        uint8_t existingID = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (uint8_t i = 0; i < m_peerCount; i++)
            {
                if (m_peerIPs[i] == senderIP) { existingID = m_peerIDs[i]; break; }
            }
        }
        if (existingID != 0)
        {
            NetworkPacket reply = {};
            reply.playerID = existingID;
            sendto(m_socket, &reply, sizeof(reply), 0, (struct sockaddr*)&from, fromLen);
            LOG("{Network} Re-handshake: re-sent player ID %d", existingID);
            continue;
        }

        // New client: find lowest available player ID and register
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_peerCount >= NET_MAX_PEERS)
            {
                LOG("{Network} Rejecting client %08X: session full", senderIP);
                continue;
            }

            uint8_t assignedID = 0;
            for (uint8_t id = 2; id <= NET_MAX_PEERS + 1; id++)
            {
                bool inUse = false;
                for (uint8_t j = 0; j < m_peerCount; j++)
                    if (m_peerIDs[j] == id) { inUse = true; break; }
                if (!inUse) { assignedID = id; break; }
            }

            m_peerIPs[m_peerCount]      = senderIP;
            m_peerIDs[m_peerCount]      = assignedID;
            m_peerLastSeen[m_peerCount]  = esp_timer_get_time();
            m_peerLastSeq[m_peerCount]   = 0xFF; // sentinel: seq=0 won't be a false duplicate
            m_peerCount++;

            NetworkPacket reply = {};
            reply.playerID = assignedID;
            sendto(m_socket, &reply, sizeof(reply), 0, (struct sockaddr*)&from, fromLen);
            LOG("{Network} Client joined, assigned player ID %d", assignedID);
        }
    }
}

void Network::RelayToOtherClients(const NetworkPacket& packet, uint32_t senderIP)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (uint8_t i = 0; i < m_peerCount; i++)
    {
        if (m_peerIPs[i] == senderIP) continue;

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = m_peerIPs[i];
        addr.sin_port = htons(NET_PORT);
        sendto(m_socket, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
}

void Network::CheckPeerTimeouts()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    int64_t now = esp_timer_get_time();

    for (int8_t i = (int8_t)m_peerCount - 1; i >= 0; i--)
    {
        int64_t elapsed_ms = (now - m_peerLastSeen[i]) / 1000LL;
        LOG("{Network} peer slot %d (pid=%d) last seen %lld ms ago", i, m_peerIDs[i], elapsed_ms);
        if (elapsed_ms > (int64_t)NET_PEER_TIMEOUT_MS)
            RemovePeer((uint8_t)i);
    }
}

void Network::RemovePeer(uint8_t peerSlot)
{
    uint8_t disconnectedID = m_peerIDs[peerSlot];
    LOGE("{Network} Player %d timed out, removing from session", disconnectedID);

    // Push a synthetic disconnect notification so the game can react via NET RECV.
    // playerID = NET_DISCONNECT_ID (0xFF), payload[0] = which player left.
    NetworkPacket notification = {};
    notification.playerID  = NET_DISCONNECT_ID;
    notification.payload[0] = disconnectedID;
    xQueueSend(m_networkQueue, &notification, 0);

    // Relay the disconnect notification to all remaining peers (called from
    // CheckPeerTimeouts which already holds m_mutex, so no re-lock needed).
    for (uint8_t i = 0; i < m_peerCount; i++)
    {
        if (i == peerSlot) continue;
        struct sockaddr_in addr = {};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = m_peerIPs[i];
        addr.sin_port        = htons(NET_PORT);
        sendto(m_socket, &notification, sizeof(notification), 0,
               (struct sockaddr*)&addr, sizeof(addr));
    }

    // Compact the arrays so there are no gaps
    for (uint8_t i = peerSlot; i < m_peerCount - 1; i++)
    {
        m_peerIPs[i] = m_peerIPs[i + 1];
        m_peerIDs[i] = m_peerIDs[i + 1];
        m_peerLastSeen[i] = m_peerLastSeen[i + 1];
    }
    m_peerCount--;

    // No peers left: client is isolated; host stays alive to accept reconnects
    if (m_peerCount == 0 && m_role != NETWORK_ROLE::HOST)
        m_connected = false;
}
