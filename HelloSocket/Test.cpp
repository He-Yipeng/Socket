#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

//#pragma comment(lib, "ws2_32.lib")

int main()
{
	//����Windows Socket2.x�Ļ���
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);


	//���Windows Socket2.x�Ļ���
	WSACleanup();
	return 0;
}