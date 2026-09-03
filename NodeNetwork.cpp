#include "NodeNetwork.h"

NodeNetwork::NodeNetwork() : listenSocket(INVALID_SOCKET), isInitialized(false) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        isInitialized = true;
    }
}

NodeNetwork::~NodeNetwork() {
    for (auto& pair : peerSockets) {
        if (pair.second != INVALID_SOCKET) {
            closesocket(pair.second);
        }
    }
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    if (isInitialized) {
        WSACleanup();
    }
}

void NodeNetwork::setNonBlocking(SOCKET socket) {
    u_long mode = 1; // 1 = Non-blocking, 0 = Blocking
    ioctlsocket(socket, FIONBIO, &mode);
}

bool NodeNetwork::startServer(int myPort) {
    if (!isInitialized) return false;

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) return false;

    setNonBlocking(listenSocket);

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(myPort);

    if (bind(listenSocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    return true;
}

bool NodeNetwork::connectToPeer(int peerId, const std::string& ip, int port) {
    SOCKET peerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (peerSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // התחברות לשרת השכן
    if (connect(peerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) { // בדיקה שאין שגיאה קריטית
            closesocket(peerSocket);
            return false;
        }
    }

    setNonBlocking(peerSocket);
    peerSockets[peerId] = peerSocket;
    return true;
}

void NodeNetwork::acceptIncomingConnections() {
    SOCKET incomingSocket = accept(listenSocket, NULL, NULL);
    if (incomingSocket != INVALID_SOCKET) {
        setNonBlocking(incomingSocket);
        // שמירת המזהה של השכן בצורה דינמית (לדוגמה לפי מספר הסוקט)
        int peerId = static_cast<int>(peerSockets.size() + 1);
        peerSockets[peerId] = incomingSocket;
    }
}

// 1. שלח הודעה
bool NodeNetwork::sendMessage(int peerId, const std::string& message) {
    if (peerSockets.find(peerId) == peerSockets.end()) return false;
    
    SOCKET s = peerSockets[peerId];
    int result = send(s, message.c_str(), static_cast<int>(message.length()), 0);
    return result != SOCKET_ERROR;
}

// 2. קבלת הודעה
std::string NodeNetwork::receiveMessage(int peerId) {
    if (peerSockets.find(peerId) == peerSockets.end()) return "";

    char buffer[1024] = {0};
    SOCKET s = peerSockets[peerId];
    int bytesReceived = recv(s, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        return std::string(buffer, bytesReceived);
    }
    return ""; // לא התקבלה הודעה או שאין מידע בממשק
}

// 3. בדיקה שלא נשלח כלום מאף שרת אליי
bool NodeNetwork::isNetworkEmpty() {
    char buffer[1024];
    for (auto& pair : peerSockets) {
        // MSG_PEEK קורא את המידע מהחוצץ מבלי למחוק אותו
        int bytes = recv(pair.second, buffer, sizeof(buffer), MSG_PEEK);
        if (bytes > 0) {
            return false; // נמצאה הודעה מחכה באחד הסוקטים
        }
    }
    return true; // שום שרת לא שלח כלום
}