/*
 * Faraz Heravi
 * server.c
 */

#include "server.h"
#include "queue.h"
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <string.h>
#include <pthread.h>
#include <curses.h>
#include <ctype.h>

#define WORKERS 10
#define DELIMS " \"\'?!.,"
#define PORT 12345

// global variables
char **dictionary;
int dictionaryLength = 0;

// queues
queue *workQueue;
queue *logQueue;

// mutexes and condition variables
pthread_mutex_t workMutex, logMutex;
pthread_cond_t workSignal, logSignal;

// messages
char* clientMessage = "Hello! Welcome to Spell Checker.\n";
char* msgRequest = "Send me some words and I'll tell you if they are spelled correctly!\nSend the \"+\" key to close the connection.\n";
char* msgPrompt = ">>>";
char* msgError = "I didn't get your message. ):\n";
char* msgClose = "Goodbye!\n";

// creates a data structure of a given dictionary
int makeDict(FILE *dict) {

    // original size
    int dictSize = 10000;
    dictionary = (char **) malloc(dictSize * sizeof(char *));

    // word buffer
    // Assuming the longest word is 100 characters
    char word[100];
    int index = 0;
    while (fscanf(dict, "%s", word) > 0) {

        // realloc memory for array
        if (index == dictSize) {
            dictSize += 1000;
            dictionary = realloc(dictionary, dictSize * sizeof(char *));
        }
        // Copy the word to the dictionary
        dictionary[index] = malloc(sizeof(word));
        strcpy(dictionary[index], word);
        index++;
    }

    fclose(dict);
    return index;
}


// convert a word into all lower case
char *lowerCase(char *word) {
    int i;
    char *lower = (char *) malloc(sizeof(word));
    for(i = 0; word[i] != '\0'; ++i) {
        lower[i] = (char) tolower(word[i]);
    }
    lower[i] = '\0';
    return lower;
}

// searches dictionary
int search(char *word) {
    for (int i = 0; dictionary[i] != NULL; ++i) {
        if (strcmp(lowerCase(dictionary[i]), lowerCase(word)) == 0) {
            return 1;
        }
    }

    return 0;
}

// logs the results into a text file
void logging(const char* result) {
    // Lock the Following Code
    pthread_mutex_lock(&logMutex);

    // Open Log File as Append
    FILE* logFile = fopen("log.txt", "a");
    // Write to File
    fprintf(logFile, "%s\n", result);
    // Close File
    fclose(logFile);

    // Unlock
    pthread_mutex_unlock(&logMutex);
    
}

// main worker thread which gets words and checks them
void *worker (void *arg) {
    int bytesReturned;
    char recvBuffer[BUF_LEN];
    recvBuffer[0] = '\0';

    // lock while removing from queue
    pthread_mutex_lock(&workMutex);
    // producer consumer problem (make sure queue is not empty)
    while(empty(workQueue)) {
        pthread_cond_wait(&workSignal, &workMutex);
    }
    // dequeue socket
    int *socketThread = (int *) dequeue(workQueue);

    // unlock
    pthread_mutex_unlock(&workMutex);

    while(1) {

    	// send prompt
	 	send(*socketThread, msgPrompt, strlen(msgPrompt), 0);

	 	// accept bytes from the client
        bytesReturned = recv(*socketThread, recvBuffer, BUF_LEN, 0);

      	// error if -1 is returned
        if(bytesReturned == -1){
            send(*socketThread, msgError, strlen(msgError), 0);
        }
            //'43' is the "+" key.
        else if(recvBuffer[0] == 43){
            printf("Connection %d is disconnected from server.\n", *socketThread);
            send(*socketThread, msgClose, strlen(msgClose), 0);
            close(*socketThread);

            // lock
            pthread_mutex_lock(&workMutex);
            // make sure queue is not empty
            while(empty(workQueue)) {
                pthread_cond_wait(&workSignal, &workMutex);
            }
            // dequeue socket
            socketThread = (int *) dequeue(workQueue);
            send(*socketThread, msgPrompt, strlen(msgPrompt), 0);
            // unlock
            pthread_mutex_unlock(&workMutex);

        }

        // if the buffer actually receives a sentence
        else {

            recvBuffer[bytesReturned-2] = '\0';
            // tokenize by word
            char *words = strtok(recvBuffer, DELIMS);

            while(words != NULL) {

                printf("%s ", words);
                char *result = malloc(sizeof(char) * BUF_LEN);

               // if the word is found in the dictionary
                if(search(words)) {
                    sprintf(result, "\"%s\" is a word\n", words);

                    // if the word is not found
                } else {
                    sprintf(result, "\"%s\" is not a word\n", words);
                }

                // print the message in client's side
                send(*socketThread, result, strlen(result), 0);

                logging(result);
                words = strtok(NULL, DELIMS);

                free(result);
            }

            puts("");
        }

    }

    return NULL;
}


