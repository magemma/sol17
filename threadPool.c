/** \file threadPool.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>
#include "config.h"
#include "message.h"
#include "threadPool.h"
#include"server.h"
#include"messageHandler.h"

/**
 * @function freeSocket
 * @brief Libera un socket
 *
 * @param value valore da liberare
 */
void freeSocket(void* value) {
	free((int*)value);
	return;
}


/**
 * @function freeThread
 * @brief Libera un thread
 *
 * @param value valore da liberare
 */
void freeThread(void* value) {
	pthread_join(*(pthread_t*)value, NULL);
	//Non può fallire, quindi non mi interesso al retval
	free((pthread_t*)value);
	return;
}

/**
 * @function threadPoolInit
 * @brief Inizializza la thread pool
 *
 * @param threadsInPool di thread nel pool
 * @return struttura dati allocata
 */
threadPool * threadPoolInit(int threadsInPool) {
	threadPool * myself = malloc(sizeof(*myself));
	if(myself == NULL) return NULL;
	myself->threadsInPool = threadsInPool;
    myself->toServeQueue = queueInit(NULL, freeSocket);
	if(myself->toServeQueue == NULL) {
		free(myself);
		return NULL;
	}
	myself->servedQueue = queueInit(NULL, freeSocket);
	if(myself->servedQueue == NULL) {
		queueFree(myself->toServeQueue);
		free(myself);
		return NULL;
	}
	myself->myThreadList = genListInit(NULL, freeThread);
	if(myself->myThreadList == NULL) {
		queueFree(myself->toServeQueue);	
		queueFree(myself->servedQueue);
		free(myself);
		return NULL;
	}
	return myself;
}


/**
 * @function threadPoolFree
 * @brief Libera la thread pool
 *
 * @param myself struttura dati threadpool
 */
void threadPoolFree(threadPool * myself) {
	if(myself == NULL) return;
    threadPoolStop(myself);
	queueFree(myself->toServeQueue);
	queueFree(myself->servedQueue);	
	genListFree(myself->myThreadList);
	free(myself);
	return;
}

/**
 * @function startRoutine
 * @brief Il codice che esegue ogni thread
 *
 * @param serverS puntatore alla struttura dati del server
 * @return NULL
 */
void* startRoutine(void* serverS) {
    maskAllSignals();
    serverStuff * server = (serverStuff*) serverS;
    while(1) {
        int * mySock = dequeue(server->myThreadPool->toServeQueue);
        if(mySock == NULL) perror("Error while dequeueing");
        mySock = readAndDealWithMessage(mySock, server);
        if(mySock == NULL) continue;
        if (enqueue(server->myThreadPool->servedQueue, (void*) mySock) == -1) perror("Error while enqueueing");
    }
    return NULL;
}

/**
 * @function threadPoolStart
 * @brief Fa partire la threadPool
 *
 * @param myself puntatore alla struttura dati threadpool da avviare
 * @param serverS puntatore alla struttura dati del server
 * @return 0 in caso di avvio con successo, -1 in caso di errore
 */
int threadPoolStart(threadPool * myself, void* serverS) {
    int res;
    pthread_t* currThread;
    for(int i=0; i<myself->threadsInPool; i++) {
        currThread = malloc(sizeof(pthread_t));
        res = pthread_create(currThread,NULL,startRoutine, serverS);
        if(res != 0) {
            free(currThread);
            return -1;
        }
        if(put(myself->myThreadList, currThread) != 0) {
            free(currThread);
            return -1;
        }
    }
    return 0;
}

/**
 * @function threadPoolKindStop
 * @brief Ferma la threadPool con rispetto per le cose che sono in corso(SIGTERM e SIGINT)
 *
 * @param myself struttura dati threadpool
 * @return 0 in caso di successo, -1 in caso di errore
 */
int threadPoolKindStop(threadPool * myself) {
    pthread_t * currThread;
    void * retVal;
    for(int i=0; i<myself->threadsInPool; i++) {
        currThread = get(myself->myThreadList);
        if(currThread == NULL) return -1;
        if(pthread_cancel(*currThread) != 0) return -1;
        if(pthread_join(*currThread, &retVal) != 0) return -1;
        free(currThread);
    }
    myself->threadsInPool = 0;
    return 0;
}

/**
 * @function threadPoolStop
 * @brief Ferma la threadPool in maniera brutale(SIGQUIT) senza occuparsi del cleanup, in modo da lasciare all'utente la possibilità di controllare lo stato della memoria
 *
 * @param myself struttura dati threadpool
 * @return 0 in caso id successo, -1 in caso di errore
 */
int threadPoolStop(threadPool * myself) {
    pthread_t * currThread;
    void * retVal;
    for(int i=0; i<myself->threadsInPool; i++) {
        currThread = get(myself->myThreadList);
        if(currThread == NULL) return -1;
        if(pthread_kill(*currThread, SIGKILL) != 0) return -1;
        if(pthread_join(*currThread, &retVal) != 0) return -1;
        free(currThread);
    }
    //Non va fatto, per definizione di chiusura brutale threadPoolFree(myself);
    return 0; 
}


