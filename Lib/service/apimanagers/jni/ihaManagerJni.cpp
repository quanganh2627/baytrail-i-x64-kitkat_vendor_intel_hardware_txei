/*
********************************************************************************
**    Intel Architecture Group
**    Copyright (C) 2015 Intel Corporation
********************************************************************************
**
**    INTEL CONFIDENTIAL
**    This file, software, or program is supplied under the terms of a
**    license agreement and/or nondisclosure agreement with Intel Corporation
**    and may not be copied or disclosed except in accordance with the
**    terms of that agreement.  This file, software, or program contains
**    copyrighted material and/or trade secret information of Intel
**    Corporation, and must be treated as such.  Intel reserves all rights
**    in this material, except as the license agreement or nondisclosure
**    agreement specifically indicate.
**
**    All rights reserved.  No part of this program or publication
**    may be reproduced, transmitted, transcribed, stored in a
**    retrieval system, or translated into any language or computer
**    language, in any form or by any means, electronic, mechanical,
**    magnetic, optical, chemical, manual, or otherwise, without
**    the prior written permission of Intel Corporation.
**
**    Intel makes no warranty of any kind regarding this code.  This code
**    is provided on an "As Is" basis and Intel will not provide any support,
**    assistance, installation, training or other services.  Intel does not
**    provide any updates, enhancements or extensions.  Intel specifically
**    disclaims any warranty of merchantability, noninfringement, fitness
**    for any particular purpose, or any other warranty.
**
**    Intel disclaims all liability, including liability for infringement
**    of any proprietary rights, relating to use of the code.  No license,
**    express or implied, by estoppel or otherwise, to any intellectual
**    property rights is granted herein.
**/
/**
********************************************************************************
**
**    @file ihaManagerJni.cpp
**
**    @brief  Defines the exported JNI functions for the IHA DLL.
**
**    @author Ranjit Narjala
**
********************************************************************************
*/

//#define LOG_NDEBUG 1
#define LOG_TAG "IhaManagerJni"

#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <android_runtime/AndroidRuntime.h>
#include <utils/String16.h>
#include <pthread.h>
#include <android/log.h>

#include "debug.h"
#include "ihaError.h"

using namespace android;
#include "ISecurity.h"

#include "com_intel_host_ipt_iha_IhaManagerJni.h"

//namespace com_intel_host_ipt_iha {
#define JNI_LOADERROR -1

static JavaVM *g_vm = NULL;
static jobject g_obj = NULL;

extern bool g_bProdLogEnabled; // defined in iha.cpp

JavaVM *gJvm = NULL;
jobject     gobjCallback = NULL;

#ifdef ANDROID
#define TCHAR wchar_t
#endif

static sp<ISecurity> sServerHandle = NULL;


/******************************************************************************
 * METHOD: IhaManagerJni_CleanupCb()
 *        Cleans up event callback during unregistration, uninstallation and
 *        DeInit.
 *
 * IN   : pEnv: JNI evn pointer
 * IN   : sAppName: applet name, if NULL clear cb for all applets
 * RETURN: none
 ******************************************************************************/
static void IhaManagerJni_CleanupCb(JNIEnv *pEnv, const char *sAppName) {
    if (!pEnv) return;

    // clear the global object reference
    if (gobjCallback) {
        pEnv->DeleteGlobalRef(gobjCallback);
        gobjCallback = NULL;
    }

}

/******************************************************************************
 * METHOD: IhaManagerJni_Callback()
 *        Wrapper callback function to handle embedded app notifications.
 *        This will call the java callback method IhaManagerJniCallback to process
 *        the notification.
 *
 * IN   : sAppName: String identifying the embedded app in the chipset.
 * IN   : dataLen: length of data in event buffer
 * IN   : pData: pointer to event data buffer
 * IN   : dataSrc: 0 - event is from embedded app \n
 *                 1 - event is from embedded FW service
 * RETURN: none
 ******************************************************************************/
