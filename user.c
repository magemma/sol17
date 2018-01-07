/** \file user.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<pthread.h>
#include"message.h"
#include"genList.h"
#include"user.h"
#include"connections.h"

/**
 * @function freeVal
 * @brief Libera un elemento di tipo user
 *
 * @param myMessage messaggio da liberare
 */
void freeValMessage(void* myMessage) {
    message_t * compilingMessage = (message_t*) myMessage;
    freeMessage(compilingMessage);
    return;
}

/**
 * @function hasKeyUser
 * @brief Compara un elemento di tipo utente con un nickname
 *
 * @param myUser utente da confrontare
 * @param myNickname nickname con il quale si è registrato
 * @return 1 se user e nickname matchano, 0 altrimenti
 */
int hasKeyUser(void* myUser, void* myNickname) {
    gUser * compilingUser = (gUser*) myUser;
    char * compilingNickname = (char*) myNickname;
    if(compilingUser == NULL) return 0;
    if(strlen(compilingNickname) != strlen(compilingUser->nickname)) return 0;
    if(strncmp(compilingUser->nickname, compilingNickname, strlen(compilingNickname)) == 0) return 1;
    return 0;
}

/**
 * @function userInit
 * @brief Crea un elemento di tipo utente
 *
 * @param nickname nickname con il quale si è registrato
 * @return il puntatore all'elemento o NULL in caso di errore
 */
gUser * userInit(char* nickname) {
    gUser * myself = malloc(sizeof(*myself));
    if(myself==NULL) return NULL;
    myself->nickname = malloc(strlen(nickname)+1);
    if(myself->nickname == NULL) {
        free(myself);
        return NULL;
    }
    strncpy(myself->nickname, nickname, strlen(nickname)+1);
    myself->myHistory = genListInit(&hasKeyUser, &freeValMessage);
    myself->myUserSocket = -1;
    if (myself->myHistory == NULL) {
        free(myself->nickname);
        free(myself);
        return NULL;
    }
    myself->historySize = 0;
    return myself;
}

/**
 * @function userFree
 * @brief Libera un elemento di tipo utente
 *
 * @param user utente da liberare
 */
void userFree(void * user) {
    gUser * compilingUser = (gUser*) user;
    if(compilingUser == NULL) return;
    free(compilingUser->nickname); 
    genListFree(compilingUser->myHistory);
    free(compilingUser);
    return;
}

/**
 * @function addSocket
 * @brief Aggiunge un socket all'utente
 *
 * @param user utente al quale aggiugnere un messaggio
 * @param sock socket che il quale si è connesso
 */
void addSocket(gUser * user, int sock) {
    user->myUserSocket = sock;
    return;
}

/**
 * @function addMessage
 * @brief Aggiunge un messaggio alla history
 *
 * @param user utente al quale aggiungere un messaggio
 * @param msg puntatore al messaggio da aggiungere
 * @return 0 in caso di successo, -1 in caso di errore
 */
int addMessage(gUser * user, message_t* msg) {
    if(put(user->myHistory, (void*)msg) == -1) return -1;
    user->historySize++;
    return 0;
}

/**
 * @function removeMessage
 * @brief Toglie un messaggio in cima alla history
 *
 * @param user utente al quale togliere un messaggio
 * @return 0 in caso di successo, -1 in caso di errore
 */
int removeMessage(gUser * user) {
    message_t * toFree = get(user->myHistory);
    if(toFree == NULL) return -1;
    freeMessage(toFree);
    user->historySize--;
    return 0;
}

/**
 * @function writeToUser
 * @brief Scrive ad un utente del sistema
 *
 * @param user utente a cui scrivere
 * @param sender utente che manda il messaggio
 * @param type tipo di messaggio (TXT_MESSAGE o FILE_MESSAGE)
 * @param content contenuto del messaggio
 * @param maxSize numero massimo di messaggi che possono stare nell'history
 * @return 1 in caso di messaggio inviato, 0 in caso di messaggio messo nella history ma non inviato, -1 in caso di errore
 */
int writeToUser(gUser * user, char* sender, op_t type, char* content, int maxSize) {
    message_t* msg = malloc(sizeof(message_t));
    if (msg == NULL) return -1;
    setHeader(&msg->hdr, type, sender);
    setData(&msg->data, user->nickname, strdup(content), strlen(content)+1);
    if(user->myUserSocket != -1) {
        if(sendRequest(user->myUserSocket, msg) == -1) {
            freeMessage(msg);
            return -1;
        }
        freeMessage(msg);
        return 1;
    } else {
        if(user->historySize == maxSize) {
            if(removeMessage(user) == -1) {
                freeMessage(msg);
                return -1;
            }
        }
        if(addMessage(user, msg) == -1) {
            freeMessage(msg);
            return -1;
        }
        // Non devo liberare il messaggio, perchè è nell'history
        return 0;
    }
}

