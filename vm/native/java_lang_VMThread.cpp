/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * java.lang.VMThread
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"
/* valera begin */
#include <valera/valera.h>
/* valera end */


/*
 * static void create(Thread t, long stacksize)
 *
 * This is eventually called as a result of Thread.start().
 *
 * Throws an exception on failure.
 */
static void Dalvik_java_lang_VMThread_create(const u4* args, JValue* pResult)
{
    Object* threadObj = (Object*) args[0];
    s8 stackSize = GET_ARG_LONG(args, 1);

    /* copying collector will pin threadObj for us since it was an argument */
    dvmCreateInterpThread(threadObj, (int) stackSize);
    RETURN_VOID();
}

/*
 * static Thread currentThread()
 */
static void Dalvik_java_lang_VMThread_currentThread(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    RETURN_PTR(dvmThreadSelf()->threadObj);
}

/*
 * void getStatus()
 *
 * Gets the Thread status. Result is in VM terms, has to be mapped to
 * Thread.State by interpreted code.
 */
static void Dalvik_java_lang_VMThread_getStatus(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    Thread* thread;
    int result;

    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    if (thread != NULL)
        result = thread->status;
    else
        result = THREAD_ZOMBIE;     // assume it used to exist and is now gone
    dvmUnlockThreadList();

    RETURN_INT(result);
}

/*
 * boolean holdsLock(Object object)
 *
 * Returns whether the current thread has a monitor lock on the specific
 * object.
 */
static void Dalvik_java_lang_VMThread_holdsLock(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    Object* object = (Object*) args[1];
    Thread* thread;

    if (object == NULL) {
        dvmThrowNullPointerException("object == null");
        RETURN_VOID();
    }

    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    int result = dvmHoldsLock(thread, object);
    dvmUnlockThreadList();

    RETURN_BOOLEAN(result);
}

/*
 * void interrupt()
 *
 * Interrupt a thread that is waiting (or is about to wait) on a monitor.
 */
static void Dalvik_java_lang_VMThread_interrupt(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    Thread* thread;

    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    if (thread != NULL)
        dvmThreadInterrupt(thread);
    dvmUnlockThreadList();
    RETURN_VOID();
}

/*
 * static boolean interrupted()
 *
 * Determine if the current thread has been interrupted.  Clears the flag.
 */
static void Dalvik_java_lang_VMThread_interrupted(const u4* args,
    JValue* pResult)
{
    Thread* self = dvmThreadSelf();
    bool interrupted;

    UNUSED_PARAMETER(args);

    interrupted = self->interrupted;
    self->interrupted = false;
    RETURN_BOOLEAN(interrupted);
}

/*
 * boolean isInterrupted()
 *
 * Determine if the specified thread has been interrupted.  Does not clear
 * the flag.
 */
static void Dalvik_java_lang_VMThread_isInterrupted(const u4* args,
    JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    Thread* thread;
    bool interrupted;

    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    if (thread != NULL)
        interrupted = thread->interrupted;
    else
        interrupted = false;
    dvmUnlockThreadList();

    RETURN_BOOLEAN(interrupted);
}

/*
 * void nameChanged(String newName)
 *
 * The name of the target thread has changed.  We may need to alert DDMS.
 */
static void Dalvik_java_lang_VMThread_nameChanged(const u4* args,
    JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    StringObject* nameStr = (StringObject*) args[1];
    Thread* thread;
    int threadId = -1;

    /* get the thread's ID */
    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    if (thread != NULL)
        threadId = thread->threadId;
    dvmUnlockThreadList();

    dvmDdmSendThreadNameChange(threadId, nameStr);
    //char* str = dvmCreateCstrFromString(nameStr);
    //ALOGI("UPDATE: threadid=%d now '%s'", threadId, str);
    //free(str);

    RETURN_VOID();
}

/*
 * void setPriority(int newPriority)
 *
 * Alter the priority of the specified thread.  "newPriority" will range
 * from Thread.MIN_PRIORITY to Thread.MAX_PRIORITY (1-10), with "normal"
 * threads at Thread.NORM_PRIORITY (5).
 */
