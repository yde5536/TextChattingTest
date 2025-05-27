#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void RunUDPServer() {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); 
    serverAddr.sin_addr.s_addr = INADDR_ANY; 

    iResult = bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "UDP 서버 시작. 포트 12345에서 대기 중..." << std::endl;

    char recvBuf[512];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        int bytesReceived = recvfrom(serverSocket, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            break;
        }
        recvBuf[bytesReceived] = '\0'; 

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

        std::cout << "클라이언트 " << clientIP << ":" << ntohs(clientAddr.sin_port) << " 로부터: " << recvBuf << std::endl;

        std::string message = "서버 응답: " + std::string(recvBuf);
        sendto(serverSocket, message.c_str(), message.length(), 0, (SOCKADDR*)&clientAddr, clientAddrLen);
    }

    closesocket(serverSocket);
    WSACleanup();
}

void RunUDPClient(const std::string& serverIP) {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); 
    

    iResult = inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    if (iResult != 1) {
        std::cerr << "InetPton failed for IP: " << serverIP << " error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::cout << "UDP 클라이언트 시작. 서버 " << serverIP << ":12345 에 연결 시도 중..." << std::endl;

    std::string message;
    char recvBuf[512];

    while (true) {
        std::cout << "보낼 메시지 입력 (exit 입력 시 종료): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
            break;
        }

        sockaddr_in fromAddr;
        int fromAddrLen = sizeof(fromAddr);
        int bytesReceived = recvfrom(clientSocket, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&fromAddr, &fromAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            break;
        }
        recvBuf[bytesReceived] = '\0';
        std::cout << "서버로부터 응답: " << recvBuf << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
}

int main()
{
    std::cout << "1. 서버로 시작 (Host)\n";
    std::cout << "2. 클라이언트로 시작 (Join Game)\n";
    std::cout << "선택: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(); // 버퍼 비우기

    if (choice == 1) {
        RunUDPServer();
    }
    else if (choice == 2) {
        std::string serverIP;
        std::cout << "서버 IP 주소 입력: ";
        std::getline(std::cin, serverIP);
        RunUDPClient(serverIP);
    }
    else {
        std::cout << "잘못된 선택입니다.\n";
    }

    return 0;
}