static void IhaManagerJni_Callback(char *sAppName,
                                   uint32_t dataLen,
                                   uint8_t *pData,
                                   uint8_t dataSrc) {
    JNIEnv *jenv = NULL;
    jclass objclass;
    jmethodID method;
    jthrowable jexc;

    // expecting the args to be valid at this point
    if (sAppName == NULL) {
        LOGERR("sAppName is NULL\n");
        return;
    }
    if (pData == NULL) {
        LOGERR("pData is NULL");
        return;
    }
    if (g_vm == NULL) {
        LOGERR("VM not initialized\n");
        return;
    }

    if (!g_vm->AttachCurrentThread(&jenv, NULL)) {
        // get the class
        objclass = jenv->GetObjectClass(gobjCallback);
        if (!objclass) {
            LOGERR("callback class not found\n");
            return;
        }
        LOGDBG(" callback class found\n");

        method = jenv->GetMethodID(objclass,
                                   "processNotification",
                                   "(Ljava/lang/String;I[BB)V");
        if (method == NULL) {
            LOGERR("callback method not found\n");
            return;
        }
        LOGDBG("callback method found\n");

        jstring strAppName = jenv->NewStringUTF(sAppName);
        if (NULL == strAppName) {
            LOGERR("string alloc for applet name failed\n");
            return;
        }

        // Allocate jni buffer to hold event data
        jbyteArray jbaData = NULL;
        if (dataLen > 0) {
            jbaData = jenv->NewByteArray((jsize)dataLen);
            if (NULL == jbaData) {
                LOGERR("JNI-IhaJni_Callback: data alloc for event failed\n");
                jenv->ReleaseStringUTFChars(strAppName, sAppName);
                return;
            }
            jenv->SetByteArrayRegion(jbaData, 0, dataLen, (jbyte *)pData);
        }

        // clear existing exceptions
        jenv->ExceptionClear();

        jenv->CallVoidMethod(gobjCallback,
                             method,
                             strAppName,
                             dataLen,
                             jbaData,
                             dataSrc);

        // must handle exceptions if any. The callback method
        // can throw IhaException, for now we will print a
        // description and clear it.
        jexc = jenv->ExceptionOccurred();
        if (jexc) {
            LOGERR("callback exited with exception\n");
            jenv->ExceptionDescribe();
            jenv->ExceptionClear();
        }

        // We are not releasing the java arrays here, they will be garbage collected
        // by the VM when the arrays and strings are no longer referenced by anyone
        //jenv->ReleaseStringUTFChars(strAppName, sAppName);
        //jenv->ReleaseByteArrayElements(jbaData, (jbyte *)pData, JNI_ABORT);
    }

    return;
}

/******************************************************************************
 * METHOD: throwIHAException()
 *        Helper function to throw an OtpIhaJniException
 * IN   : error - value of exception that needs to be thrown
 * OUT  : none
 * RETURN: none
 ******************************************************************************/
void throwIHAException(JNIEnv *pEnv, const uint32_t error) {
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }

    // Find the IhaException class
    jclass jcException = pEnv->FindClass("com/intel/host/ipt/iha/IhaException");
    if (NULL == jcException) {
        LOGERR("Could not find exception class\n");
        return;
    }

    // Find the appropriate constructor within that class
    jmethodID jmConstructor = pEnv->GetMethodID(jcException, "<init>", "(I)V");
    if (NULL == jmConstructor) {
        LOGERR("Could not find constructor\n");
        return;
    }

    // Create and throw the exception
    jobject jobjException = pEnv->NewObject(jcException, jmConstructor, (int)error);
    LOGDBG("Throwing exception... %u\n", error);
    pEnv->Throw((jthrowable)jobjException);
    return;
} // ThrowIHAException


/******************************************************************************
 * JNI version of IHA_Init()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAInit(JNIEnv *pEnv,
                                                  jclass jclazz) {
    ENTER("\n");

    sServerHandle = BpSecurity::getSecurityService();
    if (sServerHandle == NULL) {
        LOGERR("Unable to connect with the server\n");
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return;
    }

    uint32_t ret = sServerHandle->IHAInit();
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return;
    }
    LEAVE("OK\n");
    return;
} // JNI-IHAInit


/******************************************************************************
 * JNI version of IHA_DeInit()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHADeInit(JNIEnv *pEnv,
                                                    jclass jclazz) {
    uint32_t ret = IHA_RET_E_INTERNAL_ERROR;
    ENTER("\n");

    if (sServerHandle != NULL) {
        ret = sServerHandle->IHADeInit();
    }

    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return;
    } else {
        // clear the global object reference
        IhaManagerJni_CleanupCb(pEnv, NULL);

        LEAVE("OK\n");
        return;
    }
} // JNI-IHADeInit


/******************************************************************************
 * JNI version of IHA_StartProvisioning()
 ******************************************************************************/
JNIEXPORT jint JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAStartProvisioning(JNIEnv *pEnv,
                                                               jclass jclazz,
                                                               jstring jstrAppName) {
    uint32_t ret = IHA_RET_E_INTERNAL_ERROR;
    uint32_t hSessionHandle = 0;
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return 0;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return 0;
    }

    if (sServerHandle != NULL) {
        ret = sServerHandle->IHAStartProvisioning(sAppName, &hSessionHandle);
    }

    // release appName
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return 0;
    }
    LEAVE("hSessionHandle=0x%08x\n", hSessionHandle);
    return (jint)hSessionHandle;
} // JNI-IHAStartProvisioning


