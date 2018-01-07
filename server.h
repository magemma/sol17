/** \file server.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef SERVER_H
#define SERVER_H
#include<stdio.h>
#include<stdlib.h>
#include"config.h"
#include"message.h"
#include"stat.h"
#include"conf.h"
#include"genList.h"
#include"threadPool.h"

struct s2 {
    genList * usersList;
    genList * onlineList;
    conf * myConfig;
    stat* myStat;
    threadPool * myThreadPool;
    int myServerSocket;
    pthread_mutex_t myMutex;
} typedef serverStuff;


/**
 * @function init
 * @brief Alloca la memoria per la struttura dati che rappresenta il server
 *  
 * @param confPath path del file di configurazione
 * @return myself struttura dati allocata in caso di successo, altrimenti null
*/ 
serverStuff * init(char* confPath);

/**
 * @function onError
 * @brief Funzione di clean up che dealloca tutta la struttura dati del server
 *  
 * @param myself puntatore alla struttura dati da allocare
 *
*/ 
void onError(serverStuff * myself); 

/**
 * @function startCommunication
 * @brief Crea il socket e gli dà un nome
 *   
 * @param myself struttura dati del server da modificare
 * @return 0 se è andato tutto bene, -1 in caso di errore
*/ 
int startCommunication(serverStuff *myself);
#endif
