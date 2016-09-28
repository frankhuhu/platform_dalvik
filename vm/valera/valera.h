#ifndef VALERA_H_
#define VALERA_H_

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "Dalvik.h"

#define VALERA_TAG_INTERP_ENTRY         1
#define VALERA_TAG_INTERP_EXIT          2
#define VALERA_TAG_METHOD_ENTRY         3
#define VALERA_TAG_METHOD_EXIT          4
#define VALERA_TAG_NATIVE_ENTRY         5
#define VALERA_TAG_NATIVE_EXIT          6
#define VALERA_TAG_FORK_THREAD          7
#define VALERA_TAG_ATTACH_Q             8
#define VALERA_TAG_POST_MESSAGE         9
#define VALERA_TAG_ACTION_BEGIN         10
#define VALERA_TAG_ACTION_END           11
#define VALERA_TAG_INPUTEVENT_BEGIN     12
#define VALERA_TAG_INPUTEVENT_END       13
#define VALERA_TAG_BINDER_BEGIN         14
#define VALERA_TAG_BINDER_END           15
#define VALERA_TAG_OBJECT_RW            16
#define VALERA_TAG_OBJECT_RW_QUICK      17
#define VALERA_TAG_STATIC_RW            18
#define VALERA_TAG_PACKED_SWITCH        19
#define VALERA_TAG_SPARSE_SWITCH        20
#define VALERA_TAG_IFTEST               21
#define VALERA_TAG_IFTESTZ              22
#define VALERA_TAG_LIFECYCLE            23
#define VALERA_TAG_VSYNC_BEGIN          24
#define VALERA_TAG_VSYNC_END            25
#define VALERA_TAG_BINDER_PROXY_BEGIN   26
#define VALERA_TAG_BINDER_PROXY_END     27
#define VALERA_TAG_DEBUG_PRINT          98
#define VALERA_TAG_SYSTEM_EXIT          99

#define VALERA_MODE_NONE    0
#define VALERA_MODE_RECORD  1
#define VALERA_MODE_REPLAY  2

#define VALERA_RW_READ  1
#define VALERA_RW_WRITE 2

extern int valeraThreadCount;

// Tracing API
void valeraTraceInterpEntry(const Thread* thrd, const Method* method);
void valeraTraceInterpExit(const Thread* thrd, const Method* method);
void valeraTraceMethodEntry(const Thread* thrd, const Method* method);
void valeraTraceMethodExit(const Thread* thrd, const Method* method);
void valeraTraceNativeEntry(const Thread* thrd, const Method* method);
void valeraTraceNativeExit(const Thread* thrd, const Method* method);
void valeraTraceObjectRW(const Thread* thrd, const Method* curMethod, int pc, Object *obj, InstField* ifield, u4 fieldIdx, int rwType);
void valeraTraceObjectRWQuick(const Thread* thrd, const Method* curMethod, int pc, Object* obj, u4 fieldIdx, int rwType);
void valeraTraceStaticRW(const Thread* thrd, const Method* curMethod, int pc, StaticField* sfield, u4 fieldIdx, int rwType);
void valeraTracePackedSwitch(const Thread* thrd, const Method* curMethod, int pc, s4 offset);
void valeraTraceSparseSwitch(const Thread* thrd, const Method* curMethod, int pc, s4 offset);
void valeraTraceIfTest(const Thread* thrd, const Method* curMethod, int pc, int taken);
void valeraTraceIfTestZ(const Thread* thrd, const Method* curMethod, int pc, int taken);

void valeraSetTracing(bool flag);

// Threading API
void valeraForkInterpThread(Thread *parent, Thread *child);
int valeraGetThreadId();

// Event API
void valeraAttachQ(int hashCode);
void valeraPostMessage(StringObject* info);
void valeraActionBegin(StringObject* info);
void valeraActionEnd(StringObject* info);
void valeraInputEventBegin(int seq, StringObject* info);
void valeraInputEventEnd(int seq, StringObject* info);
void valeraBinderBegin(int code, int dataObj, int replyObj, int flags);
void valeraBinderEnd(int code, int dataObj, int replyObj, int flags);
void valeraBinderProxyBegin(StringObject* info);
void valeraBinderProxyEnd(StringObject* info);
void valeraVsyncBegin(int msgId);
void valeraVsyncEnd(int msgId);

// Dex class loader
void valeraLoadDexLib(const char *filename, DvmDex* pDvmDex);

// Exit Clean up
void valeraExitCleanUp();

// Debug Print
void valeraDebugPrint(StringObject* msg);

// Lifecycle
void valeraLifecycle(StringObject* info);

#endif
