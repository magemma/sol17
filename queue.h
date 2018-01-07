/** \file queue.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef QUEUE_H
#define QUEUE_H
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include "config.h"
#include "message.h"
#include "genList.h"
#include <sys/eventfd.h>
struct q {
    pthread_mutex_t myMutex;
    int myCV;
    genList * myQueue;
} typedef queue;

/**
 * @function queueInit
 * @brief Inizializza la coda
 *
 * @param hasKey funzione di comparazione
 * @param freeVal funzione per liberare un elemento
 * @return la coda
 */
queue * queueInit(int(*hasKey)(void*, void*), void(*freeVal)(void*));


/**
 * @function queueFree
 * @brief Libera la coda
 *
 * @param myself coda da liberare
 */
void queueFree(queue * myself);



/**
 * @function enqueue
 * @brief Aggiunge un elemento alla coda
 *
 * @param myself coda
 * @param value elemento da inserire in coda
 * @return 0 in caso di successo, -1 in caso di errore
 */
int enqueue(queue * myself, void* value);

/**
 * @function dequeue
 * @brief Toglie un elemento dalla coda
 *
 * @param myself coda dalla quale prelevare
 * @return value elemento prelevato o NULL in caso di errore
 */
void * dequeue(queue * myself);

#endif


