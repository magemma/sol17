/** \file stat.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <pthread.h>
#include"stat.h"


/**
 * @function statInit
 * @brief Inizializza e alloca la struttura dati stat
 *
 * @param statPath path del file delle statistiche
 * @return il puntatore alla struttura dati in caso di successo, NULL in caso di errore
 */
stat * statInit(char* statPath) {
    stat* myself = calloc(1, sizeof(*myself));
    if(myself == NULL) return NULL;
    myself->statFile = fopen(statPath, "w");
    if(myself->statFile == NULL) {
        free(myself);
        return NULL;
    }
    if(pthread_mutex_init(&myself->myMutex, NULL) != 0) {
        fclose(myself->statFile);
        free(myself);
        return NULL;
    }
    return myself;
}   

/**
 * @function statFree
 * @brief Dealloca la struttura dati stat
 *
 * @param myself puntatore alla struttura delle statistiche
 */
void statFree(stat* myself) {
    if (myself == NULL) return;
    fclose(myself->statFile);
    pthread_mutex_destroy(&myself->myMutex);
    free(myself);
    return;
}   

/**
 * @function addMember
 * @brief Incrementa di 1 il numero di membri del sistema
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void addMember(stat* myStat) {
	pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyMembers++;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function removeMember
 * @brief Decrementa di 1 il numero di membri del sistema
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void removeMember(stat* myStat) {
	pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyMembers--;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function login
 * @brief Incrementa di 1 il numero di utenti loggati
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void login(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyOnline++;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function logout
 * @brief Decrementa di 1 il numero di utenti loggati
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void logout(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyOnline--;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function messageDelivered
 * @brief Incrementa di 1 il numero di messaggi consegnati
 *		  Decrementa di 1 il numero di messaggi da consegnare.
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void messageDelivered(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyMsgDelivered++;
    myStat->howManyMsgToDeliver--;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function messageToDeliver
 * @brief Incrementa di 1 il numero di messaggi da consegnare
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void messageToDeliver(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyMsgToDeliver++;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function fileDelivered
 * @brief Incrementa di 1 il numero di file consegnati
 *		  Decrementa di 1 il numero di file da consegnare.
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void fileDelivered(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyFileDelivered++;
    myStat->howManyFileToDeliver--;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function fileToDeliver
 * @brief Incrementa di 1 il numero di file da consegnare
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void fileToDeliver(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyFileToDeliver++;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function errorMessage
 * @brief Incrementa di 1 il numero di messaggi di errore
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void errorMessage(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    myStat->howManyErrMsg++;
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}

/**
 * @function printStatsFile
 * @brief Appende nel file delle statistiche <timestamp-statdata>
 *
 * @param myStat puntatore alla struttura dati che rappresenta le statistiche
 */
void printStats(stat* myStat) {
    pthread_mutex_lock(&myStat->myMutex);
    fprintf(myStat->statFile, "%lu - %d %d %d %d %d %d %d\n", (unsigned long)time(NULL), myStat->howManyMembers, myStat->howManyOnline, myStat->howManyMsgDelivered, myStat->howManyMsgToDeliver, myStat->howManyFileDelivered, myStat->howManyFileToDeliver, myStat->howManyErrMsg);
    fflush(myStat->statFile);
    pthread_mutex_unlock(&myStat->myMutex);
    return;
}
