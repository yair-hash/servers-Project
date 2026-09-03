#include <iostream>
#include <thread>
#include <chrono>
#include "NodeNetwork.h"

int main() {
    NodeNetwork net;
    
    // הפעלת השרת על פורט מוגדר
    net.startServer(8080);

    // דוגמה להתחברות לשרת שכן (ניתן להוסיף עד 4 שרתים נוספים)
    // net.connectToPeer(1, "127.0.0.1", 8081);

    while (true) {
        // בדיקת חיבורים נכנסים חדשים
        net.acceptIncomingConnections();

        // 3. בדיקה אם לא נשלח כלום מאף שרת
        if (net.isNetworkEmpty()) {
            std::cout << "No messages received from any server..." << std::endl;
        } else {
            // 2. קבלת הודעה משרת מזהה 1
            std::string msg = net.receiveMessage(1);
            if (!msg.empty()) {
                std::cout << "New message from Node 1: " << msg << std::endl;
            }
        }

        // 1. שליחת הודעה לשרת מזהה 1
        // net.sendMessage(1, "Hello Node 1");

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}