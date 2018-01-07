/** \file conf.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef CONF_H
#define CONF_H
#include<stdio.h>
#include<stdlib.h>

struct conf1 {
    char * myPath;
    int maxConnections;
    int threadsInPool;
    int maxMsgSize;
    int maxFileSize;
    int maxHistSize;
    char * dirName;
    char * statFileName;
    FILE* myConfigFile;
} typedef conf;

/**
 * @function confInit
 * @brief Alloca la struttura dati per le configurazioni
 *  
 * @param argPath path del file di configurazione da aprire
 *
 * @return la struttura dati allocata in caso di successo, NULL in caso di errore
*/ 
conf* confInit(char * argPath);

/**
 * @function confFree
 * @brief Libera la struttura dati per le configurazioni
 *  
 * @param myself puntatore alla struttura dati da deallocare
 *
*/ 
void confFree(conf * myself);
	
/**
 * @function parseLine
 * @brief Modifica le variabili nome del campo e valore. Nota: vale la precondizione che myLine non è un commento e che contiene un =
 *  
 * @param myLine riga da parsare
 * @param fieldName nome del campo
 * @param fieldValue valore del campo
 * @return 0 in caso di successo, non ha mai errori, perchè sfrutta funzioni che non danno luogo ad errori
*/ 
int parseLine(char * myLine, char * fieldName, char * fieldValue);

/**
 * @function isBlankLine
 * @brief Data una riga ritorna 0 se la linea è formata da soli spazi, 1 se non lo è
 *  
 * @param line riga
 * @return 0 in caso di linea formata da soli spazi, 1 in caso di linea non formata da soli spazi
*/ 
int isBlankLine(char * line);

/**
 * @function parseFile
 * @brief Dato il file di configurazione e la struttura dati del server assegna ai campi del server i valori contenuti nel file di configurazione
 *  
 * @param myself struttura dati che rappresenta il server
 * @return 0 in caso di successo, -1 in caso di errore di una funzione di libreria, -2 in caso di file non formato correttamente
*/ 
int parseFile(conf * myself);



#endif
