/*
 * Faraz Heravi
 * client.c
 */

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <string.h>
#include <pthread.h>
#include <curses.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_NUM 512
#define THREADS 2

char** words;
int wordNum = 0;
int port;
int timeUnit = 0;

// mutex locks
pthread_mutex_t logMutex;

// creates a data structure of a given words
int makeList(FILE *list) {

    // original size
    int listSize = 50;
    words = (char **) malloc(listSize * sizeof(char *));

    // word buffer
    // longest word in English is 45 letters
    char word[46];
    int index = 0;
    while (fscanf(list, "%s", word) > 0) {

        // realloc memory for array
        if (index == listSize) {
            listSize += 50;
            words = realloc(words, listSize * sizeof(char *));
        }
        // Copy the word to the words
        words[index] = malloc(sizeof(word));
        strcpy(words[index], word);
        index++;
    }

    fclose(list);
    return index;
}


// logging function logs the result in a lock
void logging(const char* result) {
    // Lock the Following Code
    pthread_mutex_lock(&logMutex);

    // Open Log File as Append
    FILE* logFile = fopen("clientLog.txt", "a");
    // Write to File
    fprintf(logFile, "%s\n", result);
    // Close File
    fclose(logFile);

    pthread_mutex_unlock(&logMutex);
    // Unlock
}

// thread function which connects to server
void *client() {

    // randomize
    srand(time(NULL));

    // sockets
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return NULL;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
    }

    puts("Connected to server.");

    while(1) {
        // determine a random word for each loop
        int r = rand();

        // results
        char *result = (char *) malloc(sizeof(char) * BUF_NUM);
        int timeSend, timeRecv;

        // send word and receive result
        int bytesReturned;
        int random = r % wordNum;
        send(sock, words[random], strlen(words[random]) + 2, 0);
        ++timeUnit;

        timeSend = timeUnit;;

        // receive from server
        char buffer[BUF_NUM];
        bytesReturned = recv(sock, buffer, 1024, 0);
        ++timeUnit;
        timeRecv = timeUnit;

        // store the received data in buffer
        buffer[bytesReturned - 1] = '\0';
        printf("%s\n", buffer);

        // store all the log into result and send them to log function
        sprintf(result, "Thread id: %d, Time sent: %d, Word requested: %s, Time received: %d, Response: %s\n",
                (int) pthread_self(), timeSend, words[random], timeRecv, buffer);

        logging(result);
        free(result);
    }

}

int main (int argc, char** argv) {

    // threads
    pthread_t clients[THREADS];

    FILE *list;

    // read arguments
    if(argc == 1) {
        printf("No port number entered.\n");
    } else if(argc == 2) {
        if((list=fopen("list.txt", "r")) == NULL) {
            puts("ERROR: opening default list of words.");
            exit(1);
        }

        port = atoi(argv[1]);
        puts("Default list of words has been opened.");

        // specified dictionary
    } else if(argc == 3) {
        list = fopen(argv[2], "r");

        // if could not find specified dictionary try opening default dictionary
        if(list == NULL) {
            puts("ERROR: opening list of words.");
            if((list = fopen("words.txt", "r")) == NULL) {
                puts("ERROR: opening list of words.");
                exit(1);
            }
            puts("Default list of words has been opened.");
        } else {
            puts("Chosen list of words has been opened.");
        }
    }

    // puts all the words in a list and gets the number of them
    wordNum = makeList(list);

    // create threads
    for(int i = 0; i < THREADS; ++i) {
        pthread_create((&clients[i]), NULL, client, NULL);
    }

    // wait for threads to come back
    for (int i=0;i < THREADS;i++) {
        pthread_join(clients[i],NULL);
    }

}
