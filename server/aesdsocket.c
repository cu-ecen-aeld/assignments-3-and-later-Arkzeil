#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define MAX_PENDING_CLIENT 3

volatile sig_atomic_t flag = 0;

static void signal_handler(int signal_number){
    if(signal_number == SIGINT || signal_number == SIGTERM){
        flag = 1;
        if(remove("/var/tmp/aesdsocketdata") != 0)
            perror("Warning:");
        syslog(LOG_INFO, "Caught signal, exiting");
    }
}

void handle_socket(int sk, struct addrinfo *servinfo);

int main(int argc, char *argv[]){
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sigaction new_action;
    int value;
    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    int sk = socket(PF_INET, SOCK_STREAM, 0);
    if(sk == -1){
        printf("Error while creating socket\n");
        return -1;
    }

    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    new_action.sa_flags = SA_RESETHAND;

    if(sigaction(SIGTERM, &new_action, NULL) != 0){
        perror("Error while setting SIGTERM:");
    }
    if(sigaction(SIGINT, &new_action, NULL) != 0){
        perror("Error while setting SIGINT:");
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    // The socket address will be used in a call to the bind function. i.e. ask it to fill the localhost ip for me
    hints.ai_flags = AI_PASSIVE;
    // IPv4
    hints.ai_family = AF_INET;
    // byte-stream
    hints.ai_socktype = SOCK_STREAM;
    // get sockaddr
    int temp;
    if((temp = getaddrinfo(NULL, "9000", &hints, &servinfo)) != 0){
        //printf("%d\n", temp);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(temp));
        perror("Error while getting addrinfo: ");
        return -1;
    }

    if(argc > 1){
        // remember not to use '==', as it will compare their memory address
        if(!strcmp(argv[1],"-d")){
            printf("Daemon\n");
            pid_t PID = fork();
            int status;

            switch(PID){
                case -1:
                    printf("Error while fork\n");
                    freeaddrinfo(servinfo);
                    return -1;
                // child process
                case 0:
                    printf("Child's PID is %d\n", getpid());
                    handle_socket(sk, servinfo);
                    return 0;
                // parent process
                default:
                    printf("Parent's PID is %d\n", getpid());
                    //waitpid(PID, &status, 0);
                    return 0;
            }
        }
    }
    else
        handle_socket(sk, servinfo);

    return 0;
}

void handle_socket(int sk, struct addrinfo *servinfo){
    // Remember to initialize all variable so that valgrind won't complain
    int value = 0;
    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];

    memset(&buffer, 0, sizeof(buffer));
    //memset(&new_action, 0, sizeof(struct sigaction));

    // to reuse address
    if(setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0){
        perror("Error while setting socket option:");
        freeaddrinfo(servinfo);
        return -1;
    }

    if(bind(sk, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
        perror("Error while binding address and port:");
        freeaddrinfo(servinfo);
        return -1;
    }
    // one connection at a time
    if(listen(sk, MAX_PENDING_CLIENT) == -1){
        perror("Error while listening:");
        freeaddrinfo(servinfo);
        return -1;
    }
    //printf("Start listening...\n");

    while(!flag){
        // need to initialize to 0, otherwise segmentation fault
        struct sockaddr_in client_addr = {0};
        int addr_size = sizeof(client_addr);
        FILE *fp;
        ssize_t bytesRead;
        bool complete = false;
        int client_fd = accept(sk, (struct sockaddr*)&client_addr, &addr_size);

        if(client_fd == -1){
            perror("Error while accepting:");
            freeaddrinfo(servinfo);
            return -1;
        }
        //printf("%d\n", client_fd);

        // 32 bits addr in memory length
        char client_ip[INET_ADDRSTRLEN];
        // inet_ntoa function returns a pointer to a statically allocated buffer that may be overwritten by subsequent calls to the function.
        // Convert the client's IP address to a human-readable string
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s\n", client_ip);

        fp = fopen("/var/tmp/aesdsocketdata", "a+");

        while((bytesRead = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0){
            //printf("%d %c\n", bytesRead, buffer[bytesRead - 1]);
            /*char *token = strtok(buffer, "\n");
            char *nextToken;
            complete = false;*/

            if(buffer[bytesRead - 1] == NULL || buffer[bytesRead - 1] == '\n'){
                buffer[bytesRead - 1] = '\0';
                fputs(buffer, fp);
                fputs("\n", fp);
                fflush(fp);
                fseek(fp, 0, SEEK_SET);
                while(fgets(line, sizeof(line), fp)){
                    //printf("Sending:%s", line);
                    if(send(client_fd, line, strlen(line), 0) < 0){
                        perror("Error while sending data:");
                        //return -1;
                    }
                    //printf("Completed sending\n");
                }
                fseek(fp, 0, SEEK_END);
            }
            else
                fputs(buffer, fp);

            // I thought there will be newline character inside a long string, so I used strtok for that scenario
            // But it seemed like there will be no content after newline or null character
            /*while(token != NULL){
                complete = false;
                //printf("%s %d\n", token, strlen(token));
                nextToken = strtok(NULL, "\n");
                fputs(token, fp);
                // if it's NULL, meaning there's data in next recv()
                if(nextToken != NULL){
                    fputs("\n", fp);
                    complete = true;
                }
                else if(bytesRead < BUFFER_SIZE){
                    if(buffer[bytesRead - 1] == '\n' || buffer[bytesRead - 1] == NULL){
                        fputs("\n", fp);
                        complete = true;
                    }
                }

                fflush(fp);
                printf("%d\n", complete);

                if(complete){
                    fseek(fp, 0, SEEK_SET);
                    while(fgets(line, sizeof(line), fp)){
                        //printf("Sending:%s", line);
                        if(send(client_fd, line, strlen(line), 0) < 0){
                            perror("Error while sending data:");
                            //return -1;
                        }
                        printf("Completed sending\n");
                    }
                    fseek(fp, 0, SEEK_END);
                    break;
                }
                
                token = nextToken;
            }*/

            if(flag == 1){
                fclose(fp);
                shutdown(client_fd, 2);
                freeaddrinfo(servinfo);
                if(remove("/var/tmp/aesdsocketdata") != 0)
                    perror("Error while deleting file:");
                return 0;
            }
        }

        syslog(LOG_INFO, "Closed connection from %s\n", client_ip);

        fclose(fp);
        shutdown(client_fd, 2);
    }

    freeaddrinfo(servinfo);
}