/******************************************************************************
 * JNI version of IHA_EndProvisioning()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAEndProvisioning(JNIEnv *pEnv,
                                                             jclass jclazz,
                                                             jstring jstrAppName,
                                                             jint jiSessionHandle,
                                                             jshortArray jsaExpectedDataLen) {
    uint32_t ret = IHA_RET_E_INTERNAL_ERROR;
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected out data len
    jshort *pjsExpectedDataLen = pEnv->GetShortArrayElements(jsaExpectedDataLen, NULL);
    if (NULL == pjsExpectedDataLen) {
        LOGERR("GetSAElements for exp data len failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected data Len > 0
    if (pjsExpectedDataLen[0] <= 0) {
        LOGERR("Incorrect expected len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // Allocate output buffer
    jbyteArray jbaEncToken;
    jbaEncToken = pEnv->NewByteArray((jsize)pjsExpectedDataLen[0]);
    jbyte *pjbEncToken = pEnv->GetByteArrayElements(jbaEncToken, NULL);
    if (NULL == pjbEncToken) {
        LOGERR("GetBAElements for encToken failed\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return NULL;
    }

    // call internal IHA function
    if (sServerHandle != NULL) {
        ret = sServerHandle->IHAEndProvisioning(sAppName,
                                                jiSessionHandle,
                                                reinterpret_cast<uint16_t *>(pjsExpectedDataLen),
                                                reinterpret_cast<uint8_t *>(pjbEncToken));
    }

    // release stuff, copying C values back into the Java variables
    pEnv->ReleaseByteArrayElements(jbaEncToken, pjbEncToken, 0);
    pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    //if (IHA_RET_S_OK != ret) {
    //    LOGERR("Need to throw exception\n");
    //    throwIHAException(pEnv, ret);
    //    return NULL;
    //}

    LEAVE("\n");
    return jbaEncToken;

} // JNI-IHAEndProvisioning


/******************************************************************************
 * JNI version of IHA_SendData()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHASendData(JNIEnv *pEnv,
                                                      jclass jclazz,
                                                      jstring jstrAppName,
                                                      jint jiSessionHandle,
                                                      jshort jsDataType,
                                                      jbyteArray jbaInData) {
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // get InData
    jbyte *pjbInData = pEnv->GetByteArrayElements(jbaInData, NULL);
    if (NULL == pjbInData) {
        LOGERR("GetBAElements for inData failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHASendData(sAppName,
                                              jiSessionHandle,
                                              jsDataType,
                                              static_cast<uint16_t>(pEnv->GetArrayLength(jbaInData)),
                                              reinterpret_cast<uint8_t *>(pjbInData));

    // release stuff
    pEnv->ReleaseByteArrayElements(jbaInData, pjbInData, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return;
    }
    LEAVE("\n");
    return;
} // JNI-IHASendData


/******************************************************************************
 * JNI version of IHA_ReceiveData()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAReceiveData(JNIEnv *pEnv,
                                                         jclass jclazz,
                                                         jstring jstrAppName,
                                                         jint jiSessionHandle,
                                                         jshort jsDataType,
                                                         jshortArray jsaExpectedDataLen) {
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("JNI-IHARecvData: GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected out data len
    jshort *pjsExpectedDataLen = pEnv->GetShortArrayElements(jsaExpectedDataLen, NULL);
    if (NULL == pjsExpectedDataLen) {
        LOGERR("GetSAElements for exp data len failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected data Len > 0
    if (pjsExpectedDataLen[0] <= 0) {
        LOGERR("Incorrect expected len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // Allocate output buffer
    jbyteArray jbaOutData = pEnv->NewByteArray((jsize)pjsExpectedDataLen[0]);
    jbyte *pjbOutData = pEnv->GetByteArrayElements(jbaOutData, NULL);
    if (NULL == pjbOutData) {
        LOGERR("JNI-IHARecvData: GetBAElements for outData failed\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return NULL;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAReceiveData(sAppName,
                                                 jiSessionHandle,
                                                 jsDataType,
                                                 reinterpret_cast<uint16_t *>(pjsExpectedDataLen),
                                                 reinterpret_cast<uint8_t *>(pjbOutData));

    // release stuff, copying C values back into the Java variables
    pEnv->ReleaseByteArrayElements(jbaOutData, pjbOutData, 0);
    pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return NULL;
    }
    LEAVE("\n");
    return jbaOutData;
} // JNI-IHAReceiveData


/******************************************************************************
 * JNI version of IHA_ProcessSVPMessage()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAProcessSVPMessage(JNIEnv *pEnv,
                                                               jclass jclazz,
                                                               jstring jstrAppName,
                                                               jint jiSessionHandle,
                                                               jbyteArray jbaInData) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // get InData
    jbyte *pjbInData = pEnv->GetByteArrayElements(jbaInData, NULL);
    if (NULL == pjbInData) {
        LOGERR("GetBAElements for inData failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }
    LOGDBG("Calling Client: %s %d\n", sAppName, jiSessionHandle);

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAProcessSVPMessage(sAppName,
                                                       jiSessionHandle,
                                                       static_cast<uint16_t>(pEnv->GetArrayLength(jbaInData)),
                                                       reinterpret_cast<uint8_t *>(pjbInData));

    LOGDBG("ret=%u\n", ret);

    // release stuff
    pEnv->ReleaseByteArrayElements(jbaInData, pjbInData, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return;
    }

    LEAVE("\n");
    return;
} // JNI-IHAProcessSVPMessage


/******************************************************************************
 * JNI version of IHA_GetSVPMessage()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetSVPMessage(JNIEnv *pEnv,
                                                           jclass jclazz,
                                                           jstring jstrAppName,
                                                           jint jiSessionHandle,
                                                           jshortArray jsaExpectedDataLen) {
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected out data len
    jshort *pjsExpectedDataLen = pEnv->GetShortArrayElements(jsaExpectedDataLen, NULL);
    if (NULL == pjsExpectedDataLen) {
        LOGERR("GetSAElements for exp data len failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected data Len > 0
    if (pjsExpectedDataLen[0] <= 0) {
        LOGERR("Incorrect expected len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // Allocate output buffer
    jbyteArray jbaOutData;
    jbaOutData = pEnv->NewByteArray((jsize)pjsExpectedDataLen[0]);
    jbyte *pjbOutData = pEnv->GetByteArrayElements(jbaOutData, NULL);
    if (NULL == pjbOutData) {
        LOGERR("GetBAElements for outData failed\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return NULL;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAGetSVPMessage(sAppName,
                                                   jiSessionHandle,
                                                   reinterpret_cast<uint16_t *>(pjsExpectedDataLen),
                                                   reinterpret_cast<uint8_t *>(pjbOutData));

    // release stuff, copying C values back into the Java variables
    pEnv->ReleaseByteArrayElements(jbaOutData, pjbOutData, 0);
    pEnv->ReleaseShortArrayElements(jsaExpectedDataLen, pjsExpectedDataLen, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return NULL;
    }

    LEAVE("\n");
    return jbaOutData;

} // JNI-IHAGetSVPMessage


/******************************************************************************
 * JNI version of IHA_GetOTP()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetOTP(JNIEnv *pEnv,
                                                    jclass jclazz,
                                                    jstring jstrAppName,
                                                    jint jiSessionHandle,
                                                    jbyteArray jbaEncToken,
                                                    jbyteArray jbaVendorData,
                                                    jshortArray jsaExpectedOtpLen,
                                                    jshortArray jsaExpectedEncTokenLen) {

    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get EncToken bytes to send
    // may be NULL, so check only if !NULL
    jbyte *pjbEncToken = NULL;
    uint16_t encTokenLen = 0;
    if (jbaEncToken) {
        pjbEncToken = pEnv->GetByteArrayElements(jbaEncToken, NULL);
        if (NULL == pjbEncToken) {
            LOGERR("GetBAElements for encToken failed\n");
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
            return NULL;

        } else {
            encTokenLen = (uint16_t)(pEnv->GetArrayLength(jbaEncToken));
        }
    }

    // get VendorData bytes to send
    // may be NULL, so check only if !NULL
    jbyte *pjbVendorData = NULL;
    uint16_t vendorDataLen = 0;
    if (jbaVendorData) {
        pjbVendorData = pEnv->GetByteArrayElements(jbaVendorData, NULL);
        if (NULL == pjbVendorData) {
            LOGERR("GetBAElements for vendorData failed\n");
            pEnv->ReleaseByteArrayElements(jbaEncToken, pjbEncToken, 0);
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
            return NULL;

        } else {
            vendorDataLen = (uint16_t)(pEnv->GetArrayLength(jbaVendorData));
        }
    }

    // get expected otp len
    jshort *pjsExpectedOtpLen = pEnv->GetShortArrayElements(jsaExpectedOtpLen, NULL);
    if (NULL == pjsExpectedOtpLen) {
        LOGERR("GetSAElements for exp otp len failed\n");
        pEnv->ReleaseByteArrayElements(jbaVendorData, pjbVendorData, 0);
        pEnv->ReleaseByteArrayElements(jbaEncToken, pjbEncToken, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected otp Len > 0
    if (pjsExpectedOtpLen[0] <= 0) {
        LOGERR("Incorrect expected otp len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedOtpLen, pjsExpectedOtpLen, 0);
        pEnv->ReleaseByteArrayElements(jbaVendorData, pjbVendorData, 0);
        pEnv->ReleaseByteArrayElements(jbaEncToken, pjbEncToken, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected encToken len
    // may be NULL, so check only if !NULL
    jshort *pjsExpectedEncTokenLen = NULL;
    if (jsaExpectedEncTokenLen) {
        pjsExpectedEncTokenLen = pEnv->GetShortArrayElements(jsaExpectedEncTokenLen, NULL);
        if (NULL == pjsExpectedEncTokenLen) {
            LOGERR("GetSAElements for exp encToken len failed\n");
            pEnv->ReleaseShortArrayElements(jsaExpectedOtpLen,
                                            pjsExpectedOtpLen, 0);
            pEnv->ReleaseByteArrayElements(jbaVendorData, pjbVendorData, 0);
            pEnv->ReleaseByteArrayElements(jbaEncToken, pjbEncToken, 0);
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
            return NULL;
        }
    }

    // Allocate output buffers for otp and enc token
    uint8_t *pOutOtp = new uint8_t[pjsExpectedOtpLen[0]];
    if (pOutOtp == NULL) {
        LOGERR("Unable to allocate arrary\n");
        return NULL;
    }
    memset(pOutOtp, 0, pjsExpectedOtpLen[0]);
    jsize jszOutDataLen = pjsExpectedOtpLen[0];
    uint8_t *pOutEncToken = NULL;
    if (pjsExpectedEncTokenLen && pjsExpectedEncTokenLen[0] > 0) {
        pOutEncToken = new uint8_t[pjsExpectedEncTokenLen[0]];
        if (pOutEncToken == NULL) {
            LOGERR("Unable to allocate arrary\n");
            delete[] pOutOtp;
            return NULL;
        }

        memset(pOutEncToken, 0, pjsExpectedEncTokenLen[0]);
        jszOutDataLen += pjsExpectedEncTokenLen[0];
    }

    /*
    // Calculate expected out buffer size
    jsize jszExpectedOutDataLen = (pjsExpectedEncTokenLen)?
    ((jsize)pjsExpectedOtpLen[0]):
    ((jsize)pjsExpectedOtpLen[0] +
    (jsize)pjsExpectedEncTokenLen[0]);
    */

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAGetOTP(sAppName,
                                            jiSessionHandle,
                                            encTokenLen,
                                            reinterpret_cast<uint8_t *>(pjbEncToken), // may be NULL
                                            vendorDataLen,
                                            reinterpret_cast<uint8_t *>(pjbVendorData), // may be NULL
                                            reinterpret_cast<uint16_t *>(pjsExpectedOtpLen),
                                            pOutOtp,
                                            reinterpret_cast<uint16_t *>(pjsExpectedEncTokenLen), // may be NULL
                                            pOutEncToken); // may be NULL

    uint16_t outOtpLen = pjsExpectedOtpLen[0];
    uint16_t outEncTokenLen = 0;

    // release stuff, copying C values back into the Java variables
    if (pjsExpectedEncTokenLen) { // may be NULL
        outEncTokenLen = pjsExpectedEncTokenLen[0];
        pEnv->ReleaseShortArrayElements(jsaExpectedEncTokenLen,
                                        pjsExpectedEncTokenLen, 0);
    }
    pEnv->ReleaseShortArrayElements(jsaExpectedOtpLen,
                                    pjsExpectedOtpLen, 0);
    if (pjbVendorData) { // may be NULL
        pEnv->ReleaseByteArrayElements(jbaVendorData,
                                       pjbVendorData, 0);
    }

    if (pjbEncToken) { // may be NULL
        pEnv->ReleaseByteArrayElements(jbaEncToken,
                                       pjbEncToken, 0);
    }

    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        if (pOutOtp != NULL) {
            delete[] pOutOtp;
            pOutOtp = NULL;
        }

        if (pOutEncToken != NULL) {
            delete[] pOutEncToken;
            pOutEncToken = NULL;

        }

        throwIHAException(pEnv, ret);
        return NULL;
    } else {
        // Allocate main output buffer
        jbyteArray jbaOutData;
        jbaOutData = pEnv->NewByteArray(jszOutDataLen + 4);

        // May need to ensure that (outOtpLen + outEncTokenLen == jszOutDataLen)
        // For now, assume this is true

        pEnv->SetByteArrayRegion(jbaOutData, 0,
                                 outOtpLen,
                                 reinterpret_cast<jbyte *>(pOutOtp));
        if (outEncTokenLen > 0) {
            pEnv->SetByteArrayRegion(jbaOutData,
                                     (jsize)outOtpLen,
                                     (jsize)outEncTokenLen,
                                     (const jbyte *)pOutEncToken);
        }

        if (pOutOtp != NULL) {
            delete[] pOutOtp;
            pOutOtp = NULL;
        }

        if (pOutEncToken) {
            delete[] pOutEncToken;
            pOutEncToken = NULL;
        }

        LEAVE("\n");
        return jbaOutData;
    }
} // JNI-IHAGetOTP


