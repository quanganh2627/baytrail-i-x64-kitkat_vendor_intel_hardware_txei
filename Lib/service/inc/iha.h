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
**    @file iha.h
**
**    @brief  Defines the Intel IPT Host Agent (IHA) API
**
**    @author Ranjit Narjala
**    @author Rahul Ghosh
**
********************************************************************************
*/


#ifndef _IHA_H
#define _IHA_H

#ifndef ANDROID
#include <stdio.h>
#include <Windows.h>
#endif

#ifdef IHADLL_EXPORTS
#define IHA_WIN32DLL_API  __declspec(dllexport)
#else
#define IHA_WIN32DLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;

#define IN      // Defines an input parameter
#define OUT     // Defines an output parameter
#define INOUT   // Defines an input/output parameter

// Types for IHA_GetOTPCapabilities (IPT1.x API)
// and IHA_GetCapabilities (IPT2.0 API)
#define IHA_GETCAPS_APPVER              1
#define IHA_GETCAPS_APPSECVER           2

#define APP_NAME_MAX_LENGTH             32

/**************************************************************************//**
 * Callback function prototype for events received from the ME DAL. \n
 * The callback function will be implemented by the IHA application and
 * registered with the embedded app with IHA_RegisterEventCb() to process
 * events received from it. Different callbacks can be registered for
 * different embedded apps.
 * This prototype is used only for IPT 2.0.
 *
 * @param [in]  sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]  dataLen: length of data in event buffer
 * @param [in]  pData: pointer to event data buffer
 * @param [in]  dataSrc: 0 - event is from embedded app \n
 *                       1 - event is from embedded FW service
 *
 * @return  none
 *****************************************************************************/
typedef void (*IHA_EventCbFunc)(char *sAppName,
                                UINT32 dataLen,
                                UINT8 *pData,
                                UINT8 dataSrc);

