#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>

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

struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

struct Logout : public DataHeader
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


int processor(SOCKET cSock)
{
	//������
	char szRecv[1024] = {};
	//���տͻ�������
	int nLen = recv(cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen < 0)
	{
		printf("��������Ͽ����ӣ��������\n", cSock);
		return -1;
	}
	//printf("�յ���� %d ���ݳ��ȣ� %d\n", header->cmd, header->dataLength);

	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginResult* loginRes = (LoginResult*)szRecv;
			printf("�յ��������Ϣ, CMD_LOGIN_RESULT�����ݳ��ȣ�%d\n", loginRes->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LogoutResult* logoutRes = (LogoutResult*)szRecv;
			printf("�յ��������Ϣ, CMD_LOGOUT_RESULT�� ���ݳ��ȣ�%d\n", logoutRes->dataLength);
		}
			break;
		case CMD_NEW_USER_JOIN:
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* userJoin = (NewUserJoin*)szRecv;
			printf("�յ��������Ϣ, CMD_NEW_USER_JOIN�� ���ݳ��ȣ�%d\n", userJoin->dataLength);
			break;
		}
	return 0;
}

void cmdThread(SOCKET sock)
{
	char cmdBuf[256] = {};
	scanf("%s", cmdBuf);
	if (0 == strcmp(cmdBuf, "exit"))
	{
		printf("�˳�\n");
		return;
	}
	else if (0 == strcmp(cmdBuf, "login"))
	{
		Login login;
		strcpy(login.userName, "hyp");
		strcpy(login.passWord, "123456");
		send(sock, (const char*)&login, sizeof(Login), 0);
	}
	else if (0 == strcmp(cmdBuf, "logout"))
	{
		Logout login;
		strcpy(login.userName, "hyp");
		send(sock, (const char*)&login, sizeof(Logout), 0);
	}
	else
	{
		printf("��֧�����\n");
	}

}

int main()
{
	//����Windows Socket2.x�Ļ���
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1������һ��socket �׽��� 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		printf("���󣬽���Socketʧ��\n");
	}
	else {
		printf("����Socket�ɹ�\n");
	}
	//2��connect ���ӷ�����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("��������ʧ��\n");
	}
	else {
		printf("���ӳɹ�\n");
	}

	//�����߳�
	std::thread t1(cmdThread, sock);
	while (true)
	{
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);
		FD_SET(sock, &fdRead);
		timeval t = { 1, 0 };
		int res = select(sock + 1, &fdRead, NULL, NULL, &t);
		if (res < 0)
		{
			printf("select�������\n");
			break;
		}
		if (FD_ISSET(sock, &fdRead))
		{
			FD_CLR(sock, &fdRead);

			if (-1 == processor(sock))
			{
				printf("select�������2");
				break;
			}
		}
		//printf("����ʱ�䴦������ҵ��.....\n");
		//q
		//Sleep(1000);
	}
	
	//4�� �ر��׽���
	closesocket(sock);
	//���Windows Socket2.x�Ļ���
	WSACleanup();
	getchar();
	return 0;
}