/******************************************************************************
 * JNI version of IHA_GetOTPSStatus()
 ******************************************************************************/
JNIEXPORT jint JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetOTPSStatus(JNIEnv *pEnv,
                                                           jclass jclazz,
                                                           jstring jstrAppName,
                                                           jint jiSessionHandle,
                                                           jshort jsType) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return 0;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return 0;
    }

    uint32_t status = 0;
    uint32_t ret = sServerHandle->IHAGetOTPSStatus(sAppName,
                                                   jiSessionHandle,
                                                   jsType,
                                                   &status);

    // release appName
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return 0;
    }

    LEAVE("\n");
    return static_cast<jint>(status);
} // JNI-IHAGetOTPSStatus


/******************************************************************************
 * JNI version of IHA_GetOTPCapabilities()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetOTPCapabilities(JNIEnv *pEnv,
                                                                jclass jclazz,
                                                                jstring jstrAppName,
                                                                jshort jsType,
                                                                jshortArray jsaExpectedDataLen) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected out data len
    jshort *pjsExpectedDataLen = pEnv->GetShortArrayElements(jsaExpectedDataLen, NULL);
    if (NULL == pjsExpectedDataLen) {
        LOGERR("GetSAElements for exp data len failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected data Len > 0
    if (pjsExpectedDataLen[0] <= 0) {
        LOGERR("Incorrect expected len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // Allocate output buffer
    jbyteArray jbaOutData;
    jbaOutData = pEnv->NewByteArray((jsize)pjsExpectedDataLen[0]);
    jbyte *pjbOutData = pEnv->GetByteArrayElements(jbaOutData, NULL);
    if (NULL == pjbOutData) {
        LOGERR("GetBAElements for outData failed\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return NULL;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAGetOTPCapabilities(sAppName,
                                                        jsType,
                                                        reinterpret_cast<uint16_t *>(pjsExpectedDataLen),
                                                        reinterpret_cast<uint8_t *>(pjbOutData));

    // release stuff, copying C values back into the Java variables
    pEnv->ReleaseByteArrayElements(jbaOutData, pjbOutData, 0);
    pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                    pjsExpectedDataLen, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return NULL;
    }

    LEAVE("\n");
    return jbaOutData;

} // JNI-IHAGetOTPCapabilities


/******************************************************************************
 * JNI version of IHA_InstallOTPS()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAInstallOTPS(JNIEnv *pEnv,
                                                         jclass jclazz,
                                                         jstring jstrAppName,
                                                         jstring jstrSrcFile) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // get strFile
    const jchar *sSrcFile = pEnv->GetStringChars(jstrSrcFile, NULL);
    if (NULL == sSrcFile) {
        LOGERR("GetStringUTFChars failed for srcFile\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAInstallOTPS(sAppName,
                                                 reinterpret_cast<TCHAR *>(const_cast<jchar *>(sSrcFile)));

    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
    pEnv->ReleaseStringChars(jstrSrcFile, sSrcFile);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    }

    LEAVE("\n");
    return;
} // JNI-IHAInstallOTPS


/******************************************************************************
 * JNI version of IHA_UninstallOTPS()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAUninstallOTPS(JNIEnv *pEnv,
                                                           jclass jclazz,
                                                           jstring jstrAppName) {
    ENTER("\n");
    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }

    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAUninstallOTPS(sAppName);

    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    }

    LEAVE("\n");
    return;
} // JNI-IHAUninstallOTPS


// IPT2.0 begin change
/******************************************************************************
 * JNI version of IHA_SendAndReceiveData()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHASendAndReceiveData(JNIEnv *pEnv,
                                                                jclass jclazz,
                                                                jstring jstrAppName,
                                                                jint jiSessionHandle,
                                                                jshort jsDataType,
                                                                jbyteArray jbaInData,
                                                                jshortArray jsaExpectedOutDataLen) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get InData
    // may be NULL, so check only if !NULL
    jbyte *pjbInData = NULL;
    uint16_t inDataLen = 0;
    if (jbaInData) {
        pjbInData = pEnv->GetByteArrayElements(jbaInData, NULL);
        if (NULL == pjbInData) {
            LOGERR("GetBAElements for inData failed\n");
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
            return NULL;
        } else {
            inDataLen = static_cast<uint16_t>(pEnv->GetArrayLength(jbaInData));
        }
    }

    // get expected out data len
    // may be NULL, so check only if !NULL
    jshort *pjsExpectedOutDataLen = NULL;
    uint16_t expectedOutDataLen = 0;
    if (jsaExpectedOutDataLen) {
        pjsExpectedOutDataLen = pEnv->GetShortArrayElements(jsaExpectedOutDataLen, NULL);
        if (NULL == pjsExpectedOutDataLen) {
            LOGERR("GetSAElements for exp data len failed\n");
            if (pjbInData) {
                pEnv->ReleaseByteArrayElements(jbaInData, pjbInData, 0);
            }
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
            return NULL;
        } else {
            expectedOutDataLen = static_cast<uint16_t>(pjsExpectedOutDataLen[0]);
        }
    }

    // Allocate output buffer
    jbyteArray jbaOutData = NULL;
    jbyte *pjbOutData = NULL;
    if (expectedOutDataLen > 0) {
        if (pjsExpectedOutDataLen) {
            jbaOutData = pEnv->NewByteArray((jsize)pjsExpectedOutDataLen[0]);
        }
        pjbOutData = pEnv->GetByteArrayElements(jbaOutData, NULL);
        if (NULL == pjbOutData) {
            LOGERR("GetBAElements for outData failed\n");
            if (pjsExpectedOutDataLen) pEnv->ReleaseShortArrayElements(jsaExpectedOutDataLen,
                                                                       pjsExpectedOutDataLen, 0);
            if (pjbInData) pEnv->ReleaseByteArrayElements(jbaInData, pjbInData, 0);
            pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
            throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
            return NULL;
        }
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHASendAndReceiveData(sAppName,
                                                        jiSessionHandle,
                                                        jsDataType,
                                                        inDataLen,
                                                        reinterpret_cast<uint8_t *>(pjbInData),
                                                        &expectedOutDataLen,
                                                        reinterpret_cast<uint8_t *>(pjbOutData));

    // release stuff, copying C values back into the Java variables
    if (pjbOutData) {
        pEnv->ReleaseByteArrayElements(jbaOutData, pjbOutData, 0);
    }

    if (pjsExpectedOutDataLen) {
        pjsExpectedOutDataLen[0] = expectedOutDataLen;
        pEnv->ReleaseShortArrayElements(jsaExpectedOutDataLen,
                                        pjsExpectedOutDataLen, 0);
    }
    if (pjbInData) pEnv->ReleaseByteArrayElements(jbaInData, pjbInData, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return NULL;
    }

    LEAVE("\n");
    // Do we need to check if there is outData to send?
    return jbaOutData;

} // JNI-IHASaRData


/******************************************************************************
 * JNI version of IHA_GetCapabilities()
 ******************************************************************************/