/**************************************************************************//**
 * Initialize internal dependencies. Should be called first and only once
 * after loading the iha library.
 *
 * @param   none
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_INTERNAL_ERROR: The DLL encountered problems, and the
 *              embedded app should be considered not available. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_Init();
#else
IHA_WIN32DLL_API UINT32 IHA_Init();
#endif

/**************************************************************************//**
 * Deinitialize IHA library. Should be called before unloading the DLL.
 *
 * @param   none
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error occured. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_DeInit();
#else
IHA_WIN32DLL_API UINT32 IHA_DeInit();
#endif

/**************************************************************************//**
 * Initiate a provisioning session. This call will initialize provisioning
 * in the embedded app, and the session handle returned must be used in
 * subsequent provisioning-related calls.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [out]     pSessionHandle: Pointer to the sessionHandle, filled in by
 *                  the callee at successful call completion.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_StartProvisioning(IN char* sAppName, OUT UINT32 *pSessionHandle);
#else
IHA_WIN32DLL_API UINT32 IHA_StartProvisioning(
                            IN      char *sAppName,
                            OUT     UINT32 *pSessionHandle);
#endif

/**************************************************************************//**
 * Once provisioning is complete, this function must be called to ensure
 * state is cleaned up, and to obtain the encrypted Token Record from the
 * embedded app. The encrypted Token Record that is received must be stored
 * by the application, and supplied again in a call such as IHA_GetOTP().
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: SessionHandle identifying a provisioning
 *                  sequence.
 * @param [in, out] pEncTokenLength: Pointer to field identifying the length
 *                  of the buffer the caller has allocated for the encrypted
 *                  Token.  If this buffer is insufficient, the call will
 *                  return with an error indicating the same and this field
 *                  will specify the required buffer length.  The function
 *                  will need to be called again with the correct buffer size.
 * @param [out]     pEncToken: pointer to a buffer where the callee should
 *                  copy the encrypted token record.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_EndProvisioning(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        INOUT   UINT16 *pEncTokenLength,
                                                        OUT             UINT8 *pEncToken);
#else
IHA_WIN32DLL_API UINT32 IHA_EndProvisioning(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            INOUT   UINT16 *pEncTokenLength,
                            OUT     UINT8 *pEncToken);
#endif

/**************************************************************************//**
 * This function is used to send data to the embedded app in the FW.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of data being sent.
 * @param [in]      dataLength: Length of the input data buffer.
 * @param [in]      pData: Pointer to the input data buffer.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_SendData(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 dataType,
                                                        IN              UINT16 dataLength,
                                                        IN              UINT8 *pData);
#else
IHA_WIN32DLL_API UINT32 IHA_SendData(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 dataLength,
                            IN      UINT8 *pData);
#endif

/**************************************************************************//**
 * This function is used to obtain data from the embedded app in the FW.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of data being requested.
 * @param [in, out] pDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size.
 * @param [out]     pData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_ReceiveData(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 dataType,
                                                                        // defined by ISV^M
                                                        INOUT   UINT16 *pDataLength,
                                                        OUT             UINT8 *pData);
#else
IHA_WIN32DLL_API UINT32 IHA_ReceiveData(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                                    // defined by ISV
                            INOUT   UINT16 *pDataLength,
                            OUT     UINT8 *pData);
#endif

/**************************************************************************//**
 * This function is used to send IPT-specific provisioning data to the
 * embedded app in the FW.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence.
 * @param [in]      inDataLength: Length of the input data buffer.
 * @param [in]      pInData: Pointer to the input data buffer.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_ProcessSVPMessage(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 inDataLength,
                                                        IN              UINT8 *pInData);
#else
IHA_WIN32DLL_API UINT32 IHA_ProcessSVPMessage(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 inDataLength,
                            IN      UINT8 *pInData);
#endif

/**************************************************************************//**
 * This function is used to obtain IPT-specific provisioning data from the
 * embedded app in the FW.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence.
 * @param [in, out] pOutDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size.
 * @param [out]     pOutData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetSVPMessage(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        INOUT   UINT16 *pOutDataLength,
                                                        OUT             UINT8 *pOutData);

#else
IHA_WIN32DLL_API UINT32 IHA_GetSVPMessage(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            INOUT   UINT16 *pOutDataLength,
                            OUT     UINT8 *pOutData);
#endif


/**************************************************************************//**
 * This function is used to send and receive data to and from the embedded app
 * in the FW. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of action that is being requested.
 * @param [in]      inDataLength: Length of the input data buffer.
 * @param [in]      pInData: Pointer to the input data buffer.
 * @param [in, out] pOutDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size. Note that if no
 *                  outData is requested, this parameter must still be
 *                  allocated and the value set to 0.
 * @param [out]     pOutData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW. May be NULL if no data is
 *                  expected from the FW.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_SendAndReceiveData(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 dataType,
                                                        IN              UINT16 inDataLength,
                                                        IN              UINT8 *pInData,
                                                        INOUT   UINT16 *pOutDataLength,
                                                        OUT             UINT8 *pOutData);

#else
IHA_WIN32DLL_API UINT32 IHA_SendAndReceiveData(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 inDataLength,
                            IN      UINT8 *pInData,
                            INOUT   UINT16 *pOutDataLength,
                            OUT     UINT8 *pOutData);
#endif

/**************************************************************************//**
 * This function is used to obtain capabilities of embedded app, including
 * version information. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      type: Type of capability information requested. Can be: \n
                        1: Embedded App Version \n
                        2: Embedded App Security Version.
 * @param [in, out] pDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size.
 * @param [out]     pData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW. \n
 *                      For type 1: Output is a string containing the version.
 *                          pDataLength should be set to 6.
 *                      For type 2. Output is a string containing the version.
 *                          pDataLength should be set to 6.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetCapabilities(
                                                        IN              char *sAppName,
                                                        IN              UINT16 type,
                                                        INOUT   UINT16 *pDataLength,
                                                        OUT             UINT8 *pData);
#else
IHA_WIN32DLL_API UINT32 IHA_GetCapabilities(
                            IN      char *sAppName,
                            IN      UINT16 type,
                            INOUT   UINT16 *pDataLength,
                            OUT     UINT8 *pData);
#endif

/**************************************************************************//**
 * This function is used to retrieve the version of the IHA DLL. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [out]     pVersion: Pointer to a buffer that will contain the version
 *                  number on successful call completion.
 *                  The version number is returned in the following format:
 *                  typedef struct
 *                  {
 *                      UINT8   major;
 *                      UINT8   minor;
 *                      UINT8   build;
 *                      UINT8   subminor;
 *                  } IPT_VERSION_DETAIL;
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetVersion(OUT UINT32 *pVersion);
#else
IHA_WIN32DLL_API UINT32 IHA_GetVersion(
                            OUT     UINT32 *pVersion);
#endif

/**************************************************************************//**
 * This function is used to load the embedded app into the FW. This is
 * typically called the very first time the embedded app is used, if it has not
 * been loaded yet. It is also used every time the embedded app needs to be
 * updated. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      sSrcFile: Unicode string containing the full path,
 *                  including filename of the embedded app that needs to be
 *                  installed in the chipset.  Once this command returns
 *                  successfully, the folder and file that this path points to
 *                  can be deleted.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_Install(
                                                        IN              char *sAppName,
                                                        IN              wchar_t *sSrcFile);

#else
IHA_WIN32DLL_API UINT32 IHA_Install(
                            IN      char *sAppName,
                            IN      wchar_t *sSrcFile);
#endif

/**************************************************************************//**
 * This function is used to unload the embedded app from the FW. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_Uninstall(
                                                        IN              char *sAppName);

#else
IHA_WIN32DLL_API UINT32 IHA_Uninstall(
                            IN      char *sAppName);
#endif

/**************************************************************************//**
 * This function is used to trigger a FW update when an EPID group key
 * revocation is detected. \n
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param   none
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_FW_UPDATE_IN_PROGRESS: FW Update is already in
 *               progress.\n
 *          IHA_RET_E_FW_UPDATE_CALL_FAILED: Call to the FW Update service
 *              failed. \n
 *          IHA_RET_E_FW_UPDATE_NONE_AVAILABLE: No FW Update available. \n
 *          IHA_RET_E_FW_UPDATE_SERVICE_BUSY: FW Update service is busy. \n
 *          IHA_RET_E_FW_UPDATE_DLL_NOT_READY: FW Update service not
 *              available. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_DoFWUpdate();
#else
IHA_WIN32DLL_API UINT32 IHA_DoFWUpdate();
#endif


/**************************************************************************//**
 * This function is used to register a callback function with the embedded
 * fw to receive and process notifications from the embedded app. \n
 * This API can be used to register callback functions for multiple eApps. The
 * same callback function can be reused for multiple eApps as well. However, a
 * single eApp may only have one callback function registered. The callback
 * gets deregistered when the app gets uninstalled and/or the IHA DLL gets
 * deinitialized.\n
  * If IHA_UnregisterEventCb() was used to clear a prior registration this
 * API can be called again to re-register a callback.\n
 * If notification support is required by the IHA application then this
 * MUST be called before any other request to the embedded app, else
 * notification registration will fail.
 * This API was added in IPT 2.0, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      EventCbFunc: IHA app's event handler function
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_ALREADY_REGISTERED_FOR_EVENTS: A different event
 *              callback is already registered with the embedded app. \n
 *          IHA_RET_E_EVENTS_NOT_SUPPORTED: Embedded app does not support
 *              events in its current state \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_RegisterEventCb(
                                                        IN              char                    *sAppName,
                                                        IN              IHA_EventCbFunc EventCbFunc);
#else
IHA_WIN32DLL_API UINT32 IHA_RegisterEventCb(
                            IN      char            *sAppName,
                            IN      IHA_EventCbFunc EventCbFunc);
#endif
/**************************************************************************//**
 * This function is used to unregister an existing callback function from
 * the embedded fw. \n
 * This API should only be used if IHA_RegisterEventCb has been called previously
 * to register a callback function with the embedded FW. It returns silently with
 * success if used otherwise.\n
 * It is NOT mandatory to call this API to unregister callbacks, that will get
 * done internally at sw uninstall stage. One potential use case for the API
 * would be if client application fails to complete any post-registration
 * operation i.e. after IHA_RegisterEventCb() has returned successfully, and
 * needs to re-do the registration from scratch by unregistering the callback
 * first.\n
 * This API was added in IPT 2.1, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Unregistration completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_UnregisterEventCb(
                            IN      char            *sAppName);

#else
/**************************************************************************//**
 * This function is used to unregister an existing callback function from
 * the embedded fw. \n
 * This API should only be used if IHA_RegisterEventCb has been called previously
 * to register a callback function with the embedded FW. It returns silently with
 * success if used otherwise.\n
 * It is NOT mandatory to call this API to unregister callbacks, that will get
 * done internally at sw uninstall stage. One potential use case for the API
 * would be if client application fails to complete any post-registration
 * operation i.e. after IHA_RegisterEventCb() has returned successfully, and
 * needs to re-do the registration from scratch by unregistering the callback
 * first.\n
 * This API was added in IPT 2.1, and is only to be used by those apps that do
 * not need to run on 2011 platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Unregistration completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *****************************************************************************/
