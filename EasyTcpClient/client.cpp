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
	//缓冲区
	char szRecv[1024] = {};
	//接收客户端数据
	int nLen = recv(cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen < 0)
	{
		printf("与服务器断开连接，任务结束\n", cSock);
		return -1;
	}
	//printf("收到命令： %d 数据长度： %d\n", header->cmd, header->dataLength);

	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginResult* loginRes = (LoginResult*)szRecv;
			printf("收到服务端消息, CMD_LOGIN_RESULT，数据长度：%d\n", loginRes->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LogoutResult* logoutRes = (LogoutResult*)szRecv;
			printf("收到服务端消息, CMD_LOGOUT_RESULT， 数据长度：%d\n", logoutRes->dataLength);
		}
			break;
		case CMD_NEW_USER_JOIN:
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* userJoin = (NewUserJoin*)szRecv;
			printf("收到服务端消息, CMD_NEW_USER_JOIN， 数据长度：%d\n", userJoin->dataLength);
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
		printf("退出\n");
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
		printf("不支持命令。\n");
	}

}

int main()
{
	//启动Windows Socket2.x的环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1、建立一个socket 套接字 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		printf("错误，建立Socket失败\n");
	}
	else {
		printf("建立Socket成功\n");
	}
	//2、connect 连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误，连接失败\n");
	}
	else {
		printf("连接成功\n");
	}

	//启动线程
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
			printf("select任务结束\n");
			break;
		}
		if (FD_ISSET(sock, &fdRead))
		{
			FD_CLR(sock, &fdRead);

			if (-1 == processor(sock))
			{
				printf("select任务结束2");
				break;
			}
		}
		//printf("空闲时间处理其他业务.....\n");
		//q
		//Sleep(1000);
	}
	
	//4、 关闭套接字
	closesocket(sock);
	//清除Windows Socket2.x的环境
	WSACleanup();
	getchar();
	return 0;
}