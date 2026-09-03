#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <atomic>
#include "NodeNetwork.h"

// דגל סנכרון לסגירה נקי של ה-Thread
std::atomic<bool> keepRunning(true);

// מתודת רקע לקבלת חיבורים והודעות
void receiveWorker(NodeNetwork& net) {
    while (keepRunning) {
        // בדיקת חיבורים נכנסים חדשים
        net.acceptIncomingConnections();

        // מעבר על מזהים אפשריים ברשת לקבלת הודעות (למשל מ-1 עד 10)
        for (int id = 1; id <= 10; ++id) {
            std::string msg = net.receiveMessage(id);
            if (!msg.empty()) {
                std::cout << "\n[Message from Node " << id << "]: " << msg << std::endl;
                std::cout << "Enter command (<NodeID> <Message>): " << std::flush;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    NodeNetwork net;

    // 1. הפעלת השרת המקומי (שני את הפורט לפי המחשב הנוכחי)
    int myPort = 8080; 
    if (!net.startServer(myPort)) {
        std::cerr << "Failed to start server on port " << myPort << std::endl;
        return 1;
    }
    std::cout << "Server listening on port " << myPort << "...\n";

    // 2. התחברות לשרתים שכנים במידת הצורך (דוגמה להתחברות ל-Node 1)
     net.connectToPeer(1, "10.100.102.10", 8080);

    // 3. הפעלת Thread הרקע לקבלה
    std::thread rxThread(receiveWorker, std::ref(net));

    std::cout << "Ready! Enter messages in format: <NodeID> <Message>\n";
    std::cout << "Example: 1 Hello World\n";
    std::cout << "--------------------------------------------------\n";

    // 4. לולאה ראשית לקריאה מהמקלדת ושליחה
    std::string line;
    while (keepRunning && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line == "exit") {
            keepRunning = false;
            break;
        }

        std::stringstream ss(line);
        int targetId;
        std::string message;

        // חילוץ ה-ID המיועד והטקסט
        if (ss >> targetId) {
            std::getline(ss >> std::ws, message); // קריאת שאר השורה כהודעה
            
            if (!message.empty()) {
                bool sent = net.sendMessage(targetId, message);
                if (sent) {
                    std::cout << "-> Sent to Node " << targetId << ": " << message << std::endl;
                } else {
                    std::cout << "x Failed to send to Node " << targetId << " (Not connected)" << std::endl;
                }
            }
        } else {
            std::cout << "Invalid format. Use: <NodeID> <Message>" << std::endl;
        }
    }

    if (rxThread.joinable()) {
        rxThread.join();
    }

    return 0;
}