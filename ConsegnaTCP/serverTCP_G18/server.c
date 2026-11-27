#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define closesocket close
    #define SOCKET int
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

#define PROTOPORT 27015
#define QLEN 6
#define BUFFERSIZE 512

void ErrorHandler(const char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int send_message(SOCKET MySocket, char *inputString);
void readBuffer(SOCKET Csocket);

int main(int argc, char *argv[]) {

    int port;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    else {
        port = PROTOPORT;
    }

    if (port <= 0 || port > 65535) {
        printf("Bad port number\n");
        return 1;
    }

#ifdef _WIN32
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        ErrorHandler("Error at WSAStartup()\n");
        return 1;
    }
#endif

    SOCKET MySocket;

    MySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (MySocket == INVALID_SOCKET) {
        ErrorHandler("Socket creation failed.\n");
        ClearWinSock();
        return 1;
    }

    printf("Socket created successfully.\n");

    struct sockaddr_in sad;

    memset(&sad, 0, sizeof(sad));

    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    if (bind(MySocket, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR) {
        ErrorHandler("bind() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return 1;
    }

    printf("Socket bound to port %d.\n", port);

    if (listen(MySocket, QLEN) == SOCKET_ERROR) {
        ErrorHandler("listen() failed.\n");
        closesocket(MySocket);
        ClearWinSock();
        return 1;
    }

    printf("Server listening...\n");

    struct sockaddr_in cad;

    SOCKET clientSocket;
    socklen_t clientLen;

    printf("Waiting for a client to connect...\n");

    while (1) {

        clientLen = sizeof(cad);

        clientSocket = accept(
            MySocket,
            (struct sockaddr *)&cad,
            &clientLen
        );

        if (clientSocket == INVALID_SOCKET) {
            ErrorHandler("accept() failed.\n");
            continue;
        }

        printf("Connesso con il client il cui nome IP: %s\n",
               inet_ntoa(cad.sin_addr));

        if (send_message(clientSocket, "connessione avvenuta") == 0) {
            readBuffer(clientSocket);
        }

        closesocket(clientSocket);

        printf("Waiting for another client...\n");
    }

    closesocket(MySocket);
    ClearWinSock();

    return 0;
}

int send_message(SOCKET MySocket, char *inputString) {

    int stringLen = (int)strlen(inputString);

    int bytesSent = 0;

    while (bytesSent < stringLen) {

        int result = send(
            MySocket,
            inputString + bytesSent,
            stringLen - bytesSent,
            0
        );

        if (result == SOCKET_ERROR || result == 0) {

            ErrorHandler("send() failed.\n");
            return -1;
        }

        bytesSent += result;
    }

    printf("Message sent successfully: %s\n", inputString);
    return 0;
}

void readBuffer(SOCKET Csocket) {

    int bytesRcvd;
    char buf[BUFFERSIZE + 1];
    char reply[BUFFERSIZE];

    while (1) {

        bytesRcvd = recv(
            Csocket,
            buf,
            BUFFERSIZE,
            0
        );

        if (bytesRcvd == 0) {
            printf("Client disconnected.\n");
            break;
        }

        if (bytesRcvd == SOCKET_ERROR) {
            ErrorHandler("recv() failed.\n");
            break;
        }

        buf[bytesRcvd] = '\0';

        printf("Messaggio ricevuto: %s\n", buf);

        int n1, n2, n3;

        if (sscanf(buf, "%d %d %d", &n1, &n2, &n3) != 3) {
            ErrorHandler("Messaggio non valido.\n");
            break;
        }

        int res = n1 + n2 - n3;

        printf("la somma di n1+n2-n3=%d\n", res);

        if (res == 100) {
            send_message(Csocket, "QUIT");
            break;
        }

        snprintf(reply, sizeof(reply), "%d", res);

        if (send_message(Csocket, reply) != 0) {
            break;
        }
    }
}
