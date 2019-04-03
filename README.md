# C Spell Checker

C Spell Checker is a simple server where users can connect to it on a local host and check the spelling of words or sentences.


## Getting Started
### Prerequisites
**Telnet**
In order to connect to the server, you need a network utility that can read from and write into network connections using TCP. Make sure you install this tool on the client's device. For Linux and macOS, Telnet is recommended.
Follow the steps to install Telnet on your device.
>1.  Make sure you have Homebrew installed on your device. To check type the following command into terminal:
```brew```
If it does not recognize the command, you can install it using the guide from [here](brew.sh).
>2.  After installing Homebrew, you can easily installing Telnet by typing in the following command into the terminal:
```brew install telnet```
### Running the server
**Compile**
Before you run the server, you need to compile the program once you have downloaded the repository. There is a Makefile included with the project in order to reduce the hassle of typing all the file names. Make sure to change the directory of your shell to the project's directory. Afterwards, type in the following command:
```make server```
Note that you only need to compile once unless you delete the executable file.

**Execute**
After compilation, we only need to run the executable by typing the following command:
```./server PORT_NUMBER Dictionary_file```
where ```./server``` is the executable file which can take two arguments, a port number and a dictionary file. The port number can be any integer between *1024* and *65535*. The dictionary file can be any text file where it contains words line by line. See below for more information on the dictionary file.
Note that there is a default port number and dictionary file already defined if you don't define anything. The default port number is *12345* and the default dictionary file is *words.txt* which is included in the *dictionaries* directory.
### Connect to the server
From the client's computer, use Telnet or any utility of your choice in order to connect to the server.  For Telnet, type in the following command into the terminal:
```telnet localhost PORT_NUMBER```
where localhost on most networking systems is *127.0.0.1*. 
Once you run the above command, you should be able to connect to the server and receive the following prompt:
>Hello! Welcome to Spell Checker.
>Send me some words and I'll tell you if they are spelled correctly! 
>Send the "+" key to close the connection.
>\>>>
###  Using the Spell Checker
We are almost done!
Now you can enter words or sentences. The spell checker will check word by word and will tell you if your words are correct. All the responses will be appended into a *log.txt* file.
In order to disconnect from the server just enter the '+' character.

## Dictionary

There is a *dictionaries* directory in the project which contains list of words from different languages. The default dictionary used is *words.txt* but you can use any of the dictionaries as mentioned above. You can also add your dictionary in this directory and use it. Make sure your dictionary file contains words formatted line by line.

**Example dictionary file**
```
word1
word2
word3
...
```
## A Note on Threads
The *server.c* file is a multithreaded program, meaning that it can run workers concurrently. Each worker can serve one client at a time. If the number of clients exceed the number of threads, they have to wait in the queue. Originally, the number of threads defined in *server.c* is *10* threads, but you can change it depending on how powerful your computer is. 
> In order to change the number of threads in *server.c*, change the number of workers in the following line:
> ```#define WORKERS 10```

## Test
A test file  *client.c* is included in the repository. This test file chooses random words from the *list.txt* file and connects to the server, and it keeps sending words to the server. *client.c* also produces a log file, called *clientLog.txt*, of all the words it checked.

## Acknowledgement
Thank you to [JUST WORDS!](http://www.gwicks.net/dictionaries.htm) for providing the dictionaries listed in this project.