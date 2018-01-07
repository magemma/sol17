/** \file threadPool.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

#include "config.h"
#include "message.h"
#include "queue.h"
#include "genList.h"

struct tp {
    genList * myThreadList;
	queue * toServeQueue;
	queue * servedQueue;
    int threadsInPool;
} typedef threadPool;


/**
 * @function freeSocket
 * @brief Libera un socket
 *
 * @param value valore da liberare
 */
void freeSocket(void* value);

/**
 * @function freeThread
 * @brief Libera un thread
 *
 * @param value valore da liberare
 */
void freeThread(void* value);

/**
 * @function threadPoolInit
 * @brief Inizializza la thread pool
 *
 * @param threadsInPool di thread nel pool
 * @return struttura dati allocata
 */
threadPool * threadPoolInit(int threadsInPool);

/**
 * @function threadPoolFree
 * @brief Libera la thread pool
 *
 * @param myself struttura dati threadpool
 */
void threadPoolFree(threadPool * myself);

/**
 * @function startRoutine
 * @brief Il codice che esegue ogni thread
 *
 * @param serverS puntatore alla struttura dati del server
 * @return NULL
 */
void* startRoutine(void* serverS);

/**
 * @function threadPoolStart
 * @brief Fa partire la threadPool
 *
 * @param myself puntatore alla struttura dati threadpool da avviare
 * @param serverS puntatore alla struttura dati del server
 * @return 0 in caso di avvio con successo, -1 in caso di errore
 */
int threadPoolStart(threadPool * myself, void* serverS);

/**
 * @function threadPoolKindStop
 * @brief Ferma la threadPool con rispetto per le cose che sono in corso(SIGTERM       e SIGINT)
 *
 * @param myself struttura dati threadpool
 * @return 0 in caso di successo, -1 in caso di errore
 */
int threadPoolKindStop(threadPool * myself);

/**
 * @function threadPoolStop
 * @brief Ferma la threadPool in maniera brutale(SIGQUIT) senza occuparsi del cleanup, in modo da lasciare all'utente la possibilità di controllare lo stato della memoria
 *
 * @param myself struttura dati threadpool
 * @return 0 in caso id successo, -1 in caso di errore
 */
int threadPoolStop(threadPool * myself);


/**
 * @function maskAllSignals
 * @brief Blocca tutti i segnali che ha senso bloccare nel thread corrente
 */
static inline void maskAllSignals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

#endif
