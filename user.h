/** \file user.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#ifndef USER_H
#define USER_H
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include"message.h"
#include"genList.h"
struct structUser {
    char * nickname;
    int myUserSocket;
    genList * myHistory;
    int historySize;
   } typedef gUser;


/**
 * @function freeVal
 * @brief Libera un elemento di tipo user
 *
 * @param myMessage messaggio da liberare
 */
void freeValMessage(void* myMessage);

/**
 * @function hasKeyUser
 * @brief Compara un elemento di tipo utente con un nickname
 *
 * @param myUser utente da confrontare
 * @param myNickname nickname con il quale si è registrato
 * @return 1 se user e nickname matchano, 0 altrimenti
 */
int hasKeyUser(void* myUser, void* myNickname);

/**
 * @function userInit
 * @brief Crea un elemento di tipo utente
 *
 * @param nickname nickname con il quale si è registrato
 * @return il puntatore all'elemento o NULL in caso di errore
 */
gUser * userInit(char* nickname);

/**
 * @function userFree
 * @brief Libera un elemento di tipo utente
 *
 * @param user utente da liberare
 */
void userFree(void * user);

/**
 * @function addSocket
 * @brief Aggiunge un socket all'utente
 *
 * @param user utente al quale aggiugnere un messaggio
 * @param sock socket che il quale si è connesso
 */
void addSocket(gUser * user, int sock);

/**
 * @function addMessage
 * @brief Aggiunge un messaggio alla history
 *
 * @param user utente al quale aggiungere un messaggio
 * @param msg puntatore al messaggio da aggiungere
 * @return 0 in caso di successo, -1 in caso di errore
 */
int addMessage(gUser * user, message_t* msg);

/**
 * @function removeMessage
 * @brief Toglie un messaggio in cima alla history
 *
 * @param user utente al quale togliere un messaggio
 * @return 0 in caso di successo, -1 in caso di errore
 */
int removeMessage(gUser * user);

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
int writeToUser(gUser * user, char* sender, op_t type, char* content, int maxSize);
#endif
