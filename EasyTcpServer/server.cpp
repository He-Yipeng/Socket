#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#include <vector>
#pragma comment(lib, "ws2_32.lib")
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short cmd;
	short dataLength;
};

struct Login: public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult: public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout: public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT; 
		result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

std::vector<SOCKET> g_clients;

int processor(SOCKET cSock)
{
	//������
	char szRecv[1024] = {};
	//���տͻ�������
	int nLen = recv(cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen < 0)
	{
		printf("�ͻ���<Socket=%d>���˳��� �������\n", cSock);
		return -1;
	}
	//printf("�յ���� %d ���ݳ��ȣ� %d\n", header->cmd, header->dataLength);

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("�յ��ͻ���<Socket=%d>��� CMD_LOGIN, ���ݳ��ȣ�%d��userName=%s, passWord=%s \n",
			cSock, login->dataLength, login->userName, login->passWord);
		LoginResult res = {};
		send(cSock, (char*)&res, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)szRecv;
		printf("�յ��ͻ���<Socket=%d>��� CMD_LOGOUT, ���ݳ��ȣ�%d��userName=%s \n",
			cSock, logout->dataLength, logout->userName);
		LogoutResult res = {};
		send(cSock, (char*)&res, sizeof(LogoutResult), 0);
	}
	break;
	default:
		DataHeader header = { 0,CMD_ERROR };
		recv(cSock, (char*)&header, sizeof(DataHeader), 0);
		break;
	}
	return 0;
}

int main()
{
	//����Windows Socket2.x�Ļ���
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1������һ��socket �׽��� 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2��bind �����ڽ��տͻ��˵�����˿�
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("���󣬰����ڽ��տͻ������ӵ�����˿�ʧ��\n");
	}
	else {
		printf("������˿ڳɹ�\n");
	}
	//3��listen ��������˿�
	if (SOCKET_ERROR == listen(sock, 5))
	{
		printf("���󣬼�������˿�ʧ��\n");
	}
	else {
		printf("��������˿ڳɹ�\n");
	}


	char recvBuf[128] = {};
	while (true)
	{
		// ������ socket
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(sock, &fdRead);
		FD_SET(sock, &fdWrite);
		FD_SET(sock, &fdExp);
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		//nfds ��һ������ֵ�� ֵfd_set����������������(socket)
		//���������ļ������������ֵ+1�� windows�������������д0
		timeval t = { 1, 0 };
		int res = select(sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (res < 0)
		{
			printf("select���������\n");
			break;
		}

		if (FD_ISSET(sock, &fdRead))
		{
			FD_CLR(sock, &fdRead);
			//4��accept �ȴ��ͻ�������
			sockaddr_in	clientAddr = {};
			int nAdrLen = sizeof(sockaddr_in);
			SOCKET cSock = INVALID_SOCKET;
			cSock = accept(sock, (sockaddr*)&clientAddr, &nAdrLen);
			if (INVALID_SOCKET == cSock)
			{
				printf("���󣬽�����Ч�Ŀͻ���socket...\n");
			}
			for (size_t i = 0; i < g_clients.size(); i++)
			{
				NewUserJoin userJoin;
				send(g_clients[i], (char*)&userJoin, sizeof(NewUserJoin), 0);
			}
			g_clients.push_back(cSock);
			printf("�¿ͻ��˼���: IP = %s\n", inet_ntoa(clientAddr.sin_addr));
		}		
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}		
		printf("����ʱ�䴦������ҵ��.....\n");
	}	
	for (size_t i = 0; i < g_clients.size(); i++)
	{
		closesocket(g_clients[i]);
	}
	//6�� �ر��׽���
	closesocket(sock);
	//���Windows Socket2.x�Ļ���
	WSACleanup();
	printf("���˳����������.\n");
	getchar();
	return 0;
}