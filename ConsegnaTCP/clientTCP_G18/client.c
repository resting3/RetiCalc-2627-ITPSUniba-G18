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

#define BUFFERSIZE 512

// Function declaration
int readBuffer(SOCKET Csocket, char *out);
int send_message(SOCKET MySocket, char *inputString);

void ErrorHandler(const char *errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(void) {

    char ip[64];
    int port;

    printf("Benvenuto!\nInserisci Ip: ");
    if (scanf("%63s", ip) != 1) return -1;

    printf("Inserisci Porta: ");
    if (scanf("%d", &port) != 1) return -1;

#ifdef _WIN32
    // Inizializzazione Winsock
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0) {
        printf("Error at WSAStartup\n");
        return -1;
    }
#endif

    // CREAZIONE DELLA SOCKET
    SOCKET Csocket;

    Csocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (Csocket == INVALID_SOCKET) {
        ErrorHandler("Socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    printf("Socket created successfully.\n");

    // COSTRUZIONE DELL'INDIRIZZO DEL SERVER
    struct sockaddr_in sad;

    memset(&sad, 0, sizeof(sad));

    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(ip);
    sad.sin_port = htons(port);

    // CONNESSIONE AL SERVER
    if (connect(Csocket, (struct sockaddr *)&sad, sizeof(sad)) == SOCKET_ERROR) {

        ErrorHandler("Failed to connect.\n");

        closesocket(Csocket);
        ClearWinSock();

        return -1;
    }

    printf("Connected to server successfully.\n");

    char answer[BUFFERSIZE + 1];

    // RICEZIONE DI "connessione avvenuta"
    if (readBuffer(Csocket, answer) <= 0) {
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    printf("Messaggio ricevuto: %s\n\n", answer);

    while (1) {
        int n1, n2, n3;
        char buffer[100];

        printf("Inserire primo numero: ");
        if (scanf("%d", &n1) != 1) break;

        printf("Inserire secondo numero: ");
        if (scanf("%d", &n2) != 1) break;

        printf("Inserire terzo numero: ");
        if (scanf("%d", &n3) != 1) break;

        // Unire i tre numeri in un'unica stringa
        snprintf(buffer, sizeof(buffer), "%d %d %d", n1, n2, n3);

        if (send_message(Csocket, buffer) != 0) break;

        // RICEZIONE DELLA RISPOSTA DEL SERVER
        if (readBuffer(Csocket, answer) <= 0) break;

        printf("Messaggio ricevuto: %s\n", answer);

        if (strcmp(answer, "QUIT") == 0) {
            break;   // chiude la connessione e termina
        }

        printf("Risultato: %s\n\n", answer);
    }

    // CHIUSURA DELLA CONNESSIONE
    closesocket(Csocket);

    ClearWinSock();

    printf("\n");

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}


// Riceve un messaggio e lo mette in out. Ritorna i byte ricevuti (0 = server chiuso, <0 = errore)
int readBuffer(SOCKET Csocket, char *out) {

    int bytesRcvd;

    bytesRcvd = recv(Csocket, out, BUFFERSIZE, 0);

    if (bytesRcvd == 0) {
        printf("Server disconnected.\n");
        return 0;
    }

    if (bytesRcvd == SOCKET_ERROR) {
        ErrorHandler("recv() failed.\n");
        return -1;
    }

    out[bytesRcvd] = '\0';

    return bytesRcvd;
}


int send_message(SOCKET MySocket, char *inputString) {

    int stringLen = (int)strlen(inputString);

    // INVIARE DATI AL SERVER
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