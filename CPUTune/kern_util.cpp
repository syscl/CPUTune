//
//  kern_util.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include "kern_util.hpp"
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/fcntl.h>

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

long logFileOffset = 0L;

int writeBufferToFile(const char *path, char *buffer) {
    errno_t err = 0;
    int length = static_cast<int>(strlen(buffer));
    vnode_t vp = NULLVP;
    vfs_context_t ctxt = vfs_context_create(nullptr);
    
    int fmode=O_TRUNC | O_CREAT | FWRITE | O_NOFOLLOW;
    int cmode=S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    
    if (ctxt) {
        if ((err = vnode_open(path, fmode, cmode, VNODE_LOOKUP_NOFOLLOW, &vp, ctxt))) {
            myLOG("writeBufferToFile: vnode_open(%s) failed with error %d!\n", path, err);
        } else {
            if ((err = vnode_isreg(vp)) == VREG) {
                if ((err = vn_rdwr(UIO_WRITE, vp, buffer, length, logFileOffset, UIO_SYSSPACE, IO_NOCACHE|IO_NODELOCKED|IO_UNIT, vfs_context_ucred(ctxt), (int *) 0, vfs_context_proc(ctxt)))) {
                    myLOG("writeBufferToFile: vn_rdwr(%s) failed with error %d!\n", path, err);
                } else {
                    logFileOffset += length;
                }
            } else {
                myLOG("writeBufferToFile: vnode_isreg(%s) failed with error %d!\n", path, err);
            }
            
            if ((err = vnode_close(vp, FWASWRITTEN, ctxt))) {
                myLOG("writeBufferToFile: vnode_close(%s) failed with error %d!\n", path, err);
            }
        }
        vfs_context_rele(ctxt);
    } else {
        myLOG("writeBufferToFile: cannot obtain ctxt!\n");
        err = 0xFFFF;
    }
    
    return err;
}

void cputune_os_log(const char *format, ...) {
    char tmp[1024];
    tmp[0] = '\0';
    va_list va;
    va_start(va, format);
    vsnprintf(tmp, sizeof(tmp), format, va);
    va_end(va);
    
    if (ml_get_interrupts_enabled())
        IOLog("%s", tmp);
    
#if DEBUG
    // Write log to path
    const char *path = "/var/log/cputune.kext.log";
    writeBufferToFile(path, tmp);
#endif /* DEBUG */
    
    if (ml_get_interrupts_enabled() && ADDPR(debugPrintDelay) > 0)
        IOSleep(ADDPR(debugPrintDelay));
}