JNIEXPORT jbyteArray JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetCapabilities(JNIEnv *pEnv,
                                                             jclass jclazz,
                                                             jstring jstrAppName,
                                                             jshort jsType,
                                                             jshortArray jsaExpectedDataLen) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return NULL;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // get expected out data len
    jshort *pjsExpectedDataLen = pEnv->GetShortArrayElements(jsaExpectedDataLen, NULL);
    if (NULL == pjsExpectedDataLen) {
        LOGERR("GetSAElements for exp data len failed\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // ensure expected data Len > 0
    if (pjsExpectedDataLen[0] <= 0) {
        LOGERR("Incorrect expected len\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return NULL;
    }

    // Allocate output buffer
    jbyteArray jbaOutData;
    jbaOutData = pEnv->NewByteArray((jsize)pjsExpectedDataLen[0]);
    jbyte *pjbOutData = pEnv->GetByteArrayElements(jbaOutData, NULL);
    if (NULL == pjbOutData) {
        LOGERR("GetBAElements for outData failed\n");
        pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                        pjsExpectedDataLen, 0);
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INTERNAL_ERROR);
        return NULL;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAGetCapabilities(sAppName,
                                                     jsType,
                                                     reinterpret_cast<uint16_t *>(pjsExpectedDataLen),
                                                     reinterpret_cast<uint8_t *>(pjbOutData));

    // release stuff, copying C values back into the Java variables
    pEnv->ReleaseByteArrayElements(jbaOutData, pjbOutData, 0);
    pEnv->ReleaseShortArrayElements(jsaExpectedDataLen,
                                    pjsExpectedDataLen, 0);
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return NULL;
    }

    LEAVE("\n");
    return jbaOutData;

} // JNI-IHAGetCapabilities


