//
// Created by Christian Rhodes on 2/28/22.
//


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define BUFFER_SIZE 32

int doneParent;
int doneChild;

int handlerParent(int signum) {
    //printf("this is handlerParent(): got a signal %d\n", signum);
    doneParent = 1;
    return doneParent;
}

int handlerChild(int signum) {
    //printf("this is handlerChild(): got a signal %d\n", signum);
    doneChild = 1;
    return doneChild;
}

int main(int argc, char *argv[]) {
    pid_t pid;
    int sig;
    int memid;
    int key = IPC_PRIVATE;
    char *ptr;
    char buffer[BUFFER_SIZE];

    pid = getpid();
    doneParent = 0;
    doneChild = 0;

    // sig handler info
    struct sigaction actionParent;
    struct sigaction actionChild;
    memset(&actionParent, 0, sizeof(struct sigaction));
    memset(&actionChild, 0, sizeof(struct sigaction));

    actionParent.sa_handler = handlerParent;
    actionChild.sa_handler = handlerChild;

    sigaction(SIGUSR1, &actionParent, NULL);
    sigaction(SIGUSR2, &actionChild, NULL);

    // establish shared memory
    memid = shmget(key, BUFFER_SIZE, IPC_EXCL | 0666);
    if (memid < 0) {
        printf("shmget() failed\n");
        return(8);
    }

    // fork process
    pid = fork();
    if (pid < 0) {
        printf("fork failed\n");
        return(8);
    }

    // parent process
    if (pid > 0) {
        // parent prints its id
        printf("I am the parent, and my pid is %d\n", getpid());

        // jump to child
        kill(pid, SIGUSR1);
        while (!doneChild);

        // parent establish connection to shared memory
        ptr = (char *) shmat(memid, 0, 0);
        if (ptr == NULL) {
            printf("shmat() failed\n");
            return(8);
        }

        // parent write 'process' to shared memory
        strcpy(buffer, "process");
        printf("Parent is writing '%s' to the shared memory\n", buffer);
        strcpy(ptr, buffer);

        //reset value of doneChild
        doneChild = 0;
        // jump to child
        kill(pid, SIGUSR1);
        while (!doneChild);

        // parent write 'communication' to shared memory
        strcpy(buffer, "communication");
        printf("Parent is writing '%s' to the shared memory\n", buffer);
        strcpy(ptr, buffer);

        // jump to child
        doneChild = 0;
        kill(pid, SIGUSR1);
        while (!doneChild);


        // parent write 'is' to shared memory
        strcpy(buffer, "is");
        printf("Parent is writing '%s' to the shared memory\n", buffer);
        strcpy(ptr, buffer);

        // jump to child
        doneChild = 0;
        kill(pid, SIGUSR1);
        while (!doneChild);


        // parent write 'fun' to shared memory
        strcpy(buffer, "fun");
        printf("Parent is writing '%s' to the shared memory\n", buffer);
        strcpy(ptr, buffer);

        // jump to child
        doneChild = 0;
        kill(pid, SIGUSR1);

        // wait at very end of parent process
        wait(NULL);
    }

    // child process
    else {
        //reset value of doneParent
        doneParent = 0;
        // child prints its id
        printf("I am the child, and my pid is %d\n", getpid());
        // signal to parent so they can write to shared memory
        kill(pid, SIGUSR2);
        // wait for parent process to finish
        while(!doneParent);

        // child establish connection to shared memory
        ptr = (char *) shmat(memid, 0, 0);
        if (ptr == NULL) {
            printf("shmat() in child failed\n");
            return(8);
        }

        // receive notification, read the word, tell parent that the word has been read
        printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

        doneParent = 0;
        // signal to parent
        kill(pid, SIGUSR2);
        // wait for parent process to finish
        while(!doneParent);

        // receive notification, read the word, tell parent that the word has been read
        printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

        doneParent = 0;
        // signal to parent
        kill(pid, SIGUSR2);
        // wait for parent process to finish
        while(!doneParent);


        // receive notification, read the word, tell parent that the word has been read
        printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

        doneParent = 0;
        // signal to parent
        kill(pid, SIGUSR2);
        // wait for parent process to finish
        while(!doneParent);


        // receive notification, read the word, tell parent that the word has been read
        printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

        // finished reading from shared memory
        shmdt(ptr); // detaches shared memory

        //print that child is done
        printf("child is done");
    }

    shmdt(ptr);
    shmctl(memid, IPC_RMID, NULL);

    return 0;
}


