#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define COMMAND_MAZ_SIZE 10
#define SENSOR_MAZ_SIZE 10
#define IN_MAZ_SIZE 10
#define SENSORS_MAX_QUANTITY 4

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

char* message_treating(char *str) {
    char labelCommand[COMMAND_MAZ_SIZE];
    memset(labelCommand, 0, COMMAND_MAZ_SIZE);
    char labelSensor[SENSOR_MAZ_SIZE];
    memset(labelSensor, 0, SENSOR_MAZ_SIZE);
    char labelIn[IN_MAZ_SIZE];
    memset(labelIn, 0, IN_MAZ_SIZE);
    int sensors[SENSORS_MAX_QUANTITY];
    for(int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        sensors[i] = 0;
    }
    int count = 0;
    int equipamentId = 0;
    
    char * pch;
    // Retira o \n
    strtok (str,"\n");
    printf ("Splitting string \"%s\" into tokens:\n",str);
    pch = strtok (str," ,.-");
    while (pch != NULL)
    {
        printf ("Validate: %s\n",pch);
        if (strlen(labelCommand) < 1) {
            printf ("Commando Adicionado: %s\n",pch);
            strcat(labelCommand, pch);
        } else {
            if (strcmp(labelCommand, "add") == 0) {
                printf ("Commando eh add\n");
                if (strlen(labelSensor) < 1) {
                    printf ("Sensor Label Adicionado: %s\n",pch);
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        printf ("Sensor Label sensor(s)\n");
                        if (strcmp(pch, "in") == 0) {
                            printf ("In Label\n"); 
                            strcat(labelIn, pch);
                        }
                        if (strlen(labelIn) < 1) {
                            sensors[count] = atoi(pch);
                            count++;
                            printf ("Adiciona sensor %s\n", pch); 
                        } else {
                            printf ("Ja existe o in, agora eh puxar o equipamento\n"); 
                            equipamentId = atoi(pch);
                        }
                    } else {
                        printf ("Sensor Label desconhecido\n");
                    }
                }
            } else if (strcmp(labelCommand, "remove") == 0) {
                printf ("Commando eh remove\n");
                if (strlen(labelSensor) < 1) {
                    printf ("Sensor Label Adicionado: %s\n",pch);
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        printf ("Sensor Label sensor(s)\n");
                        if (strcmp(pch, "in") == 0) {
                            printf ("In Label\n"); 
                            strcat(labelIn, pch);
                        }
                        if (strlen(labelIn) < 1) {
                            sensors[count] = atoi(pch);
                            count++;
                            printf ("Adiciona sensor %s\n", pch); 
                        } else {
                            printf ("Ja existe o in, agora eh puxar o equipamento\n"); 
                            equipamentId = atoi(pch);
                        }
                    } else {
                        printf ("Sensor Label desconhecido\n");
                    }
                }
            } else if (strcmp(labelCommand, "list") == 0) {
                printf ("Commando eh list\n");
                if (strlen(labelSensor) < 1) {
                    printf ("Sensor Label Adicionado: %s\n",pch);
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        if (strlen(labelIn) < 1) {
                            printf ("In Label\n"); 
                            strcat(labelIn, pch);
                        } else {
                            printf ("Ja existe o in, agora eh puxar o equipamento\n"); 
                            equipamentId = atoi(pch);
                        }
                    } else {
                        printf ("Sensor Label desconhecido\n");
                    }
                }
            } else if (strcmp(labelCommand, "read") == 0) {
                printf ("Commando eh read\n");
                if (strcmp(pch, "in") == 0) {
                    printf ("In Label\n"); 
                    strcat(labelIn, pch);
                }
                if (strlen(labelIn) < 1) {
                    sensors[count] = atoi(pch);
                    count++;
                    printf ("Adiciona sensor %s\n", pch); 
                } else {
                    printf ("Ja existe o in, agora eh puxar o equipamento\n"); 
                    equipamentId = atoi(pch);
                }
            } else if (strcmp(labelCommand, "kill") == 0) {
                printf ("Commando eh kill\n");
            } else {
                printf ("Commando nao reconhecido\n");

            }
        }
        pch = strtok (NULL, " ,.-");
    }
    printf("Command: %s\n", labelCommand);
    printf("Label sensor: %s\n", labelSensor);
    printf("Sensores []:\n");
    for(int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        printf("pos %d - sensor %d\n", i, sensors[i]);
    }
    printf("Label in: %s\n", labelIn);
    printf("Equipament id: %d\n", equipamentId);

    return "sistema supervisorio";
}

void * client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] conexao de %s\n", caddrstr);

    char buf[BUFSZ];
    while (1) {
        memset(buf, 0, BUFSZ);
        size_t count = recv(cdata->csock, buf, BUFSZ - 1, 0);
        if (count == 0) {
            printf("[log] conexao fechada pelo cliente: %s\n", caddrstr);
            break;
        }

        if (strcmp(buf, "kill\n") == 0) {
            printf("[log] kill\n");
            exit(EXIT_SUCCESS);
        }

        strtok(buf, "\n");
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);
        sprintf(buf, "%s\n", message_treating(buf));
        strtok(buf, "\0");
        count = send(cdata->csock, buf, strlen(buf), 0);
        if (count != strlen(buf)) {
            logexit("send");
        }
    }
    
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Ligado a %s, esperando conexoes\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) {
            logexit("malloc");
        }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}

