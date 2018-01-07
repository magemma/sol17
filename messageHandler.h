/** \file messageHandler.h
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H
#include"config.h"
#include"message.h"
#include"stat.h"
#include"conf.h"
#include"server.h"
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<ctype.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

/**
 * @function dealWithHangUp
 * @brief Gestisce la disconnessione improvvisa di un client, analogamente a dealWithDisconenct
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 */
void dealWithHangUp(int currentSocket, serverStuff* myself);

/**
 * @function readAndDealWithMessage
 * @brief Legge un messaggio dal socket e lo passa alla funzione giusta
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 *
 * @return sock, oppure NULL se ci sono stati errori di comunicazione.
 *      In tal caso il socket viene chiuso
 */
int* readAndDealWithMessage(int* currentSocket, serverStuff* myself);
#endif