IHA_WIN32DLL_API UINT32 IHA_UnregisterEventCb(
                            IN      char            *sAppName);
#endif

/**************************************************************************//**
 * This function is used to retrieve an OTP from the embedded OTP App.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Must be 0 if called post-provisioning. If a valid
 *                  session handle is passed in, then this is to be used in the
 *                  context of a provisioning sequence, and no encrypted Token
 *                  should be passed in.
 * @param [in]      encTokenLength: Length of the buffer being passed in
 *                  containing the encrypted Token. Must be 0 if session
 *                  handle is used.
 * @param [in]      pEncToken: Pointer to the buffer containing the encrypted
 *                  Token. Must be NULL if session handle is used.
 * @param [in]      vendorDataLength: Length of the buffer being passed in
 *                  containing vendor-specific data.
 * @param [in]      pVendorData: Pointer to the buffer containing the vendor
 *                  data.  Optional.
 * @param [in, out] pOTPLength: Pointer to field identifying the length of the
 *                  buffer the caller has allocated for the OTP data to be
 *                  received from embedded app.  If this buffer is
 *                  insufficient, the call will return with an error indicating
 *                  the same and this field will specify the required buffer
 *                  length.  The function will need to be called again with the
 *                  correct buffer size.
 * @param [out]     pOTP: Pointer to a buffer where the callee should copy the
 *                  OTP received from the FW.
 * @param [in, out] pOutEncTokenLength: Pointer to field identifying the length
 *                  of the buffer the caller has allocated for the updated
 *                  encrypted Token to be received from the embedded app. This
 *                  is optional; if the caller does not require the encrypted
 *                  token to be sent back, this field can be set to 0.  If it
 *                  is set, and if this buffer is insufficient, the call will
 *                  return with an error indicating the same and this field
 *                  will specify the required buffer length.  The function will
 *                  need to be called again with the correct buffer size.  This
 *                  field must be 0 if session handle is used.
 * @param [out]     pOutEncToken: Pointer to a buffer where the callee should
 *                  copy the updated encrypted Token received from the FW.  If
 *                  pOutEncTokenLength is set to 0, this field must be set to
 *                  NULL. This field must be set to NULL if session handle is
 *                  used.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetOTP(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 encTokenLength,
                                                        IN              UINT8 *pEncToken,
                                                        IN              UINT16 vendorDataLength,
                                                        IN              UINT8 *pVendorData,
                                                        INOUT   UINT16 *pOTPLength,
                                                        OUT             UINT8 *pOTP,
                                                        INOUT   UINT16 *pOutEncTokenLength,
                                                        OUT             UINT8 *pOutEncToken);
#else
IHA_WIN32DLL_API UINT32 IHA_GetOTP(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 encTokenLength,
                            IN      UINT8 *pEncToken,
                            IN      UINT16 vendorDataLength,
                            IN      UINT8 *pVendorData,
                            INOUT   UINT16 *pOTPLength,
                            OUT     UINT8 *pOTP,
                            INOUT   UINT16 *pOutEncTokenLength,
                            OUT     UINT8 *pOutEncToken);
#endif


/**************************************************************************//**
 * This function is used to obtain status from the embedded app. \n
 * This is a legacy API, kept for backward compatibility with version 1.x.
 * Should not be used from version 2.0 onwards.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Can be NULL if session-specific status is not
 *                  required.
 * @param [in]      type: Type of status requested.
 * @param [out]     pStatus: Pointer to a field where the status from the FW
 *                  will be copied into by the callee
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetOTPSStatus(
                                                        IN              char *sAppName,
                                                        IN              UINT32 hSessionHandle,
                                                        IN              UINT16 type,
                                                        OUT             UINT32 *pStatus);

#else
IHA_WIN32DLL_API UINT32 IHA_GetOTPSStatus(
                            IN      char *sAppName,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 type,
                            OUT     UINT32 *pStatus);
#endif

/**************************************************************************//**
 * This function is used to obtain capabilities of embedded app, including
 * version information. \n
 * This is a legacy API, kept for backward compatibility with version 1.x.
 * Has been replaced by IHA_GetCapabilities() in version 2.0 onwards.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      type: Type of capability information requested. Can be: \n
                        1: Embedded App Version \n
                        2: Embedded App Security Version.
 * @param [in, out] pDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size.
 * @param [out]     pData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW. \n
 *                      For type 1: Output is a string containing the version.
 *                          pDataLength should be set to 6.
 *                      For type 2. Output is a string containing the version.
 *                          pDataLength should be set to 6.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetOTPCapabilities(
                                                        IN              char *sAppName,
                                                        IN              UINT16 type,
                                                        INOUT   UINT16 *pDataLength,
                                                        OUT             UINT8 *pData);

#else
IHA_WIN32DLL_API UINT32 IHA_GetOTPCapabilities(
                            IN      char *sAppName,
                            IN      UINT16 type,
                            INOUT   UINT16 *pDataLength,
                            OUT     UINT8 *pData);
#endif


/**************************************************************************//**
 * This function is used to load the embedded app into the FW. This is
 * typically called the very first time the embedded app is used, if it has not
 * been loaded yet. It is also used every time the embedded app needs to be
 * updated. \n
 * This is a legacy API, kept for backward compatibility with version 1.x.
 * Has been replaced by IHA_Install() in version 2.0 onwards.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      sSrcFile: Unicode string containing the full path,
 *                  including filename of the embedded app that needs to be
 *                  installed in the chipset.  Once this command returns
 *                  successfully, the folder and file that this path points to
 *                  can be deleted.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_InstallOTPS(
                                                        IN              char *sAppName,
                                                        IN              wchar_t *sSrcFile);

#else
IHA_WIN32DLL_API UINT32 IHA_InstallOTPS(
                            IN      char *sAppName,
                            IN      wchar_t *sSrcFile);
#endif

/**************************************************************************//**
 * This function is used to unload the embedded app from the FW. \n
 * This is a legacy API, kept for backward compatibility with version 1.x.
 * Has been replaced by IHA_Uninstall() in version 2.0 onwards.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_UninstallOTPS(
                                                        IN              char *sAppName);

#else
IHA_WIN32DLL_API UINT32 IHA_UninstallOTPS(
                            IN      char *sAppName);
#endif
/**************************************************************************//**
 * This function is used retrieve the embedded app's instance id. \n
 * This API was added in IPT 2.1, and is meant to be used by client applications
 * that need to reuse a non-shared embedded app instance id between multiple
 * IPT dll-s. Non-shared app instances are only created when the
 * IHA_RegisterEventCb() API is used to register an event callback with the app.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [out]     ppAppletInstId: pointer to a void pointer to store embedded
 *                  app instance. *ppAppletInstId cannot be NULL.
 * @param [out]     pFlags: pointer to UINT32 variable to store session flags in,
                    This is an optional parameter and can be set to NULL.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Retrieved app instance id successfully. \n
 *          IHA_RET_E_APPLET_SESSION_UNAVAILABLE: No session is opened on this
 *              applet \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA_GetAppInstId(IN char *sAppName,
                                         OUT void **ppAppletInstId,
                                         OUT UINT32 *pFlags);
#else
IHA_WIN32DLL_API UINT32 IHA_GetAppInstId(IN char *sAppName,
                                         OUT void **ppAppletInstId,
                                         OUT UINT32 *pFlags);
#endif

/****************************************************************************
                    IPT 3.0 APIs and types
*****************************************************************************/

