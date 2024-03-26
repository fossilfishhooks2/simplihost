#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

//#define DEFAULT_HTML_FILE "index.html"
uint16_t DEFAULT_PORT = 80;
#define CONFIG_FILE "simplihost.cfg"
//#define BUFFER_SIZE 16000
//#define MAX_FILE_SIZE 1048576
unsigned long long MAX_PATH_SIZE = 16384; // Choose an appropriate size for paths
unsigned long long MAX_HEADER_SIZE = 16384; // Choose an appropriate size for headers
int transfer = 2000;
//#define MAX_CONTENT_SIZE 1048576 // Adjust this size as needed

void removeTrailingNewline(char* str) {
    int length = strlen(str);
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0'; // Replace '\n' with '\0'
    }
}






void exitHandler() {
    printf("Cleaning up...\n");
    WSACleanup();
    Sleep(45);
    printf("Done.\n");

}


BOOL CtrlHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_C_EVENT) {
        //printf("Ctrl+C received. Cleaning up...\n");
        exitHandler();
    }
    return FALSE;
}

void handleClient(SOCKET clientSocket) {
    //printf("Handleclient was called\n");
    //Sleep(10);
    FILE* mapFile = fopen("file_map.txt", "r");
    if (mapFile == NULL) {
        printf("Failed to open file_map.txt\n");
        closesocket(clientSocket);
        return;
    }
    
    char* requestedPath = (char*)malloc(MAX_PATH_SIZE * sizeof(char));
    // Check for malloc failure...
    //printf("Allocated memory for file\n");
    if (recv(clientSocket, requestedPath, MAX_PATH_SIZE, 0) == SOCKET_ERROR) {
        printf("Failed to receive data from client\n");
        free(requestedPath);
        fclose(mapFile);
        closesocket(clientSocket);
        return;
    }
    //printf("Found requested path\n");

    //char link[MAX_FILE_SIZE], header[MAX_FILE_SIZE], filename[MAX_FILE_SIZE];
    char* link = (char*)malloc(MAX_PATH_SIZE * sizeof(char)*4);
    char* header = (char*)malloc(MAX_HEADER_SIZE * sizeof(char)*4);
    char* filename = (char*)malloc(MAX_PATH_SIZE * sizeof(char)*4);
    char* bat_filename = (char*)malloc(MAX_PATH_SIZE * sizeof(char) * 4);
    int found = 0;
    char req[256];
    while (fgets(link, sizeof(link)*8, mapFile) != NULL &&
        fgets(header, 511*8, mapFile) != NULL &&
        fgets(filename, 511*8, mapFile) != NULL &&
        fgets(bat_filename, 511 * 8, mapFile) != NULL) {
        // Removing the trailing newline characters if present
        //link[strcspn(link, (char*)0x0a)] = '\0';
        //header[strcspn(header, (char*)0x0a)] = '\0';
        //filename[strcspn(filename, (char*)0x0a)] = '\0';
        removeTrailingNewline(link);
        removeTrailingNewline(header);
        removeTrailingNewline(filename);
        removeTrailingNewline(bat_filename);
        
        
        sscanf(requestedPath, "GET %s", &req);
        printf("Requested path '%s' and link is '%s'\n", req, link);
        //printf("Lookup response: header is '%s' and filename is '%s'\n", header, filename);
        if (strcmp(req, link) == 0) {
            found = 1;
            break;
        }
    }

    fclose(mapFile);

    if (!found) {
        //404
        printf("Requested path not found in file_map.txt. Sending 404...\n");
        //FILE* requestedlink = fopen("404.list", "w");
        //fwrite(req, strlen(req), 1, requestedlink);
        //fclose(requestedlink);
        std::string temp = "404.bat ";
        temp += req;
        system(temp.c_str());
        //send(clientSocket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html", 48, 0);
        FILE* requestedFile4 = fopen("simplihost_defaults\\404.html", "r");
        if (requestedFile4 == NULL) {
            printf("Failed to open 404.html\n");
            closesocket(clientSocket);
            return;
        }

        fseek(requestedFile4, 0, SEEK_END);
        long fileSizex = ftell(requestedFile4);
        rewind(requestedFile4);
        char* fileContentx = (char*)malloc(fileSizex+2);
        if (fileContentx == NULL) {
            printf("Memory allocation failed\n");
            fclose(requestedFile4);
            return;
        }

        size_t bytesReadx = fread(fileContentx, 1, fileSizex, requestedFile4);
        
        if (bytesReadx == 0) {
            printf("Failed to read file contents in 404.html\n");
            
            
            free(fileContentx);
            fclose(requestedFile4);
            return;
        }

        /*
        * char contentLengthHeader[100];
    sprintf(contentLengthHeader, "Content-Length: %ld\r\n\r\n", fileSize);
    //send(clientSocket, "HTTP/1.1 200 OK\r\n", 18, 0);
    send(clientSocket, responseHeader, strlen(responseHeader), 0);
    send(clientSocket, contentLengthHeader, strlen(contentLengthHeader), 0);
    */
        char contentLengthHeaderx[1000];
        sprintf(contentLengthHeaderx, "Content-Length: %ld\r\n\r\n", fileSizex);
        send(clientSocket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n", strlen("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"), 0);
        Sleep(1);
        send(clientSocket, contentLengthHeaderx, strlen(contentLengthHeaderx), 0);
        //printf("HTTP HEADER:\n'%s'\n'%s'\n",)
        //printf("404: '%s'\n", fileContentx);
        send(clientSocket, fileContentx, fileSizex, 0);
        Sleep(transfer);
        closesocket(clientSocket);
        free(fileContentx);
        return;
    }
    printf("Page requested exists\nExecuting linked script...\n");
    system(bat_filename);

    FILE* requestedFile = fopen(filename, "rb");
    if (requestedFile == NULL) {
        printf("Failed to open requested file\n");
        return;
    }

    fseek(requestedFile, 0, SEEK_END);
    long fileSize = ftell(requestedFile);
    rewind(requestedFile);

    const char* responseHeader = header;
    //HTTP/1.1 200 OK\r\nContent-Type: text/html
    char contentLengthHeader[100];
    sprintf(contentLengthHeader, "Content-Length: %ld\r\n\r\n", fileSize);
    //send(clientSocket, "HTTP/1.1 200 OK\r\n", 18, 0);
    send(clientSocket, responseHeader, strlen(responseHeader), 0);
    send(clientSocket, contentLengthHeader, strlen(contentLengthHeader), 0);

    char* fileContent = (char*)malloc(fileSize);
    if (fileContent == NULL) {
        printf("Memory allocation failed\n");
        fclose(requestedFile);
        return;
    }

    size_t bytesRead = fread(fileContent, 1, fileSize, requestedFile);
    if (bytesRead < fileSize) {
        printf("Failed to read file contents\n");
        free(fileContent);
        fclose(requestedFile);
        return;
    }

    send(clientSocket, fileContent, fileSize, 0);
    printf("Sent file\n");
    Sleep(transfer);
    free(link);
    free(header);
    free(filename);
    free(requestedPath);
    free(fileContent);
    fclose(requestedFile);
    
    closesocket(clientSocket);
}

int configure() {
    printf("Configuring from ");
    printf(CONFIG_FILE);
    printf("...\n");
    FILE* configFile = fopen(CONFIG_FILE, "r");

    // Check if the file is open
    if (!configFile) {
        printf("WARNING: Could not open the configuration file.\n");
        return 1;
    }

    char line[256]; // Buffer to read lines from the file
    //DEFAULT_PORT = 80; // Initialize DEFAULT_PORT with a default value

    // Read the file line by line
    while (fgets(line, sizeof(line), configFile)) {
        // Check if the line starts with "PORT"
        if (strncmp(line, "PORT", 4) == 0) {
            // Extract the number after "PORT"
            int port;
            if (sscanf(line, "PORT %d", &port) == 1) {
                DEFAULT_PORT = port;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "MPS", 3) == 0) {
            // Extract the number after "PORT"
            unsigned long long mps;
            if (sscanf(line, "MPS %d", &mps) == 1) {
                MAX_PATH_SIZE = mps;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "MHS", 3) == 0) {
            // Extract the number after "PORT"
            unsigned long long mhs;
            if (sscanf(line, "MHS %d", &mhs) == 1) {
                MAX_HEADER_SIZE = mhs;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "PTT", 3) == 0) {
            // Extract the number after "PORT"
            unsigned long long ptt;
            if (sscanf(line, "PTT %d", &ptt) == 1) {
                transfer = ptt;
                //break; // Stop reading the file after finding the PORT line
            }
        }
    }

   

    // Close the file
    fclose(configFile);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "help") == 0) {
            printf("Simplihost 4.2.0 starting...\n");
            printf("Simplihost is a simple and lightweight web server. Like all websites, it hosts on port 80 by default. The default pessimistic transfer time is 2000 ms.\n");
            return 0;
        }
        
    }
    printf("Simplihost 4.2.0 starting...\n");
    Sleep(760);
    configure();
    printf("Attempting to host everything listed in file_map.txt, using pessimistic transfer time %d ms\n", transfer);
    printf("Use 'simplihost.exe help' for more information\n");
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Startup failed\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(DEFAULT_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Binding failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server running on port %d...\n", DEFAULT_PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        printf("Client connected\n");

        handleClient(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

