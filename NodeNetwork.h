#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class NodeNetwork {
private:
    SOCKET listenSocket;
    std::map<int, SOCKET> peerSockets; // מיפוי בין מזהה שרת (Node ID) לסוקט שלו
    bool isInitialized;

    void setNonBlocking(SOCKET socket);

public:
    NodeNetwork();
    ~NodeNetwork();

    // אתחול השרת המקומי והאזנה לחיבורים
    bool startServer(int myPort);

    // התחברות לשרת אחיד ברשת לפי IP ופורט
    bool connectToPeer(int peerId, const std::string& ip, int port);

    // קבלת חיבורים נכנסים (מבוצע בלופ המרכזי)
    void acceptIncomingConnections();

    // 1. שלח הודעה לשרת מסוים לפי ID
    bool sendMessage(int peerId, const std::string& message);

    // 2. קבלת הודעה משרת מסוים (החזרה ריקה אם אין הודעות)
    std::string receiveMessage(int peerId);

    // 3. בדיקה אם לא נשלח כלום מאף שרת (מחזיר true אם אין שום הודעה חדשה ברשת)
    bool isNetworkEmpty();
};