#define IHA_NON_SHARED_SESSION_FLAG         0
#define IHA_SHARED_SESSION_FLAG             1

// Data types to be used with IHA_Send/ReceiveData
#define IPT_DATATYPE_PROVISIONING           600 // specifies a provisioning request

// Capability types to be used with IHA_GetCapabilities
#define IPT_CAPABILITY_TYPE_APPLET          001 // IPT caps supported by current applet

/**************************************************************************//**
 * Callback function prototype for events received from the ME DAL. \n
 * The callback function will be implemented by the IHA application and
 * registered with the embedded app with IHA_RegisterEventCb() to process
 * events received from it. Different callbacks can be registered for
 * different embedded apps.
 * This prototype is used only for IPT 3.0.
 *
 * @param [in]  instanceId: IHA instanceId for session opened with embedded app
 * @param [in]  dataLen: length of data in event buffer
 * @param [in]  pData: pointer to event data buffer
 * @param [in]  dataSrc: 0 - event is from embedded app \n
 *                       1 - event is from embedded FW service
 * @param [in]  pObject: reference to caller specific object, can be NULL
 *
 * @return  none
 *****************************************************************************/
#ifndef ANDROID
typedef void (*IHA3_EventCbFunc)(UINT32 instanceId,
                                 UINT32 dataLen,
                                 UINT8 *pData,
                                 UINT8 dataSrc,
                                 void *pObject);
