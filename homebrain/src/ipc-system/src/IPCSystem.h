/***************************************************************************
 *  IPCSystem.h -
 *
 *  Created: 2019-02-18 18:45:05
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __IPCSystem_H__
#define __IPCSystem_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/prctl.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PATH_SIZE 128
#define ARG_SIZE 64
#define ARG_COUNT 6

typedef struct {
    long what;
    union {
        int argc;
        int result;
    }u;
    char args[ARG_COUNT][ARG_SIZE];
} IPCMessage_t;

enum {
    MSG_COMMON_INFO = 0,
    MSG_SYSTEM_CALL = 1,
};

enum {
    SUCCESS = 0,
    EXEC_QUIT = -1,
    CMD_ERR = -2,
};

#if defined(__ANDROID__)

#define ROOT_DIR     "/data/homebrain"
#define HB_WRITE_KEY ROOT_DIR "/hb_write_fifo"
#define HB_READ_KEY  ROOT_DIR "/hb_read_fifo"

#else

#define ROOT_DIR      "./"
#define HB_WRITE_KEY  0xabcdef01
#define HB_READ_KEY   0xabcdef02

#endif

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* __IPCSystem_H__ */

