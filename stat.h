/** \file stat.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef STAT_H
#define STAT_H
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
struct stat1 {
    int howManyMembers;
    int howManyOnline;
    int howManyMsgDelivered;
    int howManyMsgToDeliver;
    int howManyFileDelivered;
    int howManyFileToDeliver;
    int howManyErrMsg;
    FILE * statFile;
    pthread_mutex_t myMutex;
} typedef stat;

/**
 * @function statInit
 * @brief Inizializza e alloca la struttura dati stat
 *
 * @param statPath path del file delle statistiche
 * @return il puntatore alla struttura dati in caso di successo, NULL in caso di errore
 */
stat * statInit(char* statPath); 

/**
 * @function statFree
 * @brief Dealloca la struttura dati stat
 *
 * @param myself puntatore alla struttura delle statistiche
 */
void statFree(stat* myself); 

/**
 * @function addMember
 * @brief Incrementa di 1 il numero di membri del sistema
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void addMember(stat* myStat);

/**
 * @function removeMember
 * @brief Decrementa di 1 il numero di membri del sistema
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void removeMember(stat* myStat); 

/**
 * @function login
 * @brief Incrementa di 1 il numero di utenti loggati
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void login(stat* myStat); 

/**
 * @function logout
 * @brief Decrementa di 1 il numero di utenti loggati
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void logout(stat* myStat); 

/**
 * @function messageDelivered
 * @brief Incrementa di 1 il numero di messaggi consegnati.
 *		  Decrementa di 1 il numero di messaggi da consegnare.
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void messageDelivered(stat* myStat); 

/**
 * @function messageToDeliver
 * @brief Incrementa di 1 il numero di messaggi da consegnare
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void messageToDeliver(stat* myStat);

/**
 * @function fileDelivered
 * @brief Incrementa di 1 il numero di file consegnati
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void fileDelivered(stat* myStat); 

/**
 * @function fileToDeliver
 * @brief Incrementa di 1 il numero di file da consegnare
 *		  Decrementa di 1 il numero di file da consegnare.
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void fileToDeliver(stat* myStat);

/**
 * @function errorMessage
 * @brief Incrementa di 1 il numero di messaggi di errore
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void errorMessage(stat* myStat);

/**
 * @function printStatsFile
 * @brief Appende nel file delle statistiche <timestamp-statdata>
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void printStats(stat* myStat);
#endif
