/** \file messageHandler.c
 *     \author Gemma Martini 532769 
       Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
       originale dell'autore  
     */
#include"config.h"
#include"message.h"
#include"stat.h"
#include"user.h"
#include"conf.h"
#include"server.h"
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
#include<fcntl.h>
#include<libgen.h>


/**
 * @function sendUserList
 * @brief Manda la lista degli utenti online
 *
 * @param currentSocket il socket sul quale è connesso il client
 * @param myself struttura dati del server
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int sendUserList(int currentSocket, serverStuff * myself) {
    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    char * buffer = calloc(myself->myStat->howManyOnline * (MAX_NAME_LENGTH+1), 1);
    gList * curr = myself->onlineList->list;
    for(int i=0; i<myself->myStat->howManyOnline; i++) {
        if(curr == NULL) {
            free(buffer);
            return -1;
        }
        char* nickname = ((gUser*)curr->val)->nickname;
        memcpy(buffer+i*(MAX_NAME_LENGTH + 1), nickname, strlen(nickname));
        curr = curr->next;
    }
    setData(&msg.data, "", buffer, myself->myStat->howManyOnline * (MAX_NAME_LENGTH+1));
    if(sendRequest(currentSocket, &msg) == -1) {
        free(buffer);
        return -1;
    }
    free(buffer);
    return 0;
}
 
/**
 * @function dealWithRegister
 * @brief Alla richiesta di registrazione di un client il server deve:
 *  (1) aggiungere lo username passato, se esso non è già presente
 *  (2) aggiungere un elemento ad usersList
 *  (3) aggiungere un elemento a onlineList
 *  (4) incrementare howManyMembers
 *  (5) incrementare howManyOnline
 *  (6) mandare la lista di tutti gli utenti connessi (scegliere il tipo di messaggio che deve essere uguale alla risposta ad USRLIST_OP) 
 *
 * @param currentSocket il socket sul quale è connesso il client
 * @param myself struttura dati del server
 * @param myUsername username con il quale l'utente vuole registrarsi
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithRegister(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    if(contains(myself->usersList, myUsername) != NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_ALREADY, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    gUser * currUser = userInit(myUsername);
    if(currUser == NULL) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    addSocket(currUser, currentSocket);
    if(put(myself->usersList, currUser) != 0) {
        userFree(currUser);
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    if(put(myself->onlineList, currUser) != 0) {
        //Fallisce solo se l'elemento non esiste
        gl_remove(myself->usersList, currUser->nickname);
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    //Le due funzioni che seguono non danno errori
    addMember(myself->myStat);
    login(myself->myStat);
    addSocket(currUser, currentSocket);    
    if(sendUserList(currentSocket, myself) == -1) {
        gl_remove(myself->onlineList, currUser->nickname);
        gl_remove(myself->usersList, currUser->nickname);
        removeMember(myself->myStat);
        logout(myself->myStat);
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithConnect
 * @brief Alla richiesta di connessione di un client il server deve:
 *  (1) aggiungere un elemento ad onlineList
 *  (2) settare il socket dell'utente
 *  (3) incrementare howManyOnline
 *  (4) mandare la lista di tutti gli utenti connessi (scegliere il tipo di messaggio che deve essere uguale alla risposta ad USRLIST_OP) 
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param myUsername username dell'utente che vuole loggarsi
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithConnect(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    gUser * myUser = contains(myself->usersList, myUsername);
    if(myUser == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    if(contains(myself->onlineList, myUsername) == NULL) {
        if(put(myself->onlineList, myUser) != 0) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        } 
        addSocket(myUser, currentSocket);
        login(myself->myStat);
        if(sendUserList(currentSocket, myself) == -1) {
            gl_remove(myself->onlineList, myUser->nickname);
            removeMember(myself->myStat);
            addSocket(myUser, -1);
            logout(myself->myStat);
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
    }
    else {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_ALREADY, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithPostTxt
 * @brief Alla richiesta di invio messaggio ad un nickname il server deve:
 *  (1) controllare che il nickname esista
 *  (2) inviargli tale messaggio e, in caso di utente online, incrementare howManyMsgDelivered. Se l'utente è offline howManyMsgToDeliver
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param sender username dell'utente che invia
 * @param data contenuto del messaggio
 * @param receiver username dell'utente che riceve
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithPostTxt(int currentSocket, serverStuff* myself, char* sender, char* data, char* receiver) {
    pthread_mutex_lock(&myself->myMutex);
    gUser * myUser = contains(myself->onlineList, sender);
    gUser * recUser = contains(myself->usersList, receiver);
    if(myUser == NULL || recUser == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    if (strlen(data) > myself->myConfig->maxMsgSize) {
        message_t msg;
        setHeader(&msg.hdr, OP_MSG_TOOLONG, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    int ret = writeToUser(recUser, sender, TXT_MESSAGE, data, myself->myConfig->maxHistSize);
    if (ret == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    messageToDeliver(myself->myStat);
    if (ret == 1) {
        messageDelivered(myself->myStat);
    }
    // Rispondi all'utente che ha fatto la richiesta
    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", data, 0);
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithPostTxtAll
 * @brief Alla richiesta di post di un messaggio a tutti gli utenti il server deve:
 *  (1) scorrere tutta la lista di tutti gli utenti registrati e, uno per uno
 *      (a) inviargli tale messaggio
 *      (b) in caso di utente online, incrementare howManyMsgDelivered. Se l'utente è offline howManyMsgToDeliver
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param sender username dell'utente che invia
 * @param data contenuto del messaggio
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithPostTxtAll(int currentSocket, serverStuff* myself, char* sender, char* data) {
    pthread_mutex_lock(&myself->myMutex);
    gUser * myUser = contains(myself->onlineList, sender);
    if(myUser == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    if (strlen(data) > myself->myConfig->maxMsgSize) {
        message_t msg;
        setHeader(&msg.hdr, OP_MSG_TOOLONG, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    gList * recUser;
    int ret;
    for (recUser = myself->usersList->list; recUser != NULL; recUser = recUser->next) {
        ret = writeToUser(recUser->val, sender, TXT_MESSAGE, data, myself->myConfig->maxHistSize);
        if (ret == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        messageToDeliver(myself->myStat);
        if (ret == 1) {
            messageDelivered(myself->myStat);
        }
    }
    // Rispondi all'utente che ha fatto la richiesta
    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", data, 0);
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithPostFile
 * @brief Alla richiesta di invio file ad un nickname il server deve:
 *  (1) controllare che il nickname esista
 *  (2) inviargli tale messaggio e, in caso di utente online, incrementare howManyFileDelivered. Se l'utente è offline howManyFileToDeliver
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param sender username dell'utente che invia
 * @param data contenuto del file
 * @param size dimensione del file
 * @param fname nome del file inviato
 * @param receiver username dell'utente che riceve
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithPostFile(int currentSocket, serverStuff* myself, char* sender, void* data, int size, char* fname, char* receiver) {
    pthread_mutex_lock(&myself->myMutex);
    gUser * myUser = contains(myself->onlineList, sender);
    gUser * recUser = contains(myself->usersList, receiver);
    if(myUser == NULL || recUser == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    if (size > myself->myConfig->maxFileSize*1024) {
        message_t msg;
        setHeader(&msg.hdr, OP_MSG_TOOLONG, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    // Il lock viene rilasciato prima di scrivere il file
    pthread_mutex_unlock(&myself->myMutex);

    fname = basename(fname);
    char* filename = malloc(strlen(myself->myConfig->dirName)+2+strlen(fname));
    if (filename == NULL) return -1;
    strncpy(filename, myself->myConfig->dirName, strlen(myself->myConfig->dirName)+1);
    strncat(filename, "/", 1);
    strncat(filename, fname, strlen(fname)+1);
    // Uso un file descriptor per usare writeAll
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
    	free(filename);
    	return -1;
    }
    if (writeAll(fd, data, size) == -1) {
    	free(filename);
        close(fd);
        return -1;
    }
    if (close(fd) == -1) {
    	free(filename);
    	return -1;
    }

    // Il lock viene riacquisito
    pthread_mutex_lock(&myself->myMutex);
    int ret = writeToUser(
        recUser, sender, FILE_MESSAGE,
        &filename[strlen(myself->myConfig->dirName)+1],
        myself->myConfig->maxHistSize
    );
	free(filename);
    if (ret == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    fileToDeliver(myself->myStat);
    if (ret == 1) {
        fileDelivered(myself->myStat);
    }
    // Rispondi all'utente che ha fatto la richiesta
    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", data, 0);
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithGetFile
 * @brief Alla richiesta di recupero file ricevuto di un client il server deve:
 *  (1) leggere tale file ed inviarlo
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param filename file da leggere
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithGetFile(int currentSocket, serverStuff* myself, char* filename) {
    // Controlla che il nome del file non contenga /
    if (strchr(filename, '/') != NULL) {
        pthread_mutex_lock(&myself->myMutex);
        message_t msg;
        setHeader(&msg.hdr, OP_NO_SUCH_FILE, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }

    char* fname = malloc(strlen(myself->myConfig->dirName)+2+strlen(filename));
    if (fname == NULL) return -1;
    strncpy(fname, myself->myConfig->dirName, strlen(myself->myConfig->dirName)+1);
    strncat(fname, "/", 1);
    strncat(fname, filename, strlen(filename));
    int fd = open(fname, O_RDONLY);
    free(fname);
    if (fd == -1) {
        pthread_mutex_lock(&myself->myMutex);
        message_t msg;
        setHeader(&msg.hdr, OP_NO_SUCH_FILE, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    int file_size = lseek(fd, 0, SEEK_END); // Vai a fine file
    if (file_size == -1) {
        close(fd);
        return -1;
    }
    if (lseek(fd, 0, SEEK_SET) == -1) { // Torna a inizio file
        close(fd);
        return -1;
    }
    char* data = malloc(file_size);
    if (data == NULL) {
        close(fd);
        return -1;
    }
    if (readAll(fd, data, file_size) == -1) {
        close(fd);
        free(data);
        return -1;
    }
    if (close(fd) == -1) {
        free(data);
        return -1;
    }
    pthread_mutex_lock(&myself->myMutex);
    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", data, file_size);
    if(sendRequest(currentSocket, &msg) == -1) {
        free(data);
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    free(data);
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithPrevMsgs
 * @brief Alla richiesta di recupero di tutti i messaggi ricevuti di un client il server deve:
 *  (1) scorrere i vari messaggi ed inviarli
 *  (2) incrementare howManyDelivered del numero dei messaggi inviati
 *  (3) decrementare howManyToDeliver del numero dei messaggi inviati 
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param myUsername username dell'utente
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithGetPrevMsgs(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    gUser * myUser = contains(myself->onlineList, myUsername);
    if(myUser == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }

    message_t msg;
    size_t histSize = myUser->historySize;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", (const char*)&histSize, sizeof(histSize));
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    message_t* curMsg;
    while ((curMsg = get(myUser->myHistory)) != NULL) {
        if (sendRequest(currentSocket, curMsg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
	        freeMessage(curMsg);
            return -1;
        }
        if (curMsg->hdr.op == TXT_MESSAGE) {
            messageDelivered(myself->myStat);
        } else {
            fileDelivered(myself->myStat);
        }
        freeMessage(curMsg);
    }
    myUser->historySize = 0;
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithUsrList
 * @brief Alla richiesta di della lista di tutti gli utenti connessi il server deve:
 *  (1) inviare la lista di tutti gli utenti online
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param myUsername username dell'utente
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithUsrList(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    if(contains(myself->onlineList, myUsername) == NULL) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    if(sendUserList(currentSocket, myself) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithUnregister
 * @brief Alla richiesta di deregistrazione di un client il server deve:
 *  (1) rimuovere un elemento da onlineList
 *  (2) decrementare howManyOnline
 *  (3) rimuovere un elemento da usersList 
 *  (4) decrementare howManyMembers
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param myUsername username dell'utente che vuole deregistrarsi
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithUnregister(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    // Rimuove l'utente dagli utenti online
    if (gl_remove(myself->onlineList, myUsername) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    // Elimina l'utente
    gl_remove(myself->usersList, myUsername);
    logout(myself->myStat);
    removeMember(myself->myStat);

    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", "", 0);
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithDisconnect
 * @brief Alla richiesta di disconnessione di un client il server deve:
 *  (1) rimuovere un elemento ad onlineList
 *  (2) decrementare howManyOnline 
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 * @param myUsername username dell'utente che vuole disconnettersi
 *
 * @return 0 in caso di successo, -1 in caso di errore server-side
 */
