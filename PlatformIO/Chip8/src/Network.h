#pragma once
#include <cstring>
#include <string>
#include <array>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <lwip/sockets.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <lwip/inet.h>
#include <esp_timer.h>
#include "Log.h"

static constexpr size_t   NET_PAYLOAD_SIZE    = 16;
static constexpr uint16_t NET_PORT            = 7777;
static constexpr uint8_t  NET_MAX_PEERS       = 2;
static constexpr uint8_t  NET_DISCONNECT_ID   = 0xFF; // synthetic playerID for peer-lost notifications
static constexpr uint32_t NET_PEER_TIMEOUT_MS = 10000; // ms of silence before a peer is considered gone

struct NetworkPacket
{
    uint8_t playerID;
    uint8_t payload[NET_PAYLOAD_SIZE];
    uint8_t sequenceID;
};

class Network
{
public:
    enum NETWORK_ROLE 
    { 
        NONE = 0,
        HOST,
        CLIENT 
    };

    Network();
    ~Network();

    bool Init(const std::string& deviceName, uint8_t wifiChannel);

    // Session lifecycle
    bool Host(const std::string& ssid, const std::string& password);
    bool Join(const std::string& ssid, const std::string& password);
    void Disconnect();

    // Data exchange (called from opcode handlers)
    bool Send(const uint8_t* payload, size_t len);
    bool Receive(NetworkPacket& dataReceived);
    bool HasPending() const; // non-consuming check for NET WAIT

    // Status queries
    NETWORK_ROLE GetRole() const;
    uint8_t GetPlayerID() const;
    uint8_t GetPeerCount() const;
    bool IsConnected() const;
    uint32_t GetPeerIP(uint8_t peerSlot) const;

private:
    static void NetworkTask(void* arg);
    static void WiFiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    void NetworkLoop();
    void RelayToOtherClients(const NetworkPacket& pt, uint32_t senderIP);
    void CheckPeerTimeouts();
    void RemovePeer(uint8_t peerSlot); // must be called with m_mutex held
    
private:   
    std::string m_deviceName;
    uint8_t m_wifiChannel;
    
    NETWORK_ROLE m_role;
    uint8_t m_playerID;
    bool m_connected;
    uint8_t m_sequenceID;

    int m_socket;
    std::array<uint32_t, NET_MAX_PEERS> m_peerIPs;
    std::array<uint8_t,  NET_MAX_PEERS> m_peerIDs;       // player ID per slot
    std::array<int64_t,  NET_MAX_PEERS> m_peerLastSeen;  // esp_timer_get_time() per slot
    std::array<uint8_t,  NET_MAX_PEERS> m_peerLastSeq;   // last sequenceID seen per slot (duplicate filter)
    uint8_t m_peerCount;

    QueueHandle_t m_networkQueue;
    TaskHandle_t m_networkTaskHandle;
    EventGroupHandle_t m_wifiEventGroup;
    mutable std::mutex m_mutex;
    bool m_isWifiDriverInit;

    esp_netif_t* m_apNetif;
    esp_netif_t* m_staNetif;

};