static void Dalvik_java_lang_VMThread_setPriority(const u4* args,
    JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    int newPriority = args[1];
    Thread* thread;

    dvmLockThreadList(NULL);
    thread = dvmGetThreadFromThreadObject(thisPtr);
    if (thread != NULL)
        dvmChangeThreadPriority(thread, newPriority);
    //dvmDumpAllThreads(false);
    dvmUnlockThreadList();

    RETURN_VOID();
}

/*
 * static void sleep(long msec, int nsec)
 */
static void Dalvik_java_lang_VMThread_sleep(const u4* args, JValue* pResult)
{
    dvmThreadSleep(GET_ARG_LONG(args,0), args[2]);
    RETURN_VOID();
}

/*
 * public void yield()
 *
 * Causes the thread to temporarily pause and allow other threads to execute.
 *
 * The exact behavior is poorly defined.  Some discussion here:
 *   http://www.cs.umd.edu/~pugh/java/memoryModel/archive/0944.html
 */
static void Dalvik_java_lang_VMThread_yield(const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

    sched_yield();

    RETURN_VOID();
}


/* valera begin */
static void Dalvik_java_lang_VMThread_valeraIsEnabled(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    RETURN_BOOLEAN(gDvm.valeraIsEnabled == true);
}

static void Dalvik_java_lang_VMThread_valeraGetMode(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    RETURN_INT(gDvm.valeraMode);
}

static void Dalvik_java_lang_VMThread_valeraPackageName(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    StringObject *strobj = NULL;

    if (gDvm.valeraIsEnabled)
        strobj = dvmCreateStringFromCstr(gDvm.valeraPkgName.c_str());

    RETURN_PTR(strobj);
}