/******************************************************************************
 * JNI version of IHA_GetVersion()
 ******************************************************************************/
JNIEXPORT jint JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetVersion(JNIEnv *pEnv,
                                                        jclass jclazz) {
    ENTER("\n");

    uint32_t version = 0;
    uint32_t ret = sServerHandle->IHAGetVersion(&version);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return 0;
    }

    LEAVE("\n");
    return (jint)version;

} // JNI-IHAGetVersion


/******************************************************************************
 * JNI version of IHA_Install()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAInstall(JNIEnv *pEnv,
                                                     jclass jclazz,
                                                     jstring jstrAppName,
                                                     jstring jstrSrcFile) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // get strFile
    const jchar *sSrcFile = pEnv->GetStringChars(jstrSrcFile, NULL);
    if (NULL == sSrcFile) {
        LOGERR("GetStringChars failed for srcFile\n");
        pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAInstall(sAppName, (TCHAR *)sSrcFile);

    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);
    pEnv->ReleaseStringChars(jstrSrcFile, sSrcFile);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR(", Need to throw exception\n");
        throwIHAException(pEnv, ret);
    }
    LEAVE("\n");

    return;
} // JNI-IHAInstall


/******************************************************************************
 * JNI version of IHA_UninstallOTPS()
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHAUninstall(JNIEnv *pEnv,
                                                       jclass jclazz,
                                                       jstring jstrAppName) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAUninstall(sAppName);

    // clear the global object reference
    IhaManagerJni_CleanupCb(pEnv, sAppName);

    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    }
    LEAVE("\n");

    return;
} // JNI-IHAUninstall


/******************************************************************************
 * JNI version of IHA_DoFWUpdate
 ******************************************************************************/
