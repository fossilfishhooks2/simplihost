#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

//#define DEFAULT_HTML_FILE "index.html"
u_short DEFAULT_PORT=80;
#define CONFIG_FILE "simplihost.cfg"
//#define BUFFER_SIZE 16000
//#define MAX_FILE_SIZE 1048576
int MAX_PATH_SIZE = 16384; // Choose an appropriate size for paths
int MAX_HEADER_SIZE = 16384; // Choose an appropriate size for headers
int transfer = 2000;
//#define MAX_CONTENT_SIZE 1048576 // Adjust this size as needed


//bool PORT_BUSY = false;
std::atomic<bool> PORT_BUSY = false; //allow simultaneous r/w operations from multiple threads

void removeTrailingNewline(char* str) {
    int length = strlen(str);
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0'; // Replace '\n' with '\0'
    }
}






void exitHandler(int signal) {
    if (signal == SIGINT) {
        printf("[EXIT] Ctrl-C received. Exiting...\n");
    }
    else if (signal == SIGTERM) {
        printf("[EXIT] Terminate signal received. Exiting...\n");
    }
    else {
        printf("[EXIT] Signal %d received. Exiting...\n",signal);
    }
    printf("[EXIT] Cleaning up...\n");
    WSACleanup();
    
    printf("[EXIT] Done.\n");

}


void handleClient(SOCKET clientSocket) {
    try {
   // auto startTime = std::chrono::high_resolution_clock::now();
    //printf("[INFO] HandleClient was called\n");
    //Sleep(10);
    FILE* mapFile = fopen("file_map.txt", "r");
    if (mapFile == NULL) {
        printf("[RESPONDER][ERROR] Failed to open file_map.txt\n");
        closesocket(clientSocket);
        PORT_BUSY = false;
        return;
    }
    
    char* requestedPath = (char*)malloc(MAX_PATH_SIZE * sizeof(char));
    // Check for malloc failure...
    //printf("Allocated memory for file\n");
    //int32_t junk = 246032954;
    //auto startTime2 = std::chrono::high_resolution_clock::now();
    while (PORT_BUSY == true) {
        //idle to wait for another send to finish.
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b[RESPONDER][INFO] Waiting for port...");

    }
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b[RESPONDER][INFO] Port available, continuing...\n");
   // auto endTime2 = std::chrono::high_resolution_clock::now();
    PORT_BUSY = true; //in use
    if (recv(clientSocket, requestedPath, MAX_PATH_SIZE, 0) == SOCKET_ERROR) {
        printf("[RESPONDER][INFO] Failed to receive data from client\n");
        free(requestedPath);
        fclose(mapFile);
        closesocket(clientSocket);
        PORT_BUSY = false;
        return;
    }
    //printf("Found requested path\n");

    //char link[MAX_FILE_SIZE], header[MAX_FILE_SIZE], filename[MAX_FILE_SIZE];
    char* link = (char*)malloc(MAX_PATH_SIZE * sizeof(char)*4); //URL length
    char* header = (char*)malloc(MAX_HEADER_SIZE * sizeof(char)*4); //HTTP headrr
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
        printf("[RESPONDER][INFO] Requested path '%s' and link is '%s'\n", req, link);
        //printf("Lookup response: header is '%s' and filename is '%s'\n", header, filename);
        if (strcmp(req, link) == 0) {
            found = 1;
            break;
        }
    }

    fclose(mapFile);

    if (!found) {
        //404
        printf("[RESPONDER][INFO] Requested path not found in file_map.txt. Sending 404...\n");
        //FILE* requestedlink = fopen("404.list", "w");
        //fwrite(req, strlen(req), 1, requestedlink);
        //fclose(requestedlink);
        std::string temp = "404.bat ";
        temp += req;
        system(temp.c_str());
        //send(clientSocket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html", 48, 0);
        FILE* requestedFile4 = fopen("simplihost_defaults\\404.html", "r");
        if (requestedFile4 == NULL) {
            printf("[RESPONDER][ERROR]Failed to open 404.html\n");
            closesocket(clientSocket);
            PORT_BUSY = false;
            return;
        }

        fseek(requestedFile4, 0, SEEK_END);
        long fileSizex = ftell(requestedFile4);
        rewind(requestedFile4);
        char* fileContentx = (char*)malloc(fileSizex+2);
        if (fileContentx == NULL) {
            printf("[RESPONDER][ERROR] Memory allocation failed\n");
            fclose(requestedFile4);
            PORT_BUSY = false;
            return;
        }

        size_t bytesReadx = fread(fileContentx, 1, fileSizex, requestedFile4);
        
        if (bytesReadx == 0) {
            printf("[RESPONDER][ERROR] Failed to read file contents in 404.html\n");
            
            
            free(fileContentx);
            fclose(requestedFile4);
            PORT_BUSY = false;
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
        PORT_BUSY = false;
        return;
    }
    printf("[RESPONDER][INFO] Page requested exists\n[RESPONDER][INFO] Executing linked script...\n");
    system(bat_filename);

    FILE* requestedFile = fopen(filename, "rb");
    if (requestedFile == NULL) {
        printf("[RESPONDER][ERROR] Failed to open requested file\n");
        PORT_BUSY = false;
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
        printf("[RESPONDER][ERROR] Memory allocation failed\n");
        fclose(requestedFile);
        PORT_BUSY = false;
        return;
    }

    size_t bytesRead = fread(fileContent, 1, fileSize, requestedFile);
    if (bytesRead < fileSize) {
        printf("[RESPONDER][ERROR] Failed to read file contents\n");
        free(fileContent);
        fclose(requestedFile);
        PORT_BUSY = false;
        return;
    }

    send(clientSocket, fileContent, fileSize, 0);
    printf("[RESPONDER][INFO] Sent file\n");
    Sleep(transfer);
    free(link);
    free(header);
    free(filename);
    free(requestedPath);
    free(fileContent);
    free(bat_filename);
    fclose(requestedFile);
    
    closesocket(clientSocket);
    
    PORT_BUSY = false;

   // auto endTime = std::chrono::high_resolution_clock::now();

    // Calculate the duration
   // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
   // auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime2 - startTime2);

    // Output the duration
    //std::cout << "Execution time: " << duration.count() << " microseconds" << std::endl;
    //printf("[INFO] HandleClient took %lld microseconds, of which %lld were spent waiting for the port to be freed\n", duration.count(), duration2.count());
    }
    catch (const std::exception& e) {
        // Handle the error (print error message, log it, etc.)
        printf("[RESPONDER][ERROR] %s\n", e.what());
    }
}

