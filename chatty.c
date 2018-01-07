/** \file chatty.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include"config.h"
#include"message.h"
#include"stat.h"
#include"conf.h"
#include"server.h"
#include"user.h"
#include"messageHandler.h"
#include"connections.h"
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<ctype.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include<poll.h>
#include<sys/signalfd.h>

/**
 * @function hasKeyFd
 * @brief Confronta due puntatori ad int
 *  
 * @param a puntatore al primo intero
 * @param b puntatore al secondo intero
 * @return 1 se gli interi sono uguali, 0 altrimenti
*/
static int hasKeyFd(void* a, void*b) {
    if(*(int*)a == *(int*)b) return 1;
    else return 0;
}

/**
 * @function init
 * @brief Alloca la memoria per la struttura dati che rappresenta il server
 *  
 * @param confPath percorso al file di configurazione
 * @return myself struttura dati allocata in caso di successo, altrimenti null
*/ 
serverStuff * init(char* confPath) {
    serverStuff * myself = malloc(sizeof(*myself));
    if (myself == NULL) {
        return NULL;
    }
    myself->usersList = genListInit(hasKeyUser, userFree);
    if (myself->usersList == NULL) {
        free(myself);
        return NULL;
    }
    myself->onlineList = genListInit(hasKeyUser, NULL);
    if (myself->onlineList == NULL) {
        genListFree(myself->usersList);
        free(myself);
        return NULL;
    }
    myself->myConfig = confInit(confPath);
    if(myself->myConfig == NULL) {
        genListFree(myself->usersList);
        genListFree(myself->onlineList);
        free(myself);
        return NULL;
    }
    myself->myStat = statInit(myself->myConfig->statFileName);
    if(myself->myStat == NULL) {
        genListFree(myself->onlineList);
        genListFree(myself->usersList);
        confFree(myself->myConfig);
        free(myself);
        return NULL;
    }
    myself->myThreadPool = threadPoolInit(myself->myConfig->threadsInPool);
    if(myself->myThreadPool == NULL) {
        genListFree(myself->onlineList);
        genListFree(myself->usersList);
        confFree(myself->myConfig);
        statFree(myself->myStat);
        free(myself);
        return NULL;
    }
    myself->myServerSocket = 0;
    if(pthread_mutex_init(&myself->myMutex, NULL) != 0) {
        genListFree(myself->onlineList);
        genListFree(myself->usersList);
        confFree(myself->myConfig);
        statFree(myself->myStat);
        threadPoolFree(myself->myThreadPool);
        free(myself);
        return NULL;
    }
    return myself;
}

/**
 * @function onError
 * @brief Funzione di clean up che dealloca tutta la struttura dati del server
 *  
 * @param myself puntatore alla struttura dati da allocare
 *
*/ 
void onError(serverStuff * myself) {
     genListFree(myself->onlineList);
     genListFree(myself->usersList);
     confFree(myself->myConfig);
     statFree(myself->myStat);
     threadPoolFree(myself->myThreadPool);
     pthread_mutex_destroy(&myself->myMutex);
     free(myself);
     return;
}

/**
 * @function startCommunication
 * @brief Crea il socket e gli dà un nome
 *   
 * @param myself puntatore alla struttura dati del server da modificare
 * @return 0 se è andato tutto bene, -1 in caso di errore
*/ 
int startCommunication(serverStuff *myself) {
    struct sockaddr_un serveraddr;
    int res = -1;
    myself->myServerSocket = -1;
    myself->myServerSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (myself->myServerSocket < 0) {
        return -1;
    }
    unlink(myself->myConfig->myPath);
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, myself->myConfig->myPath);
    res = bind(myself->myServerSocket, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_un));
    if (res < 0) {
        return -1;
    }
    res = listen(myself->myServerSocket, 10);
    if (res < 0) {
        return -1;
    }
    printf("Ready for the client to connect().\n");
    return 0;
}

