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

errno_t writeBufferToFile(const char *path, char *buffer) {
    errno_t err = 0;
    int length = static_cast<int>(strlen(buffer));
    vnode_t vp = NULLVP;
    vfs_context_t ctxt = vfs_context_create(nullptr);
    
    int fmode = O_TRUNC | O_CREAT | FWRITE | O_NOFOLLOW;
    int cmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    
    if (ctxt) {
        if ((err = vnode_open(path, fmode, cmode, VNODE_LOOKUP_NOFOLLOW, &vp, ctxt))) {
            LOG("writeBufferToFile: vnode_open(%s) failed with error %d!\n", path, err);
        } else {
            if ((err = vnode_isreg(vp)) == VREG) {
                if ((err = vn_rdwr(UIO_WRITE, vp, buffer, length, logFileOffset, UIO_SYSSPACE, IO_NOCACHE|IO_NODELOCKED|IO_UNIT, vfs_context_ucred(ctxt), (int *) 0, vfs_context_proc(ctxt)))) {
                    LOG("writeBufferToFile: vn_rdwr(%s) failed with error %d!\n", path, err);
                } else {
                    logFileOffset += length;
                }
            } else {
                LOG("writeBufferToFile: vnode_isreg(%s) failed with error %d!\n", path, err);
            }
            
            if ((err = vnode_close(vp, FWASWRITTEN, ctxt))) {
                LOG("writeBufferToFile: vnode_close(%s) failed with error %d!\n", path, err);
            }
        }
        vfs_context_rele(ctxt);
    } else {
        LOG("writeBufferToFile: cannot obtain ctxt!\n");
        err = 0xFFFF;
    }
    
    return err;
}

int readFileData(void *buffer, off_t off, size_t size, vnode_t vnode, vfs_context_t ctxt) {
    uio_t uio = uio_create(1, off, UIO_SYSSPACE, UIO_READ);
    if (!uio) {
        // LOG("readFileData: uio_create returned null!");
        return EINVAL;
    }
    
    // imitate the kernel and read a single page from the file
    int error = uio_addiov(uio, CAST_USER_ADDR_T(buffer), size);
    if (error) {
        // LOG("readFileData: uio_addiov returned error %d!", error);
        return error;
    }
    
    if ((error = VNOP_READ(vnode, uio, 0, ctxt))) {
        return error;
    }
    
    if (uio_resid(uio)) {
        // uio_resid returned non-null
        return EINVAL;
    }
    
    return error;
}

uint8_t *readFileNBytes(const char* path, off_t off, size_t bytes) {
    vnode_t vnode = NULLVP;
    vfs_context_t ctx = vfs_context_create(nullptr);
    
    errno_t err = vnode_lookup(path, 0, &vnode, ctx);
    
    uint8_t *buffer = nullptr;
    if (!err) {
        // get the size of the file
        vnode_attr va;
        VATTR_INIT(&va);
        VATTR_WANTED(&va, va_data_size);
        size_t size = vnode_getattr(vnode, &va, ctx) ? 0 : va.va_data_size;
        
        bytes = min(bytes, size);
        if (bytes > 0) {
            buffer = new uint8_t[bytes + 1];
            if (readFileData(buffer, 0, bytes, vnode, ctx)) {
                // fail to read file
                if (buffer) {
                    delete buffer;
                    buffer = nullptr;
                }
            } else {
                // gurantee null termination
                buffer[bytes] = 0;
            }
        } else {
            // size of the file is empty or bytes is zero
        }
        vnode_put(vnode);
    } else {
        // fail to find file via path
    }
    
    vfs_context_rele(ctx);
    
    return buffer;
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