static void Dalvik_java_lang_VMThread_valeraAttachQ(const u4* args,
    JValue* pResult)
{
    int hashCode = args[0];

    valeraAttachQ(hashCode);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraPostMessage(const u4* args,
    JValue* pResult)
{
    StringObject *info = (StringObject *)args[0];

    valeraPostMessage(info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraActionBegin(const u4* args,
    JValue* pResult)
{
    StringObject *info = (StringObject *)args[0];

    valeraActionBegin(info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraActionEnd(const u4* args,
    JValue* pResult)
{
    StringObject *info = (StringObject *)args[0];

    valeraActionEnd(info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraInputEventBegin(const u4* args,
    JValue* pResult)
{
    int seq = args[0];
    StringObject *info = (StringObject *)args[1];

    valeraInputEventBegin(seq, info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraInputEventEnd(const u4* args,
    JValue* pResult)
{
    int seq = args[0];
    StringObject *info = (StringObject *)args[1];

    valeraInputEventEnd(seq, info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraBinderBegin(const u4* args,
    JValue* pResult)
{
    int code = args[0];
    int dataObj = args[1];
    int replyObj = args[2];
    int flags = args[3];

    valeraBinderBegin(code, dataObj, replyObj, flags);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraBinderEnd(const u4* args,
    JValue* pResult)
{
    int code = args[0];
    int dataObj = args[1];
    int replyObj = args[2];
    int flags = args[3];

    valeraBinderEnd(code, dataObj, replyObj, flags);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraBinderProxyBegin(const u4* args,
    JValue* pResult)
{
    StringObject *info = (StringObject *)args[1];

    valeraBinderProxyBegin(info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraBinderProxyEnd(const u4* args,
    JValue* pResult)
{
    StringObject *info = (StringObject *)args[1];

    valeraBinderProxyEnd(info);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraExitCleanUp(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    valeraExitCleanUp();

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraGetThreadId(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    int vtid = valeraGetThreadId();

    RETURN_INT(vtid);
}

static void Dalvik_java_lang_VMThread_valeraSetTracing(const u4* args,
    JValue* pResult)
{
    bool flag = args[0];

    valeraSetTracing(flag);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraIsTracing(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

    RETURN_BOOLEAN(gDvm.valeraTracing == true);
}

static void Dalvik_java_lang_VMThread_valeraDebugPrint(const u4* args,
    JValue* pResult)
{
    StringObject *msg = (StringObject *)args[0];

    valeraDebugPrint(msg);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraLifecycle(const u4* args,
    JValue* pResult)
{
    StringObject *msg = (StringObject *)args[0];

    valeraLifecycle(msg);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraVsyncBegin(const u4* args,
    JValue* pResult)
{
    int msgId = args[0];

    valeraVsyncBegin(msgId);

    RETURN_VOID();
}

static void Dalvik_java_lang_VMThread_valeraVsyncEnd(const u4* args,
    JValue* pResult)
{
    int msgId = args[0];

    valeraVsyncEnd(msgId);

    RETURN_VOID();
}

/* valera end */

const DalvikNativeMethod dvm_java_lang_VMThread[] = {
    /* valera begin */
    { "valeraIsEnabled",    "()Z",
        Dalvik_java_lang_VMThread_valeraIsEnabled },
    { "valeraGetMode",      "()I",
        Dalvik_java_lang_VMThread_valeraGetMode },
    { "valeraPackageName",  "()Ljava/lang/String;",
        Dalvik_java_lang_VMThread_valeraPackageName },
    { "valeraAttachQ",      "(I)V",
        Dalvik_java_lang_VMThread_valeraAttachQ },
    { "valeraPostMessage",  "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraPostMessage },
    { "valeraActionBegin",  "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraActionBegin },
    { "valeraActionEnd",    "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraActionEnd },
    { "valeraInputEventBegin",  "(ILjava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraInputEventBegin },
    { "valeraInputEventEnd",    "(ILjava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraInputEventEnd },
    { "valeraBinderBegin",  "(IIII)V",
        Dalvik_java_lang_VMThread_valeraBinderBegin },
    { "valeraBinderEnd",    "(IIII)V",
        Dalvik_java_lang_VMThread_valeraBinderEnd },
    { "valeraBinderProxyBegin", "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraBinderProxyBegin },
    { "valeraBinderProxyEnd",   "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraBinderProxyEnd },
    { "valeraExitCleanUp",  "()V",
        Dalvik_java_lang_VMThread_valeraExitCleanUp },
    { "valeraGetThreadId",  "()I",
        Dalvik_java_lang_VMThread_valeraGetThreadId },
    { "valeraSetTracing",   "(Z)V",
        Dalvik_java_lang_VMThread_valeraSetTracing },
    { "valeraIsTracing",    "()Z",
        Dalvik_java_lang_VMThread_valeraIsTracing },
    { "valeraDebugPrint",   "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraDebugPrint },
    { "valeraLifecycle",    "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_valeraLifecycle },
    { "valeraVsyncBegin",   "(I)V",
        Dalvik_java_lang_VMThread_valeraVsyncBegin },
    { "valeraVsyncEnd",     "(I)V",
        Dalvik_java_lang_VMThread_valeraVsyncEnd },
    /* valera end */
    { "create",         "(Ljava/lang/Thread;J)V",
        Dalvik_java_lang_VMThread_create },
    { "currentThread",  "()Ljava/lang/Thread;",
        Dalvik_java_lang_VMThread_currentThread },
    { "getStatus",      "()I",
        Dalvik_java_lang_VMThread_getStatus },
    { "holdsLock",      "(Ljava/lang/Object;)Z",
        Dalvik_java_lang_VMThread_holdsLock },
    { "interrupt",      "()V",
        Dalvik_java_lang_VMThread_interrupt },
    { "interrupted",    "()Z",
        Dalvik_java_lang_VMThread_interrupted },
    { "isInterrupted",  "()Z",
        Dalvik_java_lang_VMThread_isInterrupted },
    { "nameChanged",    "(Ljava/lang/String;)V",
        Dalvik_java_lang_VMThread_nameChanged },
    { "setPriority",    "(I)V",
        Dalvik_java_lang_VMThread_setPriority },
    { "sleep",          "(JI)V",
        Dalvik_java_lang_VMThread_sleep },
    { "yield",          "()V",
        Dalvik_java_lang_VMThread_yield },
    { NULL, NULL, NULL },
};
