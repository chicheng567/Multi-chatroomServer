#include<iostream>
#include<WS2tcpip.h>
#include<string>
#include<sstream>
#include<string.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
int main()
{
	WSADATA wsaData;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult) {
		cout << "DLL initial error!!\n" << endl;
		return 1;
	}

	//create socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(1234);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	bind(listening, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	listen(listening, SOMAXCONN);
	//sets manipulation
	fd_set mainSet;
	FD_ZERO(&mainSet);
	FD_SET(listening, &mainSet);
	bool result = 1;
	while (result) {
		fd_set copy = mainSet;
		int fdCount = select(0, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < fdCount; ++i) {
			SOCKET currentSock = copy.fd_array[i];
			if (currentSock == listening) {	//if server Socket been selected
				SOCKET newClient = accept(listening, NULL, NULL);
				cout << "New client connected" << endl;
				//add new client Sock to sets
				FD_SET(newClient, &mainSet);
				string welcome = "Welcom to the chat room!!\n";
				send(newClient, welcome.c_str(), welcome.length() + 1, 0);
			}
			else {
				char buff[4096];
				int byteIn = recv(currentSock, buff, 4096, 0);
				cout << "Recieved 1 packet" << endl;
				if (byteIn <= 0) {
					closesocket(currentSock);
					FD_CLR(currentSock, &mainSet);
					cout << "One client disconnected" << endl;
				}
				else if (buff[0] == '\\') {
					if (!strcmp(buff, "\\quit")) {
						result = 0;
						break;
					}
				}
				else {
					for (int i = 0; i < mainSet.fd_count; ++i) {
						if (mainSet.fd_array[i] != listening && mainSet.fd_array[i] != currentSock) {

							ostringstream ss;
							ss << "Client# " << currentSock << ": " << buff;
							string output = ss.str();
							cout << output << endl;
							send(mainSet.fd_array[i], output.c_str(), output.length() + 1, 0);
						}
					}
				}
			}
		}
	}

	FD_CLR(listening, &mainSet);
	closesocket(listening);
	while (mainSet.fd_count > 0) {
		string bye = "Server clossing!!\n";
		send(mainSet.fd_array[0], bye.c_str(), bye.length() + 1, 0);
		closesocket(mainSet.fd_array[0]);
		FD_CLR(mainSet.fd_array[0], &mainSet);
	}

	WSACleanup();
	return 0;
}
