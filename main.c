#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <asm-generic/socket.h>

#define clear() printf("\033[H\033[J")          // Clearing the shell using escape sequences
#define MAX_APP 10                              // Max device connection

char IP[50];
char cmd[100];
char option[100];
char str[100];

pthread_t accept_thread_id, receive_thread_id;

typedef struct {
    int id;
    int fd;
    int port_num;
    char myip[50];
    struct sockaddr_in addr;
} application;

// App current
application current_app = {0};

// Take connection from this app
application app_connect_to[MAX_APP] = {0};

// Connect to this app
// application app_connect_from[MAX_APP] = {0};

// Number of app
// int sum_app_from = 0;
int sum_app_to = 0;

// Init screen Application
void init_screen() {
    clear();
    printf("\n| -------------------------------------------------------------------------------- |");
    printf("\n| ______________________________  Chat Application  ______________________________ |");
    printf("\n|                                                                                  |");
    printf("\n| >> Use the commands below:                                                       |");
    printf("\n|                                                                                  |");
    printf("\n| 1. help                             : display user interface options             |");
    printf("\n| 2. myip                             : display IP address of this app             |");
    printf("\n| 3. myport                           : display listening port of this app         |");
    printf("\n| 4. connect <destination> <port no>  : connect to the app of another computer     |");
    printf("\n| 5. list                             : list all the connection of this app        |");
    printf("\n| 6. terminate <connection id>        : terminate the connection                   |");
    printf("\n| 7. send <connection id> <message>   : send a message to a connection             |");
    printf("\n| 8. exit                             : close all connection & terminate this app  |");
    printf("\n|                                                                                  |");
    printf("\n| -------------------------------------------------------------------------------- |\n");
}

//Function to get option command
void *choose_option(char *str, char *cmd) {
    char *token = strtok(str, " ");

    while(token != NULL) {
        strcpy(cmd, token);
        token = strtok(NULL, " ");
        break;
    }
}

// Display help
void show_help() {
    printf("\n| --------------------------------  Commands Menu  ------------------------------- |");
    printf("\n|                                                                                  |");
    printf("\n| >> myip                             : display IP address of this app             |");
    printf("\n| >> myport                           : display listening port of this app         |");
    printf("\n| >> connect <destination> <port no>  : connect to the app of another computer     |");
    printf("\n| >> list                             : list all the connection of this app        |");
    printf("\n| >> terminate <connection id>        : terminate the connection                   |");
    printf("\n| >> send <connection id> <message>   : send a message to a connection             |");
    printf("\n| >> exit                             : close all connection & terminate this app  |");
    printf("\n|                                                                                  |");
    printf("\n| -------------------------------------------------------------------------------- |\n");
}

// Display my IP
void show_myIP(char *myip) {
    FILE *fd = popen("hostname -I", "r");

    if (fd == NULL) {
        printf("\nError: can not get IP address");
        return;
    }

    if (fgets(myip, 100, fd) == NULL) {
        printf("\nError: can not get IP address");
        return;
    }

    printf("\nIP address of this app: %s", myip);
}

// Display port
void show_port() {
    printf("\nListening port of this app: %d", current_app.port_num);
}

// Display all connection
void list_connect() {
    printf("\n| --------------  App connection to  ------------- |");
    printf("\n|  ID  |          IP Address          |  Port No.  |");

    for (int i = 0; i < MAX_APP; i++) {
        if (app_connect_to[i].fd > 0) {
            printf("\n|  %d   |          %s         |   %d   |", app_connect_to[i].id, app_connect_to[i].myip, app_connect_to[i].port_num);
        }
    }
    printf("\n| ________________________________________________ |");

    // printf("\n| -------------  App connection from  ------------ |");
    // printf("\n|  ID  |          IP Address          |  Port No.  |");

    // for (int i = 0; i < sum_app_from; i++) {
    //     if (app_connect_from[i].fd > 0) {
    //         printf("\n|  %d   |          %s          |  %d  |", app_connect_from[i].id, app_connect_from[i].myip, app_connect_from[i].port_num);
    //     }
    // }
    // printf("\n| ------------------------------------------------ |");
}

// Function to send message
int send_msg(application app, char *msg) {
    if (app.fd < 0) {
        printf("\nError: this app has been terminated");
        return -1;
    }

    if (write(app.fd, msg, 100) < 0) {
        printf("\nError: can not send message");
        return -1;
    }
    return 0;
}

// Function to terminate connection
void terminate_id(application *app) {
    char str[100];
    sprintf(str, "\nThe peer at port %d has disconnected", app->port_num);
    send_msg(*app, str);

    app->fd = -1;
}

// Function to receive message from another app
static void* receive_msg(void *para) {
    application *msg_app = (application *)para;
    char buff[100];
    
    while(1) {
        if (read(msg_app->fd, buff, 100) < 0) {
            printf("\nError: can not read message");
            return NULL;
        }

        if ((msg_app->fd) >= 0) {
            printf("\n| -------------------------------------------------");
            printf("\n| __ Message receive from : %s", msg_app->myip);
            printf("\n| __ Sender's port        : %d", msg_app->port_num);
            printf("\n| __ Message              : %s", buff);
            printf("\n| -------------------------------------------------");
        }
        else {
            printf("\nNotification: %s", buff);
        }
    }
}

// Function to connect to new app
int connect_app(application app) {
    if (connect(app.fd, (struct sockaddr *)&app.addr, sizeof(app.addr)) < 0) {
        return -1;
    }
    return 0;
}