//main
int main(int argc, char** argv) {

    // initialize queues
    workQueue = createQueue();
    logQueue = createQueue();

    // threads
    pthread_t workers[WORKERS];
    pthread_t logger;

    // Initialize mutexes and condition variables
    pthread_mutex_init(&workMutex, NULL);
    pthread_cond_init(&workSignal, NULL);

    pthread_mutex_init(&logMutex, NULL);
    pthread_cond_init(&logSignal, NULL);

    // read dictionary
    FILE *dict;
    if(argc == 1) {
        printf("No port number entered.\n");
        return -1;
        // default dictionary

    } else if(argc == 2) {
        if((dict=fopen("dictionaries/words.txt", "r")) == NULL) {
            puts("ERROR: opening default dictionary");
            exit(1);
        }

        puts("Default dictionary has been opened.");

        // specified dictionary
    } else if(argc == 3) {

    	// guides to dictionaries directory
    	char dir[BUF_LEN] = "dictionaries/"; // buffer
    	strcat(dir, argv[2]);
        dict = fopen(dir, "r");

        // if could not find specified dictionary try opening default dictionary
        if(dict == NULL) {
            puts("ERROR: opening chosen dictionary");
            if((dict=fopen("dictionaries/words.txt", "r")) == NULL) {
                puts("ERROR: opening default dictionary");
                exit(1);
            }
            puts("Default dictionary has been opened.");
        } else {
            puts("Chosen dictionary has been opened.");
        }
    }


    // puts dictionary into a data structure for future thread use
    dictionaryLength = makeDict(dict);

    // create threads
    for(int i = 0; i < WORKERS; ++i) {
        pthread_create((&workers[i]), NULL, worker, NULL);
    }


    //sockaddr_in holds information about the user connection.
    //We don't need it, but it needs to be passed into accept().
    struct sockaddr_in client;
    int clientLen = sizeof(client);
    int connectionSocket, bytesReturned;
    int *clientSocket;
    int connectionPort = atoi(argv[1]);
    char recvBuffer[BUF_LEN];
    recvBuffer[0] = '\0';



    //We can't use ports below 1024 and ports above 65535 don't exist.
    if(connectionPort < 1024 || connectionPort > 65535){
        printf("Port number is either too low(below 1024), or too high(above 65535).\n");
        return -1;
    }

    //Does all the hard work for us.
    connectionSocket = open_listenfd(connectionPort);
    if(connectionSocket == -1){
        printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
        return -1;
    }



   while(1) {
        clientSocket = (int *)malloc(sizeof(int));
        // take in a client
        if((*clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) < 0) {
            printf("Error connecting to client.\n");
            return -1;
        } else {

        	// welcome messages
            send(*clientSocket, clientMessage, strlen(clientMessage), 0);
            send(*clientSocket, msgRequest, strlen(msgRequest), 0);
        }

        // mutual exclusion to the queue
        pthread_mutex_lock(&workMutex);
        // add client to the queue
        printf("Connection to %d success!\n", *clientSocket);
        enqueue(workQueue, clientSocket);
        // signal condition variable
        pthread_cond_signal(&workSignal);
        // unlock the lock
        pthread_mutex_unlock(&workMutex);

   }

    return 0;
}
