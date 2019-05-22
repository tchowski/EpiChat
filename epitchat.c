#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define PORT 5800

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[30],
        max_clients = 30, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    char buffer[1025], nick[30][50];
    fd_set readfds;
    char ascii[] = "\n\
      ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: :::    :::\n\
     :+:       :+:    :+:   :+:        :+:    :+:      :+:    :+::+:    :+:\n\
    +:+       +:+    +:+   +:+        +:+    +:+      +:+       +:+    +:+\n\
   +#++:++#  +#++:++#+    +#+        +#+    +#++:++# +#+       +#++:++#++\n\
  +#+       +#+          +#+        +#+    +#+      +#+       +#+    +#+\n\
 #+#       #+#          #+#        #+#    #+#      #+#    #+##+#    #+#\n\
#############      ###########    ###    ################## ###    ###\n\n\
Tchat EpiCrew 2019.\r\nPlease enter your nickname to be used in this chat\n\
Nickname: ";
    char *password = "Epicr3w\n";

    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
            sizeof(opt))
        < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(master_socket, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE) {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
            printf("select error");
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket,
                     (struct sockaddr *)&address, (socklen_t *)&addrlen))
                < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            while (TRUE) {
                char buffer[1024];

                memset(buffer, 0, 1024);
                send(new_socket, "Put password bitch: ", strlen("Put password bitch: \n"), 0);
                recv(new_socket, buffer, 1024, 0);
                if (strncmp((const char *)buffer, password, strlen(password)) == 0) {
                    memset(buffer, 0, 1024);
                    break;
                }
                printf("password: %s", buffer);
            }
            if (send(new_socket, ascii, strlen(ascii), 0) != strlen(ascii)) {
                perror("send");
            }
            for (i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    recv(new_socket, nick[i], 49, 0);
                    for (int j = 0; nick[i][j] != '\0'; j++) {
                        if (nick[i][j] == '\n') {
                            nick[i][j] = '\0';
                            break;
                        }
                    }
                    strcat(nick[i], ": ");
                    printf("%s\n", nick[i]);
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    getpeername(sd, (struct sockaddr *)&address,
                        (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_socket[i] = 0;
                    char disconnected[1024];
                    char delim[] = ":";
                    strtok(nick[i], delim);
                    fprintf(stderr, "%s Disconnected!\n", nick[i]);

                    snprintf(disconnected, 1024, "%s Disconnected!\n", nick[i]);
                    for (int j = 0; j < max_clients; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != client_socket[i]) {
                            send(client_socket[j], disconnected, strlen(disconnected), 0);
                        }
                    }
                    memset(nick[i], 0, 1);
                }
                else {
                    char temp[1075];
                    char *sendmsg = strcat(temp, nick[i]);
                    sendmsg = strcat(temp, buffer);
                    fprintf(stderr, "string %s", sendmsg);
                    for (int j = 0; j < max_clients; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != client_socket[i])
                            send(client_socket[j], sendmsg, strlen(sendmsg), 0);
                    }
                }
                memset(buffer, 0, 1024);
            }
        }
    }
    return 0;
}