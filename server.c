#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define COMMAND_MAZ_SIZE 10
#define SENSOR_MAZ_SIZE 10
#define IN_MAZ_SIZE 10
#define SENSORS_MAX_QUANTITY 4
#define SENSORS_MAX_QUANTITY_IN_SERVER 15

/*
As linhas representam cada um dos equipamentos.
As colunas representam cada um dos sensores.
Para verificar se um sensor esta em um equipamento, o valor linha,coluna deve ser 1. Caso contrario nao esta.

    0   1   2   3
0   0   0   0   0
1   0   0   0   0
2   0   0   0   0
3   0   0   0   0

Sensores
Temperatura 01 > 0
Pressão 02 > 1
Velocidade 03 > 2
Corrente 04 > 3

Equipamentos
Esteira 01 > 0
Guindaste 02 > 1
Ponte Rolante 03 > 2
Empilhadeira 04 > 3
*/
int database[4][4];
int quantidade_sensores = 0;

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

/*
0 > sensor(es) adicionado(s) com sucesso
1-4 > sensor ja existe no equipamento
-1 > limite excedido
*/
int add_sensor(int *sensors, int equipament) {
    printf("ADD SENSOR\n");
    if (quantidade_sensores >= SENSORS_MAX_QUANTITY_IN_SERVER) return -1;

    for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        if (sensors[i] != 0) {
            for (int pos_sensor = 0; pos_sensor<SENSORS_MAX_QUANTITY; pos_sensor ++){
                if (database[equipament-1][pos_sensor] == sensors[i]) return sensors[i];
            }
        }
    }
    for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        if (sensors[i] != 0) {
            for (int pos_sensor = 0; pos_sensor<SENSORS_MAX_QUANTITY; pos_sensor ++){
                if (database[equipament-1][pos_sensor] == 0) {
                    database[equipament-1][pos_sensor] = sensors[i];
                    quantidade_sensores++;
                    break;
                }
            }
        }
    }
    return 0;
}

