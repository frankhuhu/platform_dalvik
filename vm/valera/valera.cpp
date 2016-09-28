#include "valera.h"
#include <zlib.h>
#include <cutils/process_name.h>

//#define TRACE_LIMIT (128 * 1024)
//#define TRACE_CHUNK (140 * 1024)
#define TRACE_LIMIT (256 * 1024)
#define TRACE_CHUNK (300 * 1024)
#define TRACE_BUF_SIZE (4 * 1024)

// Threading
int valeraThreadCount = 0;

// Trace compression
static char sTraceBuf[TRACE_CHUNK];
static unsigned char sZlibOutBuf[TRACE_CHUNK];
static int sTraceCount = 0;
static pthread_mutex_t sTraceLock;
const int TRACE_TAG_SEGMENT = 1;


static void flushTrace() {
    int ret, have;
    z_stream strm;
    FILE* file;
    u8 flushStart, flushEnd;

    flushStart = dvmGetRelativeTimeUsec();

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = sTraceCount;
    strm.avail_out = TRACE_CHUNK;
    strm.next_in = (unsigned char *)sTraceBuf;
    strm.next_out = sZlibOutBuf;

    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    assert(ret == Z_OK);
    ret = deflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);
    have = TRACE_CHUNK - strm.avail_out;
    deflateEnd(&strm);

    file = fopen(gDvm.valeraLogFile.c_str(), "ab");
    assert(file != NULL);
    fwrite(&TRACE_TAG_SEGMENT, sizeof(int), 1, file);
    fwrite(&have, sizeof(int), 1, file);
    fwrite(sZlibOutBuf, 1, have, file);
    fclose(file);

    sTraceCount = 0;
    flushEnd = dvmGetRelativeTimeUsec();

    ALOGV("Valera trace flush time %d ms, size from %d to %d",
            (int)((flushEnd - flushStart) / 1000), sTraceCount, have);
}

static void dumpTrace(const char *fmt, ...) {
    va_list ap;

    dvmLockMutex(&sTraceLock);

    va_start(ap, fmt);
    int res = vsnprintf(sTraceBuf + sTraceCount, TRACE_BUF_SIZE, fmt, ap);
    assert(0 <= res && res < TRACE_BUF_SIZE);
    va_end(ap);

    sTraceCount += res;
    if (sTraceCount >= TRACE_LIMIT)
        flushTrace();

    dvmUnlockMutex(&sTraceLock);
}

static bool isInBlacklist(const char* clazzName) {
    std::vector<std::string> *v = &gDvm.valeraTraceBlacklist;
    for (std::vector<std::string>::iterator it = (*v).begin() ; it != (*v).end(); ++it) {
        const char *s = (*it).c_str();
        if (strncmp(s, clazzName, strlen(s)) == 0)
            return true;
    }
    return false;
}

void valeraTraceInterpEntry(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        //DvmDex* pDvmDex = method->clazz->pDvmDex;
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;
        //const u2* insns = method->insns;
        //const u1* baseAddr = pDvmDex->pDexFile->baseAddr;

        //dumpTrace("%d %d %p %p\n", VALERA_TAG_INTERP_ENTRY, tid, insns, baseAddr);
        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_INTERP_ENTRY, tid, clazzName, mtdName, shorty);
        /*
        dvmLockMutex(&sTraceLock);
        //int res = sprintf(sTraceBuf + sTraceCount, "I %d %s %s %s\n", tid, clazzName, mtdName, shorty);
        int res = sprintf(sTraceBuf + sTraceCount, "I %d %p %p\n", tid, insns, baseAddr);
        sTraceCount += res;
        if (sTraceCount >= TRACE_LIMIT)
            flushTrace();
        dvmUnlockMutex(&sTraceLock);
        //fprintf(gDvm.valeraLogFd, "I %d %s %s %s %p %p\n", tid, clazzName, mtdName, shorty, insns, baseAddr);
        */
    }
}