int configure() {
    printf("[CONF] Configuring from ");
    printf(CONFIG_FILE);
    printf("...\n");
    FILE* configFile = fopen(CONFIG_FILE, "r");

    // Check if the file is open
    if (!configFile) {
        printf("[CONF] WARNING: Could not open the configuration file. Falling back to defaults.\n");
        return 1;
    }

    char line[256]; // Buffer to read lines from the file
    //DEFAULT_PORT = 80; // Initialize DEFAULT_PORT with a default value

    // Read the file line by line
    while (fgets(line, sizeof(line), configFile)) {
        
        if (strncmp(line, "MPS", 3) == 0) {
            // Extract the number after "PORT"
            int mps;
            if (sscanf(line, "MPS %d", &mps) == 1) {
                MAX_PATH_SIZE = mps;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "MHS", 3) == 0) {
            // Extract the number after "PORT"
            int mhs;
            if (sscanf(line, "MHS %d", &mhs) == 1) {
                MAX_HEADER_SIZE = mhs;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "PTT", 3) == 0) {
            // Extract the number after "PORT"
            int ptt;
            if (sscanf(line, "PTT %d", &ptt) == 1) {
                transfer = ptt;
                //break; // Stop reading the file after finding the PORT line
            }
        }
        if (strncmp(line, "PRT", 3) == 0) {
            // Extract the number after "PORT"
            u_short prt;
            if (sscanf(line, "PRT %d", &prt) == 1) {
                DEFAULT_PORT = prt;
                //break; // Stop reading the file after finding the PORT line
            }
        }
    }

   

    // Close the file
    fclose(configFile);
    printf("[CONF] Configured server. Maximum header size: %d. Maximum path size: %d. Pessimistic transfer time: %d. Port: %d.\n", MAX_HEADER_SIZE, MAX_PATH_SIZE, transfer, DEFAULT_PORT);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "help") == 0) {
            printf("[INFO] Simplihost 4.3.3 starting...\n");
            printf("[HELP] Simplihost is a simple and lightweight web server. Like all HTTP websites, it hosts on port 80 by default. The default pessimistic transfer time is 2000 ms. See https://github.com/fossilfishhooks2/simplihost for more information.\n");
            return 0;
        }
        
    }
    printf("[INFO] Simplihost 4.3.3 starting...\n");
  
    configure();
    std::signal(SIGINT, exitHandler);
    std::signal(SIGTERM, exitHandler);
    std::signal(SIGABRT, exitHandler);
    std::signal(SIGBREAK, exitHandler);
    std::signal(SIGFPE, exitHandler);
    std::signal(SIGILL, exitHandler);
    std::signal(SIGSEGV, exitHandler);
    std::signal(SIGABRT_COMPAT, exitHandler);

    printf("[INFO] Attempting to host everything listed in file_map.txt, using pessimistic transfer time %d ms\n", transfer);
    printf("[INFO] Use 'simplihost.exe help' for more information\n");
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[ERROR] Startup failed\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("[ERROR] Socket creation failed\n");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(DEFAULT_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[ERROR] Binding failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[ERROR] Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("[INFO] Server running on port %d...\n", DEFAULT_PORT);

    while (1) {
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("[ERROR] Accept failed\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        printf("[INFO] Client connected\n");

        //handleClient(clientSocket);
        //std::thread t([arg1, arg2]() {
        //threadFunction(arg1, arg2);
        //});
        //std::thread t(handleClient);
         //std::thread t([clientSocket]() {
        //handleClient(clientSocket);
    //});
         //t.join();
        std::thread t([&clientSocket]() {
            handleClient(std::move(clientSocket));
            });
        t.detach();

    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
