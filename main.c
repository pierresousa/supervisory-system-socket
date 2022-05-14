#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFSZ 1024
#define COMMAND_MAZ_SIZE 10
#define SENSOR_MAZ_SIZE 10
#define IN_MAZ_SIZE 10
#define SENSORS_MAX_QUANTITY 4

int main(int argc, char **argv) {
	// char buf[BUFSZ];
	// memset(buf, 0, BUFSZ);
	// printf("mensagem> ");
	// fgets(buf, BUFSZ-1, stdin);
    // printf("%s", buf);
    // for (int i=0; i<strlen(buf); i++) {
    //     printf("%d - %c\n", i, buf[i]);
    // }

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

    // OK
    // char str[] ="add sensor 01 03 in 02\n";
    // OK
    // char str[] ="remove sensor 01 in 02\n";
    // OK
    // char str[] ="list sensors in 02\n";
    // OK
    char str[] ="read 01 03 in 01\n";
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

	exit(EXIT_SUCCESS);
}