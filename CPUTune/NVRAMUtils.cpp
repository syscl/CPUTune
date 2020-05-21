//
//  NVRAMUtils.cpp
//  CPUTune
//
//  Created by Yating Zhou on 3/22/20.
//  Copyright © 2020 syscl. All rights reserved.
//

#include "NVRAMUtils.hpp"

#include "kern_util.hpp"

#include <libkern/version.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IONVRAM.h>
#include <IOKit/IORegistryEntry.h>


bool NVRAMUtils::setKextPanicKey(void) const {
    return this->setProperty(kCPUTUNE_PANIC_KEY, osrelease, static_cast<unsigned int>(strlen(osrelease)));
}

bool NVRAMUtils::clearKextPanicKey() const {
    return this->removeProperty(kCPUTUNE_PANIC_KEY);
}

bool NVRAMUtils::isKextPanicLastBoot() const {
    size_t len = 15;
    char ver[len + 1];
    // Get the entry
    if (0 == this->getProperty(kCPUTUNE_PANIC_KEY, ver, &len)) {
        return false;
    }
    return strncmp(ver, osrelease, len);
}

IODTNVRAM *NVRAMUtils::getNVRAMEntry(void) const {
    IORegistryEntry *entry = IORegistryEntry::fromPath("/options", gIODTPlane);
    if (!entry) {
        myLOG("Failed to get NVRAM entry!");
        return nullptr;
    }
    
    IODTNVRAM *nvram = OSDynamicCast(IODTNVRAM, entry);
    if (!nvram) {
        entry->release();
        myLOG("Failed to cast to IODTNVRAM entry!");
        return nullptr;
    }
    
    return nvram;
}

int NVRAMUtils::getProperty(const char *symbol, void *value, size_t *len) const {
    if (!symbol || !len) {
        return 0;
    }
    
    IODTNVRAM *nvram = this->getNVRAMEntry();
    if (nvram == nullptr) {
        return 0;
    }
    
    size_t vlen = *len;
    *len = 0;
    OSObject *o = nvram->getProperty(symbol);
    if (o == nullptr) {
        myLOG("%s not existed in NVRAM property", symbol);
        nvram->release();
        return 0;
    }
    // Cast the object to OSString/OSData
    if (OSString *s = OSDynamicCast(OSString, o)) {
        *len = static_cast<size_t>(s->getLength() + 1);
        if (!value) {
            nvram->release();
            return 1;
        }
        memcpy(value, s->getCStringNoCopy(), min(*len, vlen));
    } else if (OSData *data = OSDynamicCast(OSData, o)) {
        *len = (size_t)data->getLength();
        if (!value) {
            nvram->release();
            return 1;
        }
        memcpy((void *)value, data->getBytesNoCopy(), min(*len, vlen));
    } else {
        myLOG("Unsupported type in NVRAM property: %s", o->getMetaClass()->getClassName());
        nvram->release();
        return 0;
    }
    nvram->release();
    return 1;
}


int NVRAMUtils::setProperty(const char *symbol, const void *value, size_t len) const {
    if (symbol == nullptr || value == nullptr) {
        return 0;
    }
    
    // Get the symbol as an OSSymbol
    const OSSymbol *sym = OSSymbol::withCStringNoCopy(symbol);
    if (sym == nullptr) {
        return 0;
    }
    // Get the data as an OSData
    OSData *data = OSData::withBytes(value, static_cast<unsigned int>(len));
    if (data == nullptr) {
        sym->release();
        return 0;
    }
    
    // Get the NVRAM registry entry
    IODTNVRAM *nvram = this->getNVRAMEntry();
    if (nvram == nullptr) {
        sym->release();
        data->release();
        return 0;
    }
    
    // Set nvram property
    int ret = nvram->setProperty(sym, data);
    nvram->sync();
    nvram->release();
    data->release();
    sym->release();
    return ret;
}

int NVRAMUtils::removeProperty(const char *symbol) const {
    if (!symbol) {
        return 0;
    }
    
    const OSSymbol *sym = OSSymbol::withCStringNoCopy(symbol);
    if (sym == nullptr) {
        return 0;
    }
    
    // Get the NVRAM registry entry
    IODTNVRAM *nvram = getNVRAMEntry();
    if (nvram == nullptr) {
        sym->release();
        return 0;
    }
    
    // Remove the symbol
    nvram->removeProperty(sym);
    nvram->sync();
    nvram->release();
    sym->release();
    return 1;
}
