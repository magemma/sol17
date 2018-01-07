/** \file user.c 
*     \author Gemma Martini 532769  
  Si dichiara che il contenuto di questo file e' in ogni sua parte opera   
  originale dell'autore   
*/ 
#include<stdio.h> 
#include<stdlib.h> 
#include"message.h"

/**
 * @function freeMessage
 * @brief scrive la struttura dati del messaggio
 *
 * @param msg puntatore al messaggio
 */
void freeMessage(message_t* msg) {
	free(msg->data.buf);
	free(msg);
	return;
}