JNIEXPORT void JNICALL
Java_com_intel_host_ipt_iha_IhaManagerJni_IHADoFWUpdate(JNIEnv *pEnv,
                                                        jclass jclazz) {
    ENTER("\n");

    uint32_t ret = sServerHandle->IHADoFWUpdate();

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
        return;
    }

    LEAVE("\n");
    return;

} // JNI-IHADoFWUpdate

/******************************************************************************
 * JNI version of IHA_RegisterEventCb
 ******************************************************************************/
JNIEXPORT void JNICALL Java_com_intel_host_ipt_iha_IhaManagerJni_IHARegisterEventCb(JNIEnv *pEnv,
                                                                                    jclass jclazz,
                                                                                    jstring jstrAppName,
                                                                                    jobject jobjcallback) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

    // store env and callback object references for future invocation
    jclass objclass = pEnv->GetObjectClass(jobjcallback);
    jmethodID method = pEnv->GetMethodID(objclass, "processNotification",
                                         "(Ljava/lang/String;I[BB)V");
    if (!method) {
        LOGERR("callback method not found\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

#ifdef EVENTCB_IMPLEMENTED
    // call internal IHA function
    uint32_t ret = sServerHandle->IHARegisterEventCb(sAppName, IhaManagerJni_Callback);
#else
    uint32_t ret = IHA_RET_E_NOT_IMPLEMENTED;
#endif
    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    } else {
        gobjCallback = pEnv->NewGlobalRef(jobjcallback);
        pEnv->GetJavaVM(&g_vm);
        LEAVE("\n");
    }

    return;

} // IHA_RegisterEventCb