void valeraTraceInterpExit(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        //DvmDex* pDvmDex = method->clazz->pDvmDex;
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;
        //const u2* insns = method->insns;
        //const u1* baseAddr = pDvmDex->pDexFile->baseAddr;

        //dumpTrace("%d %d %p %p\n", VALERA_TAG_INTERP_EXIT, tid, insns, baseAddr);
        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_INTERP_EXIT, tid, clazzName, mtdName, shorty);
    }
}

void valeraTraceMethodEntry(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        //DvmDex* pDvmDex = method->clazz->pDvmDex;
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;
        //const u2* insns = method->insns;
        //const u1* baseAddr = pDvmDex->pDexFile->baseAddr;

        //dumpTrace("%d %d %p %p\n", VALERA_TAG_METHOD_ENTRY, tid, insns, baseAddr);
        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_METHOD_ENTRY, tid, clazzName, mtdName, shorty);
        /*
        dvmLockMutex(&sTraceLock);
        //int res = sprintf(sTraceBuf + sTraceCount, "E %d %s %s %s\n", tid, clazzName, mtdName, shorty);
        int res = sprintf(sTraceBuf + sTraceCount, "E %d %p %p\n", tid, insns, baseAddr);
        sTraceCount += res;
        if (sTraceCount >= TRACE_LIMIT)
            flushTrace();
        dvmUnlockMutex(&sTraceLock);
        //fprintf(gDvm.valeraLogFd, "E %d %s %s %s %p %p\n", tid, clazzName, mtdName, shorty, insns, baseAddr);
        */
    }
}

void valeraTraceMethodExit(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        //DvmDex* pDvmDex = method->clazz->pDvmDex;
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;
        //const u2* insns = method->insns;
        //const u1* baseAddr = pDvmDex->pDexFile->baseAddr;

        //dumpTrace("%d %d %p %p\n", VALERA_TAG_METHOD_EXIT, tid, insns, baseAddr);
        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_METHOD_EXIT, tid, clazzName, mtdName, shorty);
        /*
        dvmLockMutex(&sTraceLock);
        //int res = sprintf(sTraceBuf + sTraceCount, "X %d\n", tid);
        int res = sprintf(sTraceBuf + sTraceCount, "X %d %p\n", tid, insns);
        sTraceCount += res;
        if (sTraceCount >= TRACE_LIMIT)
            flushTrace();
        dvmUnlockMutex(&sTraceLock);
        //fprintf(gDvm.valeraLogFd, "X %d %s %s %s %p %p\n", tid, clazzName, mtdName, shorty, insns, baseAddr);
        */
    }
}

void valeraTraceNativeEntry(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;

        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_NATIVE_ENTRY, tid, clazzName, mtdName, shorty);
        //dumpTrace("NE %d %s %s %s\n", tid, clazzName, mtdName, shorty);
        /*
        dvmLockMutex(&sTraceLock);
        //int res = sprintf(sTraceBuf + sTraceCount, "NE %d %s %s %s\n", tid, clazzName, mtdName, shorty);
        int res = sprintf(sTraceBuf + sTraceCount, "NE %d %p\n", tid, *method->nativeFunc);
        sTraceCount += res;
        if (sTraceCount >= TRACE_LIMIT)
            flushTrace();
        dvmUnlockMutex(&sTraceLock);
        //fprintf(gDvm.valeraLogFd, "NE %d %s %s %s %p\n", tid, clazzName, mtdName, shorty, *method->nativeFunc);
        */
    }
}

void valeraTraceNativeExit(const Thread* thrd, const Method* method) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;
        const char* clazzName = method->clazz->descriptor;
        const char* mtdName = method->name;
        const char* shorty = method->shorty;

        if (!isInBlacklist(clazzName))
            dumpTrace("%d %d %s %s %s\n", VALERA_TAG_NATIVE_EXIT, tid, clazzName, mtdName, shorty);
        //dumpTrace("NX %d %s %s %s\n", tid, clazzName, mtdName, shorty);
        /*
        dvmLockMutex(&sTraceLock);
        //int res = sprintf(sTraceBuf + sTraceCount, "NX %d\n", tid);
        int res = sprintf(sTraceBuf + sTraceCount, "NX %d %p\n", tid, *method->nativeFunc);
        sTraceCount += res;
        if (sTraceCount >= TRACE_LIMIT)
            flushTrace();
        dvmUnlockMutex(&sTraceLock);
        //fprintf(gDvm.valeraLogFd, "NX %d %s %s %s %p\n", tid, clazzName, mtdName, shorty, *method->nativeFunc);
        */
    }
}

