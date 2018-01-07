/** \file connections.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore
*/

#include"connections.h"
#include <message.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<errno.h>

/**
 * @function openConnection
 * @brief Apre una connessione AF_UNIX verso il server 
 *
 * @param path Path del socket AF_UNIX 
 * @param ntimes numero massimo di tentativi di retry
 * @param secs tempo di attesa tra due retry consecutive
 *
 * @return il descrittore associato alla connessione in caso di successo
 *         -1 in caso di errore
 */
int openConnection(char* path, unsigned int ntimes, unsigned int secs) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un server;
    if (sock < 0) {
        return -1;
    }
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, path, sizeof(server.sun_path));
    int i=0;
    for(i=0; i<ntimes; i++) {
        if (connect(sock, (const struct sockaddr *) &server, sizeof(struct sockaddr_un)) == 0) break;
        sleep(secs);
    }
    if(i == ntimes) {
        close(sock);
        return -1;
    }
    return sock;
}

/**
 * @function readAll
 * @brief Legge tutti i byte del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore a dove scrivere
 * @param dim lunghezza da leggere
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readAll(long fd, void *data, int dim) {
    int howMany = 0;
    int curr = -1;
    while (howMany < dim) {
        curr = read(fd, &((char*)data)[howMany], dim-howMany);
        if(curr == -1) {
            if(errno == EAGAIN || errno == EINTR) continue;
            else return -1;
        }
        //Nota:Gestisco la non-scrittura come errore per individuare l'eventualità in cui il client si sia disconnesso
        if(curr == 0) return -1;
        howMany+=curr;
    }
#ifdef DEBUG
    fprintf(stderr, "read: ");
    for (int i=0; i<dim; i++)
        fprintf(stderr, "%02x", ((char*)data)[i]);
    fprintf(stderr, "\n");
#endif
    return 0;
}

/**
 * @function writeAll
 * @brief Scrive tutti i byte del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore a da dove leggere il messaggio da scrivere
 * @param dim lunghezza della cosa da scrivere
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int writeAll(long fd, void *data, int dim) {
#ifdef DEBUG
    fprintf(stderr, "write: ");
    for (int i=0; i<dim; i++)
        fprintf(stderr, "%02x", ((char*)data)[i]);
    fprintf(stderr, "\n");
#endif
    int howMany = 0;
    int curr = -1;
    while (howMany < dim) {
        curr = write(fd, &((char*)data)[howMany], dim-howMany);
        if(curr == -1) {
            if(errno == EAGAIN || errno == EINTR) continue;
            else return -1;
        }
        howMany+=curr;
    }
    return 0;
}

/**
 * @function readHeader
 * @brief Legge l'header del messaggio
 *
 * @param connfd  descrittore della connessione
 * @param hdr     puntatore all'header del messaggio da ricevere
 *
 * @return 1 in caso di successo -1 in caso di errore
 */
int readHeader(long connfd, message_hdr_t *hdr) {
    if(readAll(connfd, hdr, sizeof(message_hdr_t)) == -1) return -1;
    return 1;
}

/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @return 1 in caso di successo -1 in caso di errore
 */
int readData(long fd, message_data_t *data) {
    if(readAll(fd, &data->hdr, sizeof(message_data_hdr_t)) == -1) return -1;
    data->buf = calloc(1, data->hdr.len);
    if(data->buf == NULL) return -1;
    if( readAll(fd, data->buf, data->hdr.len) == -1) {
        free(data->buf);   
        return -1;
    }
    return 1;
}

/**
 * @function readMsg
 * @brief Legge l'intero messaggio
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readMsg(long fd, message_t *msg) {
    if(readHeader(fd, &msg->hdr) == -1) return -1;
    if(readData(fd, &msg->data) == -1) return -1;
    return 0;
}

/**
 * @function sendRequest
 * @brief Invia un messaggio
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int sendRequest(long fd, message_t *msg) {
    if(writeAll(fd, &msg->hdr, sizeof(message_hdr_t)) == -1) return -1;
    if(sendData(fd, &msg->data) == -1) return -1;
    return 0;
}

/**
 * @function sendData
 * @brief Invia il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int sendData(long fd, message_data_t *msg) {
    if(writeAll(fd, &msg->hdr, sizeof(message_data_hdr_t)) == -1) return -1;
    if(writeAll(fd, msg->buf, msg->hdr.len) == -1) return -1;
    return 0;
}