/******************************************************************************
 * JNI version of IHA_UnregisterEventCb
 ******************************************************************************/
JNIEXPORT void JNICALL Java_com_intel_host_ipt_iha_IhaManagerJni_IHAUnregisterEventCb(JNIEnv *pEnv,
                                                                                      jclass jclazz,
                                                                                      jstring jstrAppName) {
    ENTER("\n");

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return;
    }

#ifdef EVENTCB_IMPLEMENTED
    // call internal IHA function
    uint32_t ret = sServerHandle->IHAUnregisterEventCb((char *)sAppName);
#else
    uint32_t ret = IHA_RET_E_NOT_IMPLEMENTED;
#endif

    // clear the global object reference
    IhaManagerJni_CleanupCb(pEnv, sAppName);

    // release stuff
    pEnv->ReleaseStringUTFChars(jstrAppName, sAppName);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    }

    LEAVE("OK\n");
    return;

}

/******************************************************************************
 * JNI version of IHA_GetAppInstId
 ******************************************************************************/
JNIEXPORT jlong JNICALL Java_com_intel_host_ipt_iha_IhaManagerJni_IHA_1GetAppInstId(JNIEnv *pEnv,
                                                                                    jclass jclazz,
                                                                                    jstring jstrAppName) {
    ENTER("\n");

    void *pAppletInstId = NULL;
    jlong AppInstId = 0;

    if (pEnv == NULL) {
        LOGERR("pEnv is null\n");
        return 0;
    }
    // get appName
    const char *sAppName = pEnv->GetStringUTFChars(jstrAppName, NULL);
    if (NULL == sAppName) {
        LOGERR("GetStringUTFChars failed for appName\n");
        throwIHAException(pEnv, IHA_RET_E_INVALID_INPUT);
        return 0;
    }

    // call internal IHA function
    uint32_t ret = sServerHandle->IHAGetAppInstId(sAppName, &pAppletInstId, NULL);

    // handle ret from IHA call
    if (IHA_RET_S_OK != ret) {
        LOGERR("Need to throw exception\n");
        throwIHAException(pEnv, ret);
    } else {
        AppInstId = reinterpret_cast<jlong>(pAppletInstId);
        LEAVE("\n");
    }

    return AppInstId;
}

// IPT2.0 end change
static JNINativeMethod gMethods[] =
{
    { "IHAInit", "()V", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAInit },
    { "IHADeInit", "()V",                   (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHADeInit },
    { "IHAGetOTP", "(Ljava/lang/String;I[B[B[S[S)[B", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetOTP },
    { "IHAStartProvisioning", "(Ljava/lang/String;)I", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAStartProvisioning },
    { "IHAEndProvisioning", "(Ljava/lang/String;I[S)[B", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAEndProvisioning },
    { "IHAGetSVPMessage", "(Ljava/lang/String;I[S)[B", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetSVPMessage },
    { "IHAProcessSVPMessage", "(Ljava/lang/String;I[B)V", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAProcessSVPMessage },
    { "IHASendAndReceiveData", "(Ljava/lang/String;IS[B[S)[B", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHASendAndReceiveData },
    { "IHAGetCapabilities", "(Ljava/lang/String;S[S)[B", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetCapabilities },
    { "IHAGetVersion", "()I", (void *)&Java_com_intel_host_ipt_iha_IhaManagerJni_IHAGetVersion }
};


extern "C"
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    ENTER("\n");
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_LOADERROR;
    } else {
        jclass clazz = env->FindClass("com/intel/host/ipt/iha/IhaManagerJni");
        if (clazz) {
            env->RegisterNatives(clazz, gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
            env->DeleteLocalRef(clazz);
            return JNI_VERSION_1_6;
        } else {
            return JNI_LOADERROR;
        }
    }
}

extern "C"
void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed\n");
        return;
    }
    env->DeleteGlobalRef(g_obj);
}

//} // namespace com_intel_host_ipt_iha
