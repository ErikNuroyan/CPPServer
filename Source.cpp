#pragma comment (lib,"ws2_32.lib")
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>


int main() {

	WSAData data;
	WORD version = MAKEWORD(2, 2);
	//Testing if the winsock initiallized properly
	if (!WSAStartup(version, &data)) {  //Returns 0 on success
		std::cout << "WinSock Initiallized Properly"<<std::endl;
	}
	else {
		std::cout << "Something went wrong" << std::endl;
		return -1;
	}

	//Creating the Socket and setting up
	SOCKET sock = socket(AF_INET, SOCK_STREAM,0);

	if (sock == INVALID_SOCKET) {
		std::cout << "Something went wrong initiallizing the socket!" << std::endl;
		return -1;
	}

	sockaddr_in setup;
	setup.sin_family = AF_INET;
	setup.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &setup.sin_addr);

	if (bind(sock, (sockaddr*)&setup, sizeof(setup)) == SOCKET_ERROR) {
		std::cout << "Socket did not bind to the given settings" << std::endl;
		return -1;
	}

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Doesn't listen to the server" << std::endl;
		return -1;
	}
	
	sockaddr_in client;
	int client_size = sizeof(client);
	SOCKET client_socket = accept(sock, (sockaddr*)&client, &client_size);

	if (client_socket == SOCKET_ERROR) {
		std::cout << "Client couldn't connect" << std::endl;
		return -1;
	}
	else {
		char msg[4096]; //We will receive message here
		ZeroMemory(msg, 4096);
		int bytes_received = recv(client_socket, msg, 4096, 0);
		if (bytes_received <= 0) {
			closesocket(client_socket);
		}
		else {
			//Parser for html request
			std::vector<std::string> parsed_req;
			std::string temp = "";
			std::cout << msg<<std::endl;
			for (int i = 0; i < bytes_received; i++) {
				char c = msg[i];
				if ((c == ' ' || c == '\r' || c =='\n')){
					if (temp.size() == 0)continue;
					parsed_req.push_back(temp);
					temp = "";
					continue;
				}
				temp += c;
			}
			std::cout << parsed_req[0] << std::endl;

			if (parsed_req[1] == "/") {
				std::ifstream ifs("index.html");
				if (!ifs.is_open()) {
					std::cout << "Error finding html" << std::endl;
				}
				else {
					//Getting the content of html file
					std::string html((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
					ifs.close();

					//Setting up the packet to be sent
					std::ostringstream oss;
					oss << "HTTP/1.1 " << 200 << " OK\r\n";  //200 to be changed if not OK
					oss << "Cache-Control: no-cache, private\r\n";
					oss << "Content-Type: text/html\r\n";
					oss << "Content-Length: " << html.size()<< "\r\n";
					oss << "\r\n";
					oss << html;
					html = oss.str();

					//Sending the packet
					send(client_socket, html.c_str(), html.size() + 1, 0);
					//Do this to get the new request
					client_socket = accept(sock, (sockaddr*)&client, &client_size);
					ZeroMemory(msg, 4096);
					bytes_received=recv(client_socket, msg, 4096, 0);
					std::cout << "Bytes Received: "<<bytes_received;
					std::cout << msg << std::endl;


				}
			}
		}
	}


	system("pause");
	return 0;
}