void valeraTraceObjectRW(const Thread* thrd, const Method* curMethod, int pc, Object *obj, InstField* ifield, u4 fieldIdx, int rwType) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;
        const char* clazz = ifield->clazz->descriptor;
        const char* name = ifield->name;
        const int off = ifield->byteOffset;
        //const char* clazz = obj->clazz->descriptor;
        const char rw = rwType == VALERA_RW_READ ? 'R' : 'W';

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %s %s %d %x %d %c\n", VALERA_TAG_OBJECT_RW, tid, pc, clazz, name, off, obj, fieldIdx, rw);
    }
}

void valeraTraceObjectRWQuick(const Thread* thrd, const Method* curMethod, int pc, Object *obj, u4 fieldIdx, int rwType) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;
        const char* clazz = obj->clazz->descriptor;
        const char rw = rwType == VALERA_RW_READ ? 'R' : 'W';

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %s %x %d %c\n", VALERA_TAG_OBJECT_RW_QUICK, tid, pc, clazz, obj, fieldIdx, rw);
    }
}

void valeraTraceStaticRW(const Thread* thrd, const Method* curMethod, int pc, StaticField* sfield, u4 fieldIdx, int rwType) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;
        const char* clazz = sfield->clazz->descriptor;
        const char* name = sfield->name;
        const char rw = rwType == VALERA_RW_READ ? 'R' : 'W';

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %s %s %x %d %c\n", VALERA_TAG_STATIC_RW, tid, pc, clazz, name, sfield, fieldIdx, rw);
    }
}

void valeraTracePackedSwitch(const Thread* thrd, const Method* curMethod, int pc, s4 offset) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %d\n", VALERA_TAG_PACKED_SWITCH, tid, pc, offset);
    }
}

void valeraTraceSparseSwitch(const Thread* thrd, const Method* curMethod, int pc, s4 offset) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %d\n", VALERA_TAG_SPARSE_SWITCH, tid, pc, offset);
    }
}

void valeraTraceIfTest(const Thread* thrd, const Method* curMethod, int pc, int taken) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %d\n", VALERA_TAG_IFTEST, tid, pc, taken);
    }
}

void valeraTraceIfTestZ(const Thread* thrd, const Method* curMethod, int pc, int taken) {
    if (gDvm.valeraTracing || thrd->valeraThreadDoTracing) {
        int tid = thrd->valeraThreadId;

        if (curMethod != NULL && !isInBlacklist(curMethod->clazz->descriptor))
            dumpTrace("%d %d %d %d\n", VALERA_TAG_IFTESTZ, tid, pc, taken);
    }
}

void valeraForkInterpThread(Thread *parent, Thread *child) {
    if (gDvm.valeraIsEnabled) {
        int parentTid = parent->valeraThreadId;
        int childTid = child->valeraThreadId;
        std::string parentName = dvmGetThreadName(parent);
        std::replace(parentName.begin(), parentName.end(), ' ', '_');
        std::string childName = dvmGetThreadName(child);
        std::replace(childName.begin(), childName.end(), ' ', '_');

        dumpTrace("%d %d %s %d %s\n", VALERA_TAG_FORK_THREAD, parentTid, parentName.c_str(),
                childTid, childName.c_str());
        //dumpTrace("FORK %d(%s) => %d(%s)\n", parentTid, parentName.c_str(), childTid, childName.c_str());
    }
}

void valeraAttachQ(int hashCode) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();

        dumpTrace("%d %d %d\n", VALERA_TAG_ATTACH_Q, self->valeraThreadId, hashCode);
        //dumpTrace("AttachQ %d %d\n", self->valeraThreadId, hashCode);
    }
}

void valeraPostMessage(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s\n", VALERA_TAG_POST_MESSAGE, self->valeraThreadId, str);
        free(str);
    }
}

