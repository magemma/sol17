/** \file genList.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include"genList.h"

/**
 * @function genListInit
 * @brief Crea una lista generica
 *
 * @param hasKey il puntatore alla funzione di confronto
 * @param freeVal il puntatore alla funzione per liberare un elemento
 * @return il puntatore alla lista o NULL in caso di errore di malloc
 */
genList * genListInit(int (*hasKey)(void*, void*), void(*freeVal)(void*)) {
    genList* myList = malloc(sizeof(*myList));
    if(myList == NULL) return NULL;
    myList->list = NULL;
    myList->hasKey = hasKey;
    myList->freeVal = freeVal;
    myList->lastNext=&(myList->list);
    return myList;
}


/**
 * @function genListFree
 * @brief Distrugge una lista generica
 *
 * @param toDestroy il puntatore alla lista da liberare
 * @return
 */
void genListFree(genList * toDestroy) {
    if(toDestroy == NULL) return;
    gList * curr;
    while(toDestroy->list!=NULL) {
        curr = toDestroy->list->next;
        if (toDestroy->freeVal != NULL) toDestroy->freeVal(toDestroy->list->val);
        free(toDestroy->list);
        toDestroy->list=curr;
    }
    free(toDestroy);
    return;
}



/**
 * @function get
 * @brief Prende un elemento dalla cima
 *
 * @param list la lista dalla quale prelevare
 * @return l'elemento prelevato, NULL se la lista è vuota
 */
void* get(genList *list) {
    gList * elem;
    void * value;
    if(list->list == NULL) {
        return NULL;
    }
    if(list->list->next == NULL) {
        list->lastNext=&(list->list);
    }
    elem = list->list;
    list->list = list->list->next;
    value = (void*)elem->val;
    free(elem);
    return value;
}

/**
 * @function put
 * @brief Inserisce un elemento in coda
 *
 * @param list la lista nella quale inserire
 * @param elem elemento da inserire
 *
 * @return 0 in caso di successo, -1 in caso di errore
 */
int put(genList *list, void* elem) {
    gList * compileElem = malloc(sizeof(gList));
    if(compileElem == NULL) return -1;
    compileElem->val=elem;
    compileElem->next=NULL;
    *(list->lastNext) = compileElem;
    list->lastNext = &(compileElem->next);
    return 0;
}

/**
 * @function contains
 * @brief Restituisce un elemento che corrisponde alla chiave passata, se esso è presente, altrimenti NULL
 * 
 * @param list la lista dalla quale prelevare
 * @param key la chiave da ricercare
 * @return l'elemento prelevato oppure NULL, se non presente
 */
void* contains(genList *list, void *key) {
    gList * elem = list->list;
    while(elem != NULL) {
        if(list->hasKey(elem->val, key) == 1) {
            return (void*)elem->val;
        }
        elem = elem->next;
    }
    return NULL;
}

/**
 * @function remove
 * @brief Rimove dalla lista l'elemento con una certa chiave
 *
 * @param list la lista dalla quale rimuovere
 * @param key la chiave da rimuovere
 * @return 0 in caso di successo, -1 in caso elemento non presente
 */
int gl_remove(genList *list, void * key) {
    gList * elem = list->list;
    gList * toFree;
    if(elem == NULL) return -1;
    if(list->hasKey(elem->val, key) == 1) {
        toFree=elem;
        if(elem->next==NULL) {
            list->lastNext = &(list->list);
        }
        list->list=elem->next;
        if(list->freeVal != NULL) list->freeVal(toFree->val);
        free(toFree);
        return 0;
    }
    while(elem->next != NULL) {
        if(list->hasKey(elem->next->val, key) == 1) {
            toFree = elem->next;
            if(elem->next->next == NULL) {
                list->lastNext = &(elem->next);
            }
            elem->next=elem->next->next;
            if(list->freeVal != NULL) list->freeVal(toFree->val);
            free(toFree);
            return 0;
        }
        elem = elem->next;
    }
    return -1;
}