#endif
/****************************************************************************//**
 * This function is used to start a new session of the embedded app in the FW.
 * This must be called after the embedded app has been installed with
 * IHA_Install().
 * Optionally, a callback function can also be registered with the embedded
 * app to receive asynchronous notifications from it. The callback function
 * registration will be specific to the instanceId associated with the current
 * session that will be opened. If another session is opened with the same
 * embedded app it will be possible to register another callback with the
 * instanceId associated with that new session.
 * Registering a callback opens an exclusive non-shared session with the
 * embedded app. If an application requires to use a non-shared session even
 * though asynchronous notifications are not needed, it can register a dummy
 * callback with this function to do so.
 * This API was added in IPT 3.0, and is only to be used by those apps that do
 * not run on 2012 and earlier platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      EventCbFunc: IHA app's event handler function. This is
 *                  optional, can be set to NULL if event registration is not
 *                  required.
 * @param [in]      pObject: Reference to caller specific data blob that will be returned
 *                  unchanged when the callback is invoked. Can be used to co-
 *                  relate the callback with the caller's object.
 * @param [out]     pInstanceId: IHA instanceId returned for the new session that
 *                  was opened with the embedded app
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifndef ANDROID
IHA_WIN32DLL_API UINT32 IHA3_StartInstance(
                            IN      char *sAppName,
                            IN      IHA3_EventCbFunc EventCbFunc,
                            IN      void *pObject,
                            OUT     UINT32 *pInstanceId);
#endif
/****************************************************************************//**
 * This function is used to stop a session with the embedded app executing
 * in the FW. This call should be made as the last call when the IHA application
 * is done with its operations with the app. If a callback function was
 * registered with this instanceId that will be unregistered before the session
 * is closed.
 * This API was added in IPT 3.0, and is only to be used by those apps that do
 * not run on 2012 and earlier platforms.
 *
 * @param [in]      instanceId: IHA instance for session opened with embedded app
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n\
 *          IHA_RET_E_INVALID_INSTANCEID: instanceId did not map to a embedded app
 *          session.
 *****************************************************************************/
