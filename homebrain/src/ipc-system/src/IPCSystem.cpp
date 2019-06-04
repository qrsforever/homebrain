/***************************************************************************
 *  IPCSystem.cpp -
 *
 *  Created: 2019-02-18 18:45:49
 *
 *  Copyright QRS
 ****************************************************************************/

#include "IPCSystem.h"
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static pid_t gSmartHombPid = -1;

static int gChnnl_w = -1;
static int gChnnl_r = -1;

void _CatchSignal(int signo)
{
    pid_t pid;
    switch (signo) {
        case SIGCHLD:
            pid = waitpid(-1, NULL, 0);
            if (gSmartHombPid == pid)
                exit(1);
            printf("child process quit:%d\n", pid);
            break;
        case SIGINT:
            exit(0); /* when ctrl + C or subprocess died, the parent process exit. */
    }
}

int _InitChannelIPC()
{
#if defined(__ANDROID__)
    if (access(HB_READ_KEY, F_OK) < 0) {
         if (mkfifo(HB_READ_KEY, 0755) < 0){
            perror("mkfifo");
            return -1;
         }
    }
    if ((gChnnl_w = open(HB_READ_KEY, O_RDWR)) < 0) {
        perror("open write fifo");
        return -1;
    }

    if (access(HB_WRITE_KEY, F_OK) < 0) {
         if (mkfifo(HB_WRITE_KEY, 0755) < 0){
            perror("mkfifo");
            return -1;
         }
    }
    if ((gChnnl_r = open(HB_WRITE_KEY, O_RDWR)) < 0) {
        perror("open read fifo");
        return -1;
    }
#else
    if ((gChnnl_w = msgget(HB_READ_KEY, IPC_CREAT|0660)) < 0) {
        perror("msgget");
        return -1;
    }
    if (msgctl(gChnnl_w, IPC_RMID, NULL) < 0) {
        perror("msgctl");
        return -1;
    }
    if ((gChnnl_w = msgget(HB_READ_KEY, IPC_CREAT|0660)) < 0) {
        perror("msgget");
        return -1;
    }

    if ((gChnnl_r = msgget(HB_WRITE_KEY, IPC_CREAT|0660)) < 0) {
        perror("msgget");
        return -1;
    }
    if (msgctl(gChnnl_r, IPC_RMID, NULL) < 0) {
        perror("msgctl");
        return -1;
    }
    if ((gChnnl_r = msgget(HB_WRITE_KEY, IPC_CREAT|0660)) < 0) {
        perror("msgget");
        return -1;
    }
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, _CatchSignal);
    signal(SIGCHLD, _CatchSignal);

    pid_t pid = -1;
    char root_dir[PATH_SIZE + 1] = { 0 };
    bool manual_smarthb = false;
    int opt;

    strncpy(root_dir, ROOT_DIR, PATH_SIZE);
    while ((opt = getopt(argc, argv, "d:M")) != -1) {
        switch (opt) {
            case 'd':
                if (access(optarg, F_OK) < 0) {
                    perror("root dir not exist");
                    exit(-1);
                }
                strncpy(root_dir, optarg, PATH_SIZE);
                break;
            case 'M':
                manual_smarthb = true;
                break;
            default:
                exit(1);
        }
    }

    if (_InitChannelIPC() < 0) {
        perror("init channel");
        exit(1);
    }

    if (chdir(root_dir) < 0)
        perror("chdir");
    if (!manual_smarthb) {
        pid = fork();
        if (pid < 0) {
            perror("fork\n");
            exit(1);
        } else if (pid == 0) {
            prctl(PR_SET_PDEATHSIG, SIGHUP); /* when parent process died, the subprocess will die */
            // for (int fd = 0; fd < 256; close(fd++));
            execlp("smarthb", "smarthb", 0);
            perror("execlp\n");
            exit(0);
        }
        gSmartHombPid = pid;
    }
    IPCMessage_t msg;
    for(;;) {
        memset(&msg, 0, sizeof(IPCMessage_t));
#if defined(__ANDROID__)
        if (read(gChnnl_r, &msg, sizeof(IPCMessage_t)) < 0) {
#else
        if (msgrcv(gChnnl_r, &msg, sizeof(IPCMessage_t) - sizeof(long), 0, 0) < 0) {
#endif
            if (errno == EINTR)
                continue;
            perror("msgrcv/read");
            exit(1);
        }
        switch (msg.what) {
            case MSG_SYSTEM_CALL:
                if ((pid = fork()) < 0) {
                    if (errno != EINTR) {
                        perror("fock");
                        exit(1);
                    }
                } else if (pid == 0) {
                    prctl(PR_SET_PDEATHSIG, SIGHUP);
                    for (int fd = 0; fd < 256; close(fd++));
                    char *argv[msg.u.argc + 1];
                    int i = 0;
                    for (i = 0; i < msg.u.argc; ++i)
                        argv[i] = msg.args[i];
                    argv[i] = NULL;
                    execvp(argv[0], argv);
                }
                msg.u.result = pid;
                break;
            default:
                msg.u.result = CMD_ERR;
        }
#if defined(__ANDROID__)
        if (write(gChnnl_w, &msg, sizeof(IPCMessage_t)) < 0) {
#else
        if (msgsnd(gChnnl_w, &msg, sizeof(IPCMessage_t) - sizeof(long), 0) < 0) {
#endif
            perror("write/msgsnd");
        }
    }
    return 0;
}