// Function to take accept new app
static void* accept_app(void* para) {
    int client_fd, ret;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    while(1) {
        client_fd = accept(current_app.fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd == -1) {
            printf("\nError: can not accept new app");
            return NULL;
        }

        // Save infor new app
        app_connect_to[sum_app_to].fd = client_fd;
        app_connect_to[sum_app_to].id = sum_app_to;
        app_connect_to[sum_app_to].addr = client_addr;
        app_connect_to[sum_app_to].port_num = ntohs(client_addr.sin_port);

        inet_ntop(AF_INET, &client_addr.sin_addr, app_connect_to[sum_app_to].myip, 50);

        printf("\nAccept a new connection from address: %s, setup at port: %d\n", app_connect_to[sum_app_to].myip, app_connect_to[sum_app_to].port_num);

        // Create thread to get message from new app
        if (ret = pthread_create(&receive_thread_id, NULL, &receive_msg, &app_connect_to[sum_app_to])) {
            printf("\nError: can not create thread to receive message");
        }

        sum_app_to++;
    }
}

void exit_handler() {
    printf("\n-------->>> Exiting program .....\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    init_screen();

    if (signal(SIGINT,exit_handler) == SIG_ERR) {
        printf("\nCan not handler SIGINT");
    }

    // Create socket for this app
    current_app.fd = socket(AF_INET, SOCK_STREAM, 0);

    // Prevent error: “address already in use”
    setsockopt(current_app.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));

    if (current_app.fd == -1) {
        printf("\nError: can not create socket for this app");
        return -1;
    }

    // Read the portnumber on the command line
    if (argc < 2) {
        printf("\nNo port provided\ncommand: ./server <port number>\n");
        exit(EXIT_FAILURE);
    } 
    else {
        current_app.port_num = atoi(argv[1]);
    }
    
    current_app.addr.sin_family = AF_INET;
    current_app.addr.sin_port = htons(current_app.port_num);
    current_app.addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(current_app.fd, (struct sockaddr *)&current_app.addr, sizeof(current_app.addr)) == -1) {
        printf("\nError: can not make bind for this socket");
        return -1;
    }

    if (listen(current_app.fd, MAX_APP) == -1) {
        printf("\nError: can not listen for this device");
        return -1;
    }

    printf("\nApplication is listening on port : %d\n", current_app.port_num);

    // if (pthread_create(&accept_thread_id, NULL, &accept_app, NULL)) {
    //     printf("\nError: can not create thread for accept new app");
    //     return -1;
    // }

    while(1) {
        if (pthread_create(&accept_thread_id, NULL, &accept_app, NULL)) {
            printf("\nError: can not create thread for accept new app");
            return -1;
        }
        
        printf("\nEnter your command:  ");
        fgets(cmd, 130, stdin);
        cmd[strcspn(cmd, "\n")] = '\0';
        strcpy(str, cmd);

        // Input command and take option
        choose_option(str, option);

        if (!strcmp(option, "help")) {
            show_help();
        }

        else if (!strcmp(option, "myip")) {
            show_myIP(current_app.myip);
        }

        else if (!strcmp(option, "myport")) {
            show_port();
        }

        else if (!strcmp(option, "connect")) {
            int port_no;
            char IP[20];
            char temp[10];

            // Gtet IP and port
            sscanf(cmd, "%s %s %d", temp, IP, &port_no);

            // Define new app by IP and port
            app_connect_to[sum_app_to].fd = socket(AF_INET, SOCK_STREAM, 0);
            app_connect_to[sum_app_to].id = sum_app_to;
            app_connect_to[sum_app_to].port_num = port_no;
            strcpy(app_connect_to[sum_app_to].myip, IP);
            app_connect_to[sum_app_to].addr.sin_family = AF_INET;
            app_connect_to[sum_app_to].addr.sin_port = htons(app_connect_to[sum_app_to].port_num);

            inet_pton(AF_INET, app_connect_to[sum_app_to].myip, &app_connect_to[sum_app_to].addr.sin_addr);

            if (connect_app(app_connect_to[sum_app_to])) {
                printf("\nError: can not connect to new app");
            }
            else {
                printf("\nConnection successfully\n");
                sum_app_to++;
            }
        }

        else if (!strcmp(option, "list")) {
            list_connect();
        }

        else if (!strcmp(option, "terminate")) {
            int id_temp;
            char temp[10];


            // Get input and id
            sscanf(cmd, "%s %d", temp, &id_temp);

            for (int i = 0; i < sum_app_to; i++) {
                if (id_temp == app_connect_to[i].id) {
                    terminate_id(&app_connect_to[i]);
                    sum_app_to--;
                }
            }
        }

        else if (!strcmp(option, "send")) {
            char temp[10];
            char msg[100];
            int id;

            // Take ID and message
            sscanf(cmd, "%s %d %[^\n]", temp, &id, msg);

            for (int i = 0; i < sum_app_to; i++) {
                if (id == app_connect_to[i].id) {
                    send_msg(app_connect_to[i], msg);
                }
            }
        }

        else if (!strcmp(option, "exit")) {
            for (int i = 0; i < sum_app_to; i++) {
                terminate_id(&app_connect_to[i]);
            }

            printf("\n| --------------------------------------- |");
            printf("\n| >>> _____ Exiting Application _____ <<< |");
            printf("\n| --------------------------------------- |");
            break;
        }

        else {
            printf("\nInvalid Command!");
        }
    }
    return 0;
}