#ifndef ANDROID
UINT32 IHA3_StopInstance(IN UINT32 instanceId);
#endif
/****************************************************************************//**
 * This function is used to load the embedded app into the FW and start a
 * session with it. If used, this must be the first call to the app. It is
 * basically a convenience function that combines IHA_Install() and
 * IHA3_StartInstance().
 * Optionally, a callback function can also be registered with the embedded
 * app to receive asynchronous notifications from it. The callback function
 * registration will be specific to the instanceId associated with the current
 * session that will be opened. If another session is opened with the same
 * embedded app it will be possible to register another callback with the
 * instanceId associated with that new session.
 * Registering a callback opens an exclusive non-shared session with the
 * embedded app. If an application requires to use a non-shared session even
 * though asynchronous notifications are not needed, it can register a dummy
 * callback with this function to do so.
 * This API was added in IPT 3.0, and is only to be used by those apps that do
 * not run on 2012 and earlier platforms.
 *
 * @param [in]      sAppName: String identifying the embedded app in the
 *                  chipset.
 * @param [in]      sSrcFile: Unicode string containing the full path,
 *                  including filename of the embedded app that needs to be
 *                  installed in the chipset.  Once this command returns
 *                  successfully, the folder and file that this path points to
 *                  can be deleted.
 * @param [in]      EventCbFunc: IHA app's event handler function. This is
 *                  optional, can be set to NULL if event registration is not
 *                  required.
 * @param [in]      pObject: Reference to caller specific data blob that will be returned
 *                  unchanged when the callback is invoked. Can be used to co-
 *                  relate the callback with the caller's object.
 * @param [out]     pInstanceId: IHA instanceId returned for the new session that
 *                  was opened with the embedded app
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *****************************************************************************/
#ifndef ANDROID
IHA_WIN32DLL_API UINT32 IHA3_LoadInstance(
                            IN      char *sAppName,
                            IN      wchar_t *sSrcFile,
                            IN      IHA3_EventCbFunc EventCbFunc,
                            IN      void *pObject,
                            OUT     UINT32 *pInstanceId);
#endif
/****************************************************************************//**
 * This function is used to unload the embedded app from the FW and close the
 * associated session. If used, this will be the last call to the embedded app
 * when the IHA application is done with its operations with the app.
 * If a callback function was registered with this instanceId that will be
 * unregistered before the session is closed and the app is unloaded.
 * This is basically a convenience function that combines IHA3_StopInstance()
 * and IHA_Uninstall().
 * If uninstalling the embedded app is not desired then applications should
 * use IHA3_StopInstance() instead.
 * This API was added in IPT 3.0, and is only to be used by those apps that do
 * not run on 2012 and earlier platforms.
 *
 * @param [in]  instanceId: IHA instanceId for session opened with embedded app
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n\
 *          IHA_RET_E_INVALID_INSTANCEID: instanceId did not map to a embedded app
 *          session.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_UnloadInstance(IN UINT32 instanceId);
#else
IHA_WIN32DLL_API UINT32 IHA3_UnloadInstance(IN UINT32 instanceId);
#endif
/****************************************************************************//**
 * This function is used to send and receive data to and from a specific session
 * opened with the embedded app, the session being identified by the IHA
 * instanceId \n
 * This API was added in IPT 3.0, and is only to be used by those apps that do
 * not run on 2012 and earlier platforms.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of action that is being requested.
 * @param [in]      inDataLength: Length of the input data buffer.
 * @param [in]      pInData: Pointer to the input data buffer.
 * @param [in, out] pOutDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size. Note that if no
 *                  outData is requested, this parameter must still be
 *                  allocated and the value set to 0.
 * @param [out]     pOutData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW. May be NULL if no data is
 *                  expected from the FW.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.\n
 *          IHA_RET_E_INVALID_INSTANCEID: instanceId did not map to a embedded app
 *          session.\n
 *****************************************************************************/
#ifndef ANDROID
IHA_WIN32DLL_API UINT32 IHA3_SendAndReceiveData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 inDataLength,
                            IN      UINT8 *pInData,
                            INOUT   UINT16 *pOutDataLength,
                            OUT     UINT8 *pOutData);
#else
UINT32 IHA3_SendAndReceiveData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 inDataLength,
                            IN      UINT8 *pInData,
                            INOUT   UINT16 *pOutDataLength,
                            OUT     UINT8 *pOutData);

