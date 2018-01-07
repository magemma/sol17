/** \file conf.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include"conf.h"


/**
 * @function confInit
 * @brief Alloca la struttura dati per le configurazioni
 *  
 * @param argPath path del file di configurazione da aprire
 *
 * @return la struttura dati allocata in caso di successo, NULL in caso di errore
*/ 
conf* confInit(char* argPath) {
    conf* myself = malloc(sizeof(*myself));
    if (myself == NULL) {
    	return NULL;
    }
    myself->myConfigFile = fopen(argPath, "r");
    if (myself->myConfigFile == NULL) {
    	free(myself);
        return NULL;
    }
    myself->myPath=NULL;
    myself->dirName=NULL;
    myself->statFileName=NULL;
    int parseRes = parseFile(myself);
    if(parseRes == -1) {
        fclose(myself->myConfigFile);
        free(myself->myPath);
        free(myself->dirName);
        free(myself->statFileName);
        free(myself);
        return NULL;
    }
    fclose(myself->myConfigFile);
    #ifdef DEBUG   
    printf("UnixPath: %s\n MaxConnections: %d\n ThreadsInPool: %d\n MaxMsgSize: %d\n MaxFileSize: %d\n MaxHistMsgs: %d\n DirName: %s\n StatFileName: %s\n", myself->myPath, myself->maxConnections, myself->threadsInPool, myself->maxMsgSize, myself->maxFileSize, myself->maxHistSize, myself->dirName, myself->statFileName);
    #endif
    return myself;
}

/**
 * @function confFree
 * @brief Libera la struttura dati per le configurazioni
 *  
 * @param myself puntatore alla struttura dati da deallocare
 *
*/ 
void confFree(conf * myself) {
	free(myself->myPath);
    free(myself->dirName);
    free(myself->statFileName);
    free(myself);
    return;
}


/**
 * @function parseLine
 * @brief Modifica le variabili nome del campo e valore. Nota: vale la precondizione che myLine non è un commento e che contiene un =
 *  
 * @param myLine riga da parsare
 * @param fieldName nome del campo
 * @param fieldValue valore del campo
 * @return 0 in caso di successo, non ha mai errori, perchè sfrutta funzioni che non danno luogo ad errori
*/ 
int parseLine(char * myLine, char * fieldName, char * fieldValue) {
    char *p = NULL;
    int isName = 1;
    for (p = myLine; *p; ++p) {
        if (! isspace((int)*p)) {
            //Se non è uno spazio
            if(*p == '=') {
                isName = 0;
                continue;
            }
            if(isName == 1) {
                strncat(fieldName,(const char *) p, 1);
            }
            else {
                strncat(fieldValue, (const char *) p, 1);
            }
        }
    }
    return 0;
}


/**
 * @function isBlankLine
 * @brief Data una riga ritorna 0 se la linea è formata da soli spazi, 1 se non lo è
 *  
 * @param line riga
 * @return 0 in caso di linea formata da soli spazi, 1 in caso di linea non formata da soli spazi
*/ 
int isBlankLine(char * line) {
    char *p = NULL;
    for (p = line; *p; ++p) {
        if (! isspace((int)*p)) {
            //Se non è uno spazio
            return 1;
        }
    }
    return 0;
}

/**
 * @function parseFile
 * @brief Dato il file di configurazione e la struttura dati del server assegna ai campi del server i valori contenuti nel file di configurazione
 *  
 * @param myself struttura dati che rappresenta il server
 * @return 0 in caso di successo, -1 in caso di errore di una funzione di libreria, -2 in caso di file non formato correttamente
*/ 
int parseFile(conf * myself) {
    char * line = NULL;
    size_t len = 0;
    // Legge tutte le righe che può
    while (getline(&line, &len,myself->myConfigFile) != -1) {
		if(strchr(line, '#') == NULL) {
			//Non è un commento
	        if(isBlankLine(line) == 0) {
                //La linea è formata da soli spazi
                continue;
            }
            if(strstr(line, "=") == NULL) {
	            //Il file non è in un formato valido
                errno = EINVAL;
	            return -2;
	        }
        	else {
		        char * fieldName=calloc(strlen(line)+1, sizeof(char));
		        char * fieldValue=calloc(strlen(line)+1, sizeof(char));
		        if( fieldName == NULL || fieldValue == NULL) {
		            free(fieldName);
                    free(fieldValue);
                    return -1;
		        }
		        if (parseLine(line, fieldName, fieldValue) == -1) {
		            free(fieldName);
                    free(fieldValue);
                    return -1;
		        }
		        else {
                    //Nota: alloco esattamente la memoria necessaria a contenere il campo
		            if (strncmp(fieldName, "UnixPath", strlen("UnixPath")+1) == 0) {
                        myself->myPath = calloc(strlen(fieldValue)+1, sizeof(char));
                        if(myself->myPath == NULL) {
                            free(fieldName);
                            free(fieldValue);
                            return -1;
                        }
                        strncpy(myself->myPath, fieldValue, strlen(fieldValue)+1);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
		            if (strncmp(fieldName, "MaxConnections", strlen("maxConnections")+1) == 0) {
		                myself->maxConnections = (int) strtol((const char *) fieldValue, NULL, 10);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
		            if (strncmp(fieldName, "ThreadsInPool", strlen("ThreadsInPool")+1) == 0) {
		                myself->threadsInPool = (int) strtol((const char *) fieldValue, NULL, 10);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
		            if (strncmp(fieldName, "MaxMsgSize", strlen("MaxMsgSize")+1) == 0) {
		                myself->maxMsgSize = (int) strtol((const char *) fieldValue, NULL, 10);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }

		            if (strncmp(fieldName, "MaxFileSize", strlen("MaxFileSize")+1) == 0) {
		                myself->maxFileSize = (int) strtol((const char *) fieldValue, NULL, 10);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }

		            if (strncmp(fieldName, "MaxHistMsgs", strlen("MaxHistMsgs")+1) == 0) {
		                myself->maxHistSize = (int) strtol((const char *) fieldValue, NULL, 10);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
		            if (strncmp(fieldName, "DirName", strlen("DirName")+1) == 0) {
		                myself->dirName = calloc(strlen(fieldValue)+1, sizeof(char));
                        if(myself->dirName == NULL) {
                            free(fieldName);
                            free(fieldValue);
                            return -1;
                        }
                        strncpy(myself->dirName, fieldValue, strlen(fieldValue)+1);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
		             if (strncmp(fieldName, "StatFileName", strlen("StatFileName")+1) == 0) {
		                myself->statFileName = calloc(strlen(fieldValue)+1, sizeof(char));
                        if(myself->statFileName == NULL) {
                            free(fieldName);
                            free(fieldValue);
                            return -1;
                        }
                        strncpy(myself->statFileName, fieldValue, strlen(fieldValue)+1);
		                free(fieldName);
                        free(fieldValue);
                        continue;
		            }
            	}
        	}
    	}
    }
    free(line);
    return 0;
}
