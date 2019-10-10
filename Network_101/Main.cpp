#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <vector>

bool select_recv(SOCKET sock, int interval_us = 1)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = interval_us;
	return(select(sock + 1, &fds, 0, 0, &tv) == 1);
}

bool select_accept(SOCKET socket, int interval_us = 1)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = interval_us;
	return (select(socket + 1, &fds, 0, 0, &tv) == 1);
}

void SendMessageToEveryOne(std::vector<SOCKET> clients, std::string message)
{
	for (int i = 0; i < clients.size(); i++)
	{
		send(clients[i], &message[0], message.size(), NULL);
	}
}

int main()
{
	WORD word = MAKEWORD(2, 2);
	WSADATA wsaData;

	if (WSAStartup(word, &wsaData) != 0)
	{
		std::cerr << "Failed to get socket version\n";
		return -1;
	}
	std::cout << "Startup socket version\n";

	SOCKET m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_listenSocket == INVALID_SOCKET)
	{
		std::cerr << "Failed to create socket listener\n";
		return -2;
	}
	std::cout << "Create socket listener\n";

	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	hint.sin_port = htons(45555);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(m_listenSocket, (sockaddr*)&hint, sizeof hint) != 0)
	{
		std::cerr << "Failed to bind socket to address\n";
		return -2;
	}
	std::cout << "Bind socket listener to address\n";

	if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Failed to listen\n";
		return -3;
	}
	std::cout << "Start listening\n";

	std::vector<SOCKET> m_clients;

	while (true)
	{
		if (select_accept(m_listenSocket))
		{
			SOCKET clientSocket = accept(m_listenSocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET)
			{
				std::cerr << "Failed to create client socket\n";
				return -4;
			}
			std::cout << "New client connected\n";
			SendMessageToEveryOne(m_clients, "New client connected\n");
			m_clients.push_back(clientSocket);
		}

		std::vector<int> indexClientDisconnected;

		for (int i = 0; i < m_clients.size(); i++)
		{
			if (select_recv(m_clients[i]))
			{
				const auto bufferLength = 512;
				std::vector<char> buffer;
				buffer.resize(bufferLength);

				const auto sizeReception = recv(m_clients[i], &buffer[0], bufferLength, NULL);

				if (sizeReception == 0)
				{
					std::cout << "Client disconnected\n";
					SendMessageToEveryOne(m_clients, "Client disconnected\n");
					indexClientDisconnected.push_back(i);
					continue;
				}

				for (int j = 0; j < m_clients.size(); j++)
				{
					if (j != i)
					{
						send(m_clients[j], &buffer[0], bufferLength, NULL);
					}
				}

				for (auto k = 0; k < sizeReception; k++)
				{
					std::cout << static_cast<char>(buffer[k]);
				}
			}
		}

		if (!indexClientDisconnected.empty())
		{
			std::vector<SOCKET> newClients;

			for (int i = 0; i < m_clients.size(); i++)
			{
				bool isDisconnected = false;

				for (int j = 0; j < indexClientDisconnected.size(); j++)
				{
					if (i == j)
					{
						isDisconnected = true;
						break;
					}
				}

				if (!isDisconnected)
				{
					newClients.push_back(m_clients[i]);
				}
			}

			m_clients.clear();

			for (int i = 0; i < newClients.size(); i++)
			{
				m_clients.push_back(newClients[i]);
			}
		}
	}

	system("pause");

	closesocket(m_listenSocket);
	WSACleanup();

	return 0;
}