#endif
/**************************************************************************//**
 * This function is used retrieve the embedded app's instance id. \n
 * This API was added in IPT 3.0, and is meant to be used by client applications
 * that need to reuse a non-shared embedded app instance id between multiple
 * IPT dll-s. Non-shared app instances are only created when the
 * IHA3_LoadInstance() API is called with a non-NULL callback function.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [out]     ppAppletInstId: pointer to a void pointer to store embedded
 *                  app instance. *ppAppletInstId cannot be NULL.
 * @param [out]     pFlags: pointer to UINT32 variable to store session flags in,
                    This is an optional parameter and can be set to NULL.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Retrieved app instance id successfully. \n
 *          IHA_RET_E_APPLET_SESSION_UNAVAILABLE: No session is opened on this
 *              applet \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_INVALID_INSTANCEID: instanceId did not map to a embedded app
 *          session.\n
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_GetAppInstId(IN UINT32 instanceId,
                                          OUT void **ppAppletInstId,
                                          OUT UINT32 *pFlags);
#else
IHA_WIN32DLL_API UINT32 IHA3_GetAppInstId(IN UINT32 instanceId,
                                          OUT void **ppAppletInstId,
                                          OUT UINT32 *pFlags);
#endif
/**************************************************************************//**
 * Initiate a provisioning session. This call will initialize provisioning
 * in the embedded app, and the session handle returned must be used in
 * subsequent provisioning-related calls.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [out]     pSessionHandle: Pointer to the provisioning sessionHandle,
 *                  filled in by the callee at successful call completion.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error.
 *****************************************************************************/
#ifndef ANDROID
IHA_WIN32DLL_API UINT32 IHA3_StartProvisioning(
                            IN  UINT32 instanceId,
                            OUT UINT32 *pSessionHandle);
#else
UINT32 IHA3_StartProvisioning(
                            IN  UINT32 instanceId,
                            OUT UINT32 *pSessionHandle);
