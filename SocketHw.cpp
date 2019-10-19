#include <stdio.h>
#include <winsock.h>
#include <windows.h>
#include <thread>
using namespace std;

void Server(); // Build server
void Client(); // Connect to server

void Sender(SOCKET); // Both need a sender and receiver
void Receiver(SOCKET);

#define PRINTERROR(s)   \
		fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())

int main() {
	int n;
	thread local;

	// Choose server or client
	printf("[0] Server\n[1] Client\n > ");
	scanf("%d", &n);

	if (n) local = thread(Client);
	else local = thread(Server);

	local.join();

	return 0;
}

void Client() {
	char szServer[256]; int nPort;
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	int nRet = WSAStartup(wVersionRequested, &wsaData);
	printf("Connect Server: ");
	scanf("%s", szServer);
	printf("Connect Port: ");
	scanf("%d", &nPort);

	LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);
	if (lpHostEntry == NULL) {
		PRINTERROR("gethostbyname()");
		return;
	}

	SOCKET	theSocket;
	theSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (theSocket == INVALID_SOCKET) {
		PRINTERROR("socket()");
		return;
	}

	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR) * lpHostEntry->h_addr_list);
	saServer.sin_port = htons(nPort);

	nRet = connect(theSocket, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("socket()");
		closesocket(theSocket);
		return;
	}

	thread S(Sender, theSocket), R(Receiver, theSocket);
	S.join(); R.join();
	closesocket(theSocket);
	WSACleanup();
	return;
}

void Sender(SOCKET theSocket) {
	do {
		char szBuf[256];
		getchar();
		scanf("%s", szBuf);
		printf("S: %s\n", szBuf);
		if (send(theSocket, szBuf, strlen(szBuf), 0) == SOCKET_ERROR)
			break;
	} while(1);
}

void Receiver(SOCKET remoteSocket) {
	do {
		char szBuf[256] = {0};
		if (recv(remoteSocket, szBuf, sizeof(szBuf), 0) == SOCKET_ERROR)
			break;
		printf("R: %s\n", szBuf);
	} while (1);
}

void Server() {
	int nPort;
	printf("Port: ");
	scanf("%d", &nPort);
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	int nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested) {
		fprintf(stderr, "\n Wrong version\n");
		return;
	}

	SOCKET	listenSocket;

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		PRINTERROR("socket()");
		return;
	}

	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	saServer.sin_port = htons(nPort);

	nRet = bind(listenSocket, (LPSOCKADDR)&saServer, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("bind()");
		closesocket(listenSocket);
		return;
	}

	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("gethostname()");
		closesocket(listenSocket);
		return;
	}

	nRet = listen(listenSocket, SOMAXCONN);
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("listen()");
		closesocket(listenSocket);
		return;
	}

	SOCKET	remoteSocket;

	remoteSocket = accept(listenSocket, NULL, NULL);
	if (remoteSocket == INVALID_SOCKET) {
		PRINTERROR("accept()");
		closesocket(listenSocket);
		return;
	}

	thread S(Sender, remoteSocket), R(Receiver, remoteSocket);
	S.join(); R.join();

	closesocket(remoteSocket);
	closesocket(listenSocket);
	WSACleanup();

	return;
}