int dealWithDisconnect(int currentSocket, serverStuff * myself, char * myUsername) {
    pthread_mutex_lock(&myself->myMutex);
    // Rimuove l'utente dagli utenti online
    if (gl_remove(myself->onlineList, myUsername) == -1) {
        message_t msg;
        setHeader(&msg.hdr, OP_NICK_UNKNOWN, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        pthread_mutex_unlock(&myself->myMutex);
        if(sendRequest(currentSocket, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            return -1;
        }
        pthread_mutex_unlock(&myself->myMutex);
        return 0;
    }
    logout(myself->myStat);

    message_t msg;
    setHeader(&msg.hdr, OP_OK, "");
    setData(&msg.data, "", "", 0);
    if(sendRequest(currentSocket, &msg) == -1) {
        pthread_mutex_unlock(&myself->myMutex);
        return -1;
    }
    pthread_mutex_unlock(&myself->myMutex);
    return 0;
}

/**
 * @function dealWithHangUp
 * @brief Gestisce la disconnessione improvvisa di un client, analogamente a dealWithDisconenct
 *
 * @param currentSocket socket della connessione
 * @param myself struttura dati del server
 */
void dealWithHangUp(int currentSocket, serverStuff* myself) {
    pthread_mutex_lock(&myself->myMutex);
    gList* maybeToDisconnect;
    for (maybeToDisconnect = myself->onlineList->list; maybeToDisconnect != NULL; maybeToDisconnect = maybeToDisconnect->next) {
        if (((gUser*)maybeToDisconnect->val)->myUserSocket == currentSocket) {
            addSocket((gUser*)maybeToDisconnect->val, -1);
            gl_remove(myself->onlineList, ((gUser*)maybeToDisconnect->val)->nickname);
            logout(myself->myStat);
            break;
        }
    }
    pthread_mutex_unlock(&myself->myMutex);
}

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
int* readAndDealWithMessage(int* currentSocket, serverStuff* myself) {
    int sock = *currentSocket;
    message_t msg;
    message_data_t content; // Per POSTFILE_OP
    if (readMsg(sock, &msg) == -1) {
        free(currentSocket);
        dealWithHangUp(sock, myself);
        close(sock);
        return NULL;
    }
    int ret = 0;
    switch (msg.hdr.op) {
    case REGISTER_OP:
        ret = dealWithRegister(sock, myself, msg.hdr.sender);
        break;
    case CONNECT_OP:
        ret = dealWithConnect(sock, myself, msg.hdr.sender);
        break;
    case POSTTXT_OP:
        ret = dealWithPostTxt(sock, myself, msg.hdr.sender, msg.data.buf, msg.data.hdr.receiver);
        break;
    case POSTTXTALL_OP:
        ret = dealWithPostTxtAll(sock, myself, msg.hdr.sender, msg.data.buf);
        break;
    case POSTFILE_OP:
        // POSTFILE è speciale e ha due body
        if (readData(sock, &content) == -1) {
            free(currentSocket);
            free(msg.data.buf);
            dealWithHangUp(sock, myself);
            close(sock);
            return NULL;
        }
        ret = dealWithPostFile(sock, myself, msg.hdr.sender, content.buf, content.hdr.len, msg.data.buf, msg.data.hdr.receiver);
        free(content.buf);
        break;
    case GETFILE_OP:
        ret = dealWithGetFile(sock, myself, msg.data.buf);
        break;
    case GETPREVMSGS_OP:
        ret = dealWithGetPrevMsgs(sock, myself, msg.hdr.sender);
        break;
    case USRLIST_OP:
        ret = dealWithUsrList(sock, myself, msg.hdr.sender);
        break;
    case UNREGISTER_OP:
        ret = dealWithUnregister(sock, myself, msg.hdr.sender);
        break;
    case DISCONNECT_OP:
        ret = dealWithDisconnect(sock, myself, msg.hdr.sender);
        break;
    default:
        ret = -1;
    }
    free(msg.data.buf);
    if (ret == -1) {
        free(currentSocket);
        pthread_mutex_lock(&myself->myMutex);
        message_t msg;
        setHeader(&msg.hdr, OP_FAIL, "");
        setData(&msg.data, "", "", 0);
        errorMessage(myself->myStat);
        if(sendRequest(sock, &msg) == -1) {
            pthread_mutex_unlock(&myself->myMutex);
            dealWithHangUp(sock, myself);
            close(sock);
            return NULL;
        }
        pthread_mutex_unlock(&myself->myMutex);
        dealWithHangUp(sock, myself);
        close(sock);
        return NULL;
    }
    return currentSocket;
}