#endif
/**************************************************************************//**
 * Once provisioning is complete, this function must be called to ensure
 * state is cleaned up, and to obtain the encrypted Token Record from the
 * embedded app. The encrypted Token Record that is received must be stored
 * by the application, and supplied again in a call such as IHA_GetOTP().
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      hSessionHandle: SessionHandle identifying a provisioning
 *                  sequence.
 * @param [in, out] pEncTokenLength: Pointer to field identifying the length
 *                  of the buffer the caller has allocated for the encrypted
 *                  Token.  If this buffer is insufficient, the call will
 *                  return with an error indicating the same and this field
 *                  will specify the required buffer length.  The function
 *                  will need to be called again with the correct buffer size.
 * @param [out]     pEncToken: pointer to a buffer where the callee should
 *                  copy the encrypted token record.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_EndProvisioning(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            INOUT   UINT16 *pEncTokenLength,
                            OUT     UINT8 *pEncToken);
#else
IHA_WIN32DLL_API UINT32 IHA3_EndProvisioning(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            INOUT   UINT16 *pEncTokenLength,
                            OUT     UINT8 *pEncToken);
#endif
/**************************************************************************//**
 * This function is used to send data to the embedded app in the FW.\n
 * For SVP (provisioning) related messages use dataType IPT_DATATYPE_PROVISIONING.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of data being sent.
 * @param [in]      dataLength: Length of the input data buffer.
 * @param [in]      pData: Pointer to the input data buffer.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_SendData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 dataLength,
                            IN      UINT8 *pData);
#else
IHA_WIN32DLL_API UINT32 IHA3_SendData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            IN      UINT16 dataLength,
                            IN      UINT8 *pData);
#endif
/**************************************************************************//**
 * This function is used to obtain data from the embedded app in the FW.\n
 * For SVP (provisioning) related messages use dataType IPT_DATATYPE_PROVISIONING.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Optional.
 * @param [in]      dataType: Type of data being requested.
 * @param [in, out] pDataLength: Pointer to field identifying the length of
 *                  the buffer the caller has allocated for the data.  If this
 *                  buffer is insufficient, the call will return with an error
 *                  indicating the same and this field will specify the
 *                  required buffer length.  The function will need to be
 *                  called again with the correct buffer size.
 * @param [out]     pData: Pointer to a buffer where the callee should copy
 *                  the data received from the FW.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_ReceiveData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            INOUT   UINT16 *pDataLength,
                            OUT     UINT8 *pData);
#else
IHA_WIN32DLL_API UINT32 IHA3_ReceiveData(
                            IN      UINT32 instanceId,
                            IN      UINT32 hSessionHandle,
                            IN      UINT16 dataType,
                            INOUT   UINT16 *pDataLength,
                            OUT     UINT8 *pData);
#endif
/**************************************************************************//**
 * This function is used to retrieve an OTP from the embedded OTP App.
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      hSessionHandle: Session handle identifying a provisioning
 *                  sequence. Must be 0 if called post-provisioning. If a valid
 *                  session handle is passed in, then this is to be used in the
 *                  context of a provisioning sequence, and no encrypted Token
 *                  should be passed in.
 * @param [in]      encTokenLength: Length of the buffer being passed in
 *                  containing the encrypted Token. Must be 0 if session
 *                  handle is used.
 * @param [in]      pEncToken: Pointer to the buffer containing the encrypted
 *                  Token. Must be NULL if session handle is used.
 * @param [in]      vendorDataLength: Length of the buffer being passed in
 *                  containing vendor-specific data.
 * @param [in]      pVendorData: Pointer to the buffer containing the vendor
 *                  data.  Optional.
 * @param [in, out] pOTPLength: Pointer to field identifying the length of the
 *                  buffer the caller has allocated for the OTP data to be
 *                  received from embedded app.  If this buffer is
 *                  insufficient, the call will return with an error indicating
 *                  the same and this field will specify the required buffer
 *                  length.  The function will need to be called again with the
 *                  correct buffer size.
 * @param [out]     pOTP: Pointer to a buffer where the callee should copy the
 *                  OTP received from the FW.
 * @param [in, out] pOutEncTokenLength: Pointer to field identifying the length
 *                  of the buffer the caller has allocated for the updated
 *                  encrypted Token to be received from the embedded app. This
 *                  is optional; if the caller does not require the encrypted
 *                  token to be sent back, this field can be set to 0.  If it
 *                  is set, and if this buffer is insufficient, the call will
 *                  return with an error indicating the same and this field
 *                  will specify the required buffer length.  The function will
 *                  need to be called again with the correct buffer size.  This
 *                  field must be 0 if session handle is used.
 * @param [out]     pOutEncToken: Pointer to a buffer where the callee should
 *                  copy the updated encrypted Token received from the FW.  If
 *                  pOutEncTokenLength is set to 0, this field must be set to
 *                  NULL. This field must be set to NULL if session handle is
 *                  used.
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_GetOTP(
                IN      UINT32  instanceId,
                IN      UINT32  hSessionHandle,  // NULL for typical usage
                IN      UINT16  encTokenLength,
                IN      UINT8  *pEncToken,
                IN      UINT16  vendorDataLength,
                IN      UINT8  *pVendorData,
                INOUT   UINT16 *pOTPLength,
                OUT     UINT8  *pOTP,
                INOUT   UINT16 *pOutEncTokenLength,
                OUT     UINT8  *pOutEncToken);
#else
IHA_WIN32DLL_API UINT32 IHA3_GetOTP(
                IN      UINT32  instanceId,
                IN      UINT32  hSessionHandle,  // NULL for typical usage
                IN      UINT16  encTokenLength,
                IN      UINT8  *pEncToken,
                IN      UINT16  vendorDataLength,
                IN      UINT8  *pVendorData,
                INOUT   UINT16 *pOTPLength,
                OUT     UINT8  *pOTP,
                INOUT   UINT16 *pOutEncTokenLength,
                OUT     UINT8  *pOutEncToken);
#endif
/**************************************************************************//**
 * This function is used to obtain capabilities of embedded app based on
 * the type of capability specified\n
 * This is a new API for IPT 3.0 and behaves differently from the IPT 2.0
 * IHA_GetCapabilities API. This API will not return version numbers assc
 * with the applet but is mainly for retrieving different types of functional
 * capabilities the applet provides. Capability is always returned as an
 * integer bitmap.
 * Capability Type: 0x1 - returns IPT feature set supported by embedded app
 *
 * @param [in]      instanceId: IHA instanceId for session opened with embedded app.
 * @param [in]      type: Type of capability information requested.
 * @param [out]     pCaps: Pointer to field to store retrieved capability bitmap
 *
 * @return  UINT32 value as defined in ihaError.h. \n
 *          IHA_RET_S_OK: Initialization completed successfully. \n
 *          IHA_RET_E_LIBRARY_NOT_INITIALIZED: The IHA DLL is not initialized.
 *              Call IHA_Init() first. \n
 *          IHA_RET_E_MEMORY: Unable to allocate memory. \n
 *          IHA_RET_E_INVALID_INPUT: Invalid input parameters. \n
 *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
 *              stack; should re-try operation. \n
 *          IHA_RET_E_APPLET_FATAL: Error in FW / embedded app that requires
 *              a reset of any state-full operations. \n
 *          IHA_RET_E_APPLET_FILE_ERROR: Embedded app could not be loaded
 *              into the FW as it was missing, or there were other issues
 *              with the file. Will need to be re-installed using
 *              IHA_Install(). \n
 *          IHA_RET_E_APPLET_AUTH_FAILED: Embedded app not authorized to run
 *              on this FW. \n
 *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
 *          IHA_RET_E_OUTPUT_BEFFER_LENGTH_INSUFF: The output buffer length is
 *          insufficient, and the required length is stored in pEncTokenLength.
 *          Function needs to be called again with the correct buffer size.
 *****************************************************************************/
#ifdef ANDROID
UINT32 IHA3_GetCapabilities(
                        IN      UINT32  instanceId,
                        IN      UINT16  type,
                        OUT     UINT32 *pCaps);
#else
IHA_WIN32DLL_API UINT32 IHA3_GetCapabilities(
                        IN      UINT32  instanceId,
                        IN      UINT16  type,
                        OUT     UINT32 *pCaps);
#endif
#ifdef __cplusplus
}
#endif

#endif // _IHA_H