/**
 * @function putSocket
 * @brief inserisce un socket nella lista
 *   
 * @param list lista nella quale aggiungere
 * @param a intero da aggiungere
 * @return 0 se è andato tutto bene, -1 in caso di errore
*/ 
static int putSocket(genList * list, int a) {
    int * val = malloc(sizeof(*val));
    if(val == NULL) return -1;
    *val = a;
    if(put(list, val) == -1) {
        free(val);
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    int err = getopt(argc, argv, "f");
	if (err == -1) {
		perror("Errore in getopt");
		return -1;
	}

    serverStuff * myself = init(argv[optind]);
    if (myself == NULL) {
        perror("Error while creating server struct");
        return -1;
    }
    if (startCommunication(myself) == -1) {
       perror("Error while starting communication");
        onError(myself);
        return -1;
    }
    if(threadPoolStart(myself->myThreadPool, myself) == -1) {
        perror("Error while starting thread pool");
        onError(myself);
        return -1;
    }
    genList * myFdList = genListInit(hasKeyFd, free);
    if(myFdList == NULL) {
        perror("Error while initilizing fd list");
        onError(myself);
        return -1;
    }
    if(putSocket(myFdList, myself->myServerSocket) == -1) {
        perror("Error while adding socket to fdlist");
        onError(myself);
        genListFree(myFdList);
        return -1;
    }
    // Aggiunge il semaforo della coda delle risposte a poll
    if(putSocket(myFdList, myself->myThreadPool->servedQueue->myCV) == -1) {
        perror("Error while adding queue semaphore to fdlist");
        onError(myself);
        genListFree(myFdList);
        return -1;
    }
    // Inizializza signalfd e lo aggiunge a poll
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    int sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
        perror("Error creating signalfd");
        onError(myself);
        genListFree(myFdList);
        return -1;
    }
    maskAllSignals();
    if(putSocket(myFdList, sfd) == -1) {
        perror("Error while adding signalfd to fdlist");
        onError(myself);
        close(sfd);
        genListFree(myFdList);
        return -1;
    }
    int howManySocket = 3;
    int howManyConn = 0;
    while(1) {
        struct pollfd * events = calloc(howManySocket, sizeof(struct pollfd));
        if(events == NULL) {
            perror("Error while allocating array");
            onError(myself);
            close(sfd);
            genListFree(myFdList);
            return -1;
        }
        int howManyBis = howManySocket;
        int i=0;
        gList * curr = myFdList->list;
        for(i=0; i<howManySocket; i++) {
            events[i].fd = *(int*)curr->val;
            curr = curr->next;
            events[i].events = POLLIN;
        }
        if (poll(events, howManySocket, -1) == -1) {
            perror("Error while polling");
            onError(myself);
            free(events);
            close(sfd);
            genListFree(myFdList);
            return -1;
        }
        for(i=0; i<howManyBis; i++) {
            if ((events[i].revents & POLLHUP) || (events[i].revents & POLLERR)) {
                gl_remove(myFdList, &events[i].fd);
                dealWithHangUp(events[i].fd, myself);
                close(events[i].fd);
                howManySocket--;
                howManyConn--;
                continue;
            }
            // Nuovo client
            if(events[i].fd == myself->myServerSocket && (events[i].revents & POLLIN)) {
                int clientSocket = accept(myself->myServerSocket, NULL, NULL);
                if (clientSocket < 0) {
                    perror("Accept error");
                    free(events);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
                if (howManyConn == myself->myConfig->maxConnections) {
                    message_t msg;
                    setHeader(&msg.hdr, OP_FAIL, "");
                    setData(&msg.data, "", "", 0);
                    errorMessage(myself->myStat);
                    if (sendRequest(clientSocket, &msg) == -1) {
                        perror("Send error");
                    }
                    close(clientSocket);
                    continue;
                }
                if(putSocket(myFdList, clientSocket) == -1) {
                    perror("Error while adding socket to fd list");
                    free(events);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
                howManySocket++;
                continue;
            }
            // C'è una risposta dalla thread pool
            if(events[i].fd == myself->myThreadPool->servedQueue->myCV && (events[i].revents & POLLIN)) {
                int* answer = dequeue(myself->myThreadPool->servedQueue);
                if (answer == NULL) {
                    perror("Error while dequeueing");
                    free(events);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
                if(put(myFdList, answer) == -1) {
                    perror("Error while adding socket to fd list");
                    free(answer);
                    free(events);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
                howManySocket++;
                continue;
            }
            // Arrivato un segnale
            if (events[i].fd == sfd && (events[i].revents & POLLIN)) {
                struct signalfd_siginfo fdsi;
                if (read(sfd, &fdsi, sizeof(struct signalfd_siginfo)) == -1) {
                    perror("Error reading from signalfd!");
                    close(sfd);
                    free(events);
                    onError(myself);
                    genListFree(myFdList);
                    return -1;
                }
                int ret = 0;
                switch (fdsi.ssi_signo) {
                    case SIGINT:
                    case SIGTERM:
                        if (threadPoolKindStop(myself->myThreadPool) == -1) {
                            perror("Error stopping the threadpool!");
                            ret = -1;
                        }
                        close(sfd);
                        free(events);
                        onError(myself);
                        genListFree(myFdList);
                        return ret;
                    case SIGQUIT:
                        // Non fa cleanup su SIGQUIT
                        if (threadPoolStop(myself->myThreadPool) == -1) {
                            perror("Error stopping the threadpool!");
                            ret = -1;
                        }
                        return ret;
                    case SIGUSR1:
                        printStats(myself->myStat);
                        break;
                    default:
                        perror("Read unexpected signal");
                        close(sfd);
                        free(events);
                        onError(myself);
                        genListFree(myFdList);
                        return -1;
                }
                continue;
            }
            // Messaggio da un client
            if (events[i].revents & POLLIN) {
                int * myFd = malloc(sizeof(int));
                if(myFd == NULL) {
                    perror("Error while allocating new integer");
                    free(events);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
                *myFd = events[i].fd;
                gl_remove(myFdList, myFd);
                howManySocket--;
                if(enqueue(myself->myThreadPool->toServeQueue, myFd) == -1) {
                    perror("Error while enqueueing fd into toServeQueue");
                    free(events);
                    free(myFd);
                    onError(myself);
                    close(sfd);
                    genListFree(myFdList);
                    return -1;
                }
            }
        }
        free(events);
    }
    genListFree(myFdList);
    onError(myself);
    close(sfd);
    return 0;
}