void valeraActionBegin(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s %d\n", VALERA_TAG_ACTION_BEGIN, self->valeraThreadId, 
                str, gDvm.valeraTracing || self->valeraThreadDoTracing);
        free(str);
    }
}

void valeraActionEnd(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s %d\n", VALERA_TAG_ACTION_END, self->valeraThreadId, 
                str, gDvm.valeraTracing || self->valeraThreadDoTracing);
        free(str);
    }
}

void valeraLoadDexLib(const char *filename, DvmDex* pDvmDex) {
    const char *proc_name = get_process_name();
    if (!strcmp(proc_name, "zygote")) {
        ALOGI("Hi, I'm Zygote. I'm loading %s %p", filename, pDvmDex->pDexFile->baseAddr);
        std::string str = std::string("/data/local/zygote.dexload");
        FILE *file = fopen(str.c_str(), "a");
        if (file != NULL) {
            fprintf(file, "%s %p\n", filename, pDvmDex->pDexFile->baseAddr);
            fclose(file);
        }
    }
    if (gDvm.valeraTracing) {
        std::string str = std::string("/data/data/") + gDvm.valeraPkgName + "/valera/valera.dexload";
        FILE *file = fopen(str.c_str(), "a");
        if (file != NULL) {
            fprintf(file, "%s %p\n", filename, pDvmDex->pDexFile->baseAddr);
            fclose(file);
        }
    }
}

void valeraInputEventBegin(int seq, StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %d %s\n", VALERA_TAG_INPUTEVENT_BEGIN, self->valeraThreadId, seq, str);
        free(str);
    }
}

void valeraInputEventEnd(int seq, StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %d %s\n", VALERA_TAG_INPUTEVENT_END, self->valeraThreadId, seq, str);
        free(str);
    }
}

void valeraBinderBegin(int code, int dataObj, int replyObj, int flags) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();

        dumpTrace("%d %d %d %x %x %d\n", VALERA_TAG_BINDER_BEGIN, self->valeraThreadId, 
                code, dataObj, replyObj, flags);
    }
}

void valeraBinderEnd(int code, int dataObj, int replyObj, int flags) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();

        dumpTrace("%d %d %d %x %x %d\n", VALERA_TAG_BINDER_END, self->valeraThreadId, 
                code, dataObj, replyObj, flags);
    }
}

void valeraBinderProxyBegin(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s\n", VALERA_TAG_BINDER_PROXY_BEGIN, self->valeraThreadId, str);
        free(str);
    }
}

void valeraBinderProxyEnd(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s\n", VALERA_TAG_BINDER_PROXY_END, self->valeraThreadId, str);
        free(str);
    }
}

void valeraExitCleanUp() {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        dumpTrace("%d %d\n", VALERA_TAG_SYSTEM_EXIT, self->valeraThreadId);
        gDvm.valeraIsEnabled = false;
        gDvm.valeraTracing = false;

        // Flush the tracing buffer.
        flushTrace();
    }
}

int valeraGetThreadId() {
    Thread *self = dvmThreadSelf();
    return self->valeraThreadId;
}

void valeraSetTracing(bool flag) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        self->valeraThreadDoTracing = flag;
    }
}

void valeraDebugPrint(StringObject* msg) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(msg);
        dumpTrace("%d %d %s\n", VALERA_TAG_DEBUG_PRINT, self->valeraThreadId, str);
        free(str);
    }
}

void valeraLifecycle(StringObject* info) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();
        char *str = dvmCreateCstrFromString(info);
        dumpTrace("%d %d %s\n", VALERA_TAG_LIFECYCLE, self->valeraThreadId, str);
        free(str);
    }
}

void valeraVsyncBegin(int msgId) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();

        dumpTrace("%d %d %d\n", VALERA_TAG_VSYNC_BEGIN, self->valeraThreadId, msgId);
    }
}

void valeraVsyncEnd(int msgId) {
    if (gDvm.valeraIsEnabled) {
        Thread *self = dvmThreadSelf();

        dumpTrace("%d %d %d\n", VALERA_TAG_VSYNC_END, self->valeraThreadId, msgId);
    }
}