/*
0 > sensor(es) removido(s) com sucesso
1-4 > sensor nao existe no equipamento
*/
int remove_sensor(int *sensors, int equipament) {
    printf("REMOVE SENSOR\n");

    for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        int has_sensor = 0;
        for (int pos_sensor = 0; pos_sensor<SENSORS_MAX_QUANTITY; pos_sensor ++){
            if (database[equipament-1][pos_sensor] == sensors[i]) {
                has_sensor = 1;
                break;
            }
        }
        if (has_sensor == 0) return sensors[i];
    }
    for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        if (sensors[i] != 0) {
            for (int pos_sensor = 0; pos_sensor<SENSORS_MAX_QUANTITY; pos_sensor ++){
                if (database[equipament-1][pos_sensor] == sensors[i]) {
                    database[equipament-1][pos_sensor] = 0;
                    quantidade_sensores--;
                    for (int pos_sensor_move = pos_sensor; pos_sensor_move<(SENSORS_MAX_QUANTITY-1); pos_sensor_move ++){
                        database[equipament-1][pos_sensor_move] = database[equipament-1][pos_sensor_move+1];
                    }
                }
            }
        }
    }
    return 0;
}

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
    pch = strtok (str," ,.-");
    while (pch != NULL)
    {
        if (strlen(labelCommand) < 1) {
            strcat(labelCommand, pch);
        } else {
            if (strcmp(labelCommand, "add") == 0) {
                if (strlen(labelSensor) < 1) {
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        if (strcmp(pch, "in") == 0) {
                            strcat(labelIn, pch);
                        }
                        if (strlen(labelIn) < 1) {
                            sensors[count] = atoi(pch);
                            count++;
                        } else {
                            equipamentId = atoi(pch);
                        }
                    } else {
                    }
                }
            } else if (strcmp(labelCommand, "remove") == 0) {
                if (strlen(labelSensor) < 1) {
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        if (strcmp(pch, "in") == 0) {
                            strcat(labelIn, pch);
                        }
                        if (strlen(labelIn) < 1) {
                            sensors[count] = atoi(pch);
                            count++;
                        } else {
                            equipamentId = atoi(pch);
                        }
                    } else {
                    }
                }
            } else if (strcmp(labelCommand, "list") == 0) {
                if (strlen(labelSensor) < 1) {
                    strcat(labelSensor, pch);
                } else {
                    if ((strcmp(labelSensor, "sensor") == 0) || (strcmp(labelSensor, "sensors") == 0)) { 
                        if (strlen(labelIn) < 1) {
                            strcat(labelIn, pch);
                        } else {
                            equipamentId = atoi(pch);
                        }
                    } else {
                    }
                }
            } else if (strcmp(labelCommand, "read") == 0) {
                if (strcmp(pch, "in") == 0) {
                    strcat(labelIn, pch);
                }
                if (strlen(labelIn) < 1) {
                    sensors[count] = atoi(pch);
                    count++;
                } else {
                    equipamentId = atoi(pch);
                }
            } else if (strcmp(labelCommand, "kill") == 0) {
            } else {
            }
        }
        pch = strtok (NULL, " ,.-");
    }
    
    if (strcmp(labelCommand, "add") == 0) {
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (i>0) {
                if (sensors[i] < 0 || sensors[i] > 4) return "invalid sensor";
            } else {
                if (sensors[i] <= 0 || sensors[i] > 4) return "invalid sensor";
            }
        }
        if (equipamentId <= 0 || equipamentId > 4) return "invalid equipment";

        int add = add_sensor(sensors, equipamentId);
        if (add == -1) {
            printf("limit exceeded\n");
            return "limit exceeded";
        }
        if (add == 0) {
            char *buf = malloc (sizeof (char) * BUFSZ);
            snprintf(buf, BUFSZ, "sensor");
            for(int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
                if (sensors[i] != 0) {
                    char str[BUFSZ];
                    snprintf(str, BUFSZ, " 0%d", sensors[i]);
                    strcat(buf, str);
                }
            }
            strcat(buf, " added");
            return buf;
        }
        char *buf = malloc (sizeof (char) * BUFSZ);
        snprintf(buf, BUFSZ, "sensor 0%d already exists in 0%d", add, equipamentId);
        return buf;
    }
    
    if (strcmp(labelCommand, "remove") == 0) {
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (i>0) {
                if (sensors[i] < 0 || sensors[i] > 4) return "invalid sensor";
            } else {
                if (sensors[i] <= 0 || sensors[i] > 4) return "invalid sensor";
            }
        }
        if (equipamentId <= 0 || equipamentId > 4) return "invalid equipment";
        int remove = remove_sensor(sensors, equipamentId);
        if (remove == 0) {
            char *buf = malloc (sizeof (char) * BUFSZ);
            snprintf(buf, BUFSZ, "sensor");
            for(int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
                if (sensors[i] != 0) {
                    char str[BUFSZ];
                    snprintf(str, BUFSZ, " 0%d", sensors[i]);
                    strcat(buf, str);
                }
            }
            strcat(buf, " removed");
            return buf;
        }
        char *buf = malloc (sizeof (char) * BUFSZ);
        snprintf(buf, BUFSZ, "sensor 0%d does not exist in 0%d", remove, equipamentId);
        return buf;
    }
    
    if (strcmp(labelCommand, "list") == 0) {
        if (equipamentId <= 0 || equipamentId > 4) return "invalid equipment";
        int has_sensor = 0;
        char *buf = malloc (sizeof (char) * BUFSZ);
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (database[equipamentId-1][i] != 0){
                has_sensor = 1;
                char str[BUFSZ];
                snprintf(str, BUFSZ, "0%d ", database[equipamentId-1][i]);
                strcat(buf, str);
            }
        }
        if (has_sensor == 1) return buf;
        return "none";
    }
    
    if (strcmp(labelCommand, "read") == 0) {
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (i>0) {
                if (sensors[i] < 0 || sensors[i] > 4) return "invalid sensor";
            } else {
                if (sensors[i] <= 0 || sensors[i] > 4) return "invalid sensor";
            }
        }
        if (equipamentId <= 0 || equipamentId > 4) return "invalid equipment";
        char *buf = malloc (sizeof (char) * BUFSZ);
        int has_sensor = 0;
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (sensors[i] != 0){
                for (int pos_sensor = 0; pos_sensor<SENSORS_MAX_QUANTITY; pos_sensor ++){
                    if (database[equipamentId-1][pos_sensor] == sensors[i]) {
                        has_sensor = 0;
                        break;
                    } else{
                        has_sensor = sensors[i];
                    }
                }
            }
        }
        if (has_sensor != 0) {
            snprintf(buf, BUFSZ, "sensor(s) 0%d not installed", has_sensor);
            return buf;
        }
        memset(buf, 0, BUFSZ);
        srand((unsigned int)time(NULL));
        for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
            if (sensors[i] != 0){
                char str[BUFSZ];
                float x = ((float)rand()/(float)(RAND_MAX)) * 10;
                snprintf(str, BUFSZ, "%.2f ", x);
                strcat(buf, str);
            }
        }
        return buf;
    }
    
    return "invalid command";
}

void imprime_database() {
    printf("DATABASE\n");
    for (int i = 0; i<SENSORS_MAX_QUANTITY; i++) {
        printf("[");
        for (int j = 0; j<SENSORS_MAX_QUANTITY; j++) {
            printf(" %d", database[i][j]);
        }
        printf(" ]\n");
    }
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
        imprime_database();
    }
    
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    for (int i = 0; i<4; i++) {
        for (int j = 0; j<4; j++) {
            database[i][j] = 0;
        }
    }

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

