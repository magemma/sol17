/** \file queue.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include<sys/eventfd.h>
#include<pthread.h>
#include<unistd.h>
#include "config.h"
#include "message.h"
#include "genList.h"
#include"queue.h"

/**
 * @function queueInit
 * @brief Inizializza la coda
 *
 * @param hasKey funzione di comparazione
 * @param freeVal funzione per liberare un elemento
 * @return la coda o NULL in caso di errore
 */
queue * queueInit(int (*hasKey)(void*, void*), void(*freeVal)(void*)) {
    queue * myself = malloc(sizeof(*myself));
    if(myself == NULL) return NULL;
    myself->myQueue = genListInit(hasKey, freeVal);
    if(myself->myQueue == NULL) {
        free(myself);
        return NULL;
    }
    if(pthread_mutex_init(&myself->myMutex, NULL) != 0) {
        genListFree(myself->myQueue);
        free(myself);
        return NULL;
    }
    myself->myCV = eventfd(0, EFD_SEMAPHORE);
    if(myself->myCV == -1) {
        genListFree(myself->myQueue);
        free(myself);
        return NULL;
    }
    return myself;   
}

/**
 * @function queueFree
 * @brief Libera la coda
 *
 * @param myself coda da liberare
 */
void queueFree(queue * myself) {
	if(myself == NULL) return;
	pthread_mutex_destroy(&myself->myMutex);
	close(myself->myCV);
	genListFree(myself->myQueue);
	free(myself);
	return;
}


/**
 * @function enqueue
 * @brief Aggiunge un elemento alla coda
 *
 * @param myself coda
 * @param value elemento da inserire in coda
 * @return 0 in caso di successo, -1 in caso di errore
 */
int enqueue(queue * myself, void* value) {
	pthread_mutex_lock(&myself->myMutex);
	if(put(myself->myQueue, value) == -1) {
		pthread_mutex_unlock(&myself->myMutex);
		return -1;
	}
	pthread_mutex_unlock(&myself->myMutex);
	uint64_t one = 1;
	if(write(myself->myCV, &one, sizeof(one)) == -1) return -1; 
	return 0;
}



/**
 * @function dequeue
 * @brief Toglie un elemento dalla coda
 *
 * @param myself coda dalla quale prelevare
 * @return value elemento prelevato o NULL in caso di errore
 */
void * dequeue(queue * myself) {
	uint64_t one = -1;
	void * value;
	int boh;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &boh);
	if(read(myself->myCV, &one, sizeof(one)) == -1) return NULL;
	//Il file descriptor non è non bloccante, quindi non fallisce. Appena riparte l'esecuzione so di aver prenotato un elemento
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &boh);
	pthread_mutex_lock(&myself->myMutex);
	value = get(myself->myQueue);
	if(value == NULL) {
		pthread_mutex_unlock(&myself->myMutex);
		return NULL;
	}
	pthread_mutex_unlock(&myself->myMutex);
	return value;
}

