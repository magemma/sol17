/** \file genList.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef GENLIST_H
#define GENLIST_H
#include<stdio.h>
#include<stdlib.h>

struct sl0 {
    void * val;
    struct sl0 * next;
} typedef gList;

struct sl {
    gList * list;
    int (*hasKey)(void*, void*);//Primo parametro è l'elemento da comparare, il secondo è la chiave
    void (*freeVal)(void*);//Funzione che libera un elemento val
    gList ** lastNext;
} typedef genList;


/**
 * @function genListInit
 * @brief Crea una lista generica
 *
 * @param hasKey il puntatore alla funzione di confronto
 * @param freeVal il puntatore alla funzione per liberare un elemento
 * @return il puntatore alla lista, NULL in caso di errore di malloc
 */
genList * genListInit(int (*hasKey)(void*, void*), void (*freeVal)(void*));


/**
 * @function genListFree
 * @brief Distrugge una lista generica
 *
 * @param toDestroy il puntatore alla lista da liberare
 * @return
 */
void genListFree(genList * toDestroy);


/**
 * @function get
 * @brief Prende un elemento dalla cima
 *
 * @param list la lista dalla quale prelevare
 * @return l'elemento prelevato, NULL se la lista è vuota
 */
void* get(genList*list);

/**
 * @function put
 * @brief Inserisce un elemento in coda
 *
 * @param list la lista nella quale inserire
 * @param elem elemento da inserire
 *
 * @return 0 in caos di successo, -1 in caso di errore
 */
int put(genList*list, void* elem);

/**
 * @function contains
 * @brief Restituisce un elemento che corrisponde alla chiave passata, se esso è presente, altrimenti NULL
 * 
 * @param list la lista dalla quale prelevare
 * @param key la chiave da ricercare
 * @return l'elemento prelevato oppure NULL, se non presente
 */
void* contains(genList *list, void * key);

/**
 * @function remove
 * @brief Rimove dalla lista l'elemento con una certa chiave
 *
 * @param list la lista dalla quale rimuovere
 * @param key la chiave da rimuovere
 * @return 0 in caso di successo, -1 in caso di errore
 */
int gl_remove(genList*list, void * key);

#endif




