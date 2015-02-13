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
**    @file ihaError.h
**
**    @brief  Contains return codes for OTPF and IHA interfaces
**
**    @author Ranjit Narjala
**    @author Amol Kulkarni
**    @author Pradeep Sebestian
**
********************************************************************************
*/

#ifndef _IHA_ERROR_H
#define _IHA_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------
// IHA error codes for IPT 2.0
// ---------------------------------------------------------------

#define IHA_RET_S_OK                            0   // No errors during command processing

// Common to both applet and IHA DLL
#define IHA_RET_E_INTERNAL_ERROR                10  // Generic error
#define IHA_RET_E_OUTPUT_BUFFER_LENGTH_INSUFF   11  // Output buffer length insufficient
#define IHA_RET_E_MEMORY                        12  // Unable to allocate memory
#define IHA_RET_E_INVALID_INPUT                 13  // Invalid input received
#define IHA_RET_E_NOT_IMPLEMENTED               14  // API not implemented
#define IHA_RET_E_COMMS_ERROR                   15  // Communication error with the
                                                    // underlying stack
#define IHA_RET_R_INVALID_REQUEST               16  // Request cannot be executed in
                                                    // current state

// Used within IHA DLL
#define IHA_RET_E_LIBRARY_NOT_INITIALIZED       101 // IHA DLL is not initialized. Please
                                                    // call IHA_Init() before any other calls
// 2.0 errors
#define IHA_RET_E_FW_UPDATE_FAILED              105 // FW Update failed
#define IHA_RET_E_FW_UPDATE_IN_PROGRESS         106 // FW Update in progress
#define IHA_RET_E_FW_UPDATE_CALL_FAILED         107 // One of the calls to the underlying
                                                    // FW update components failed
#define IHA_RET_E_FW_UPDATE_NONE_AVAILABLE      108 // No FW Update available
#define IHA_RET_E_FW_UPDATE_SERVICE_BUSY        109 // The FW Update service is busy
#define IHA_RET_E_FW_UPDATE_DLL_NOT_READY       110 // The underlying FW Update component
                                                    // is unavailable
#define IHA_RET_E_NOT_REGISTERED_FOR_EVENTS     120 // eApp instance is not registered
                                                    // to receive events
#define IHA_RET_E_ALREADY_REGISTERED_FOR_EVENTS 121 // eApp instance is already registered
                                                    // to receive events, new callback rejected
#define IHA_RET_E_EVENTS_NOT_SUPPORTED          122 // eApp loaded in shared mode, hence
                                                    // events are not supported

// Used within IHA DLL to return meta-level applet errors
#define IHA_RET_E_APPLET_FATAL                  150 // An error in the eApp has occured that
                                                    // requires a reset of state-ful transactions
#define IHA_RET_E_APPLET_MISSING_SRC            151 // eApp install failed since the source file
                                                    // could not be found
#define IHA_RET_E_APPLET_AUTH_FAILED            152 // eApp could not be loaded into the FW as
                                                    // it is not authorized
#define IHA_RET_E_APPLET_FILE_ERROR             153 // eApp culd not be loaded into the FW as
                                                    // it was an invalid file or format
#define IHA_RET_E_APPLET_NOT_INSTALLED          154 // eApp not installed. Please call
                                                    // IHA_Install()
#define IHA_RET_E_INVALID_APPID                 155 // Invalid appID
#define IHA_RET_E_APPLET_INVALID_FILE_EXT       156 // Invalid file extention
#define IHA_RET_E_APPLET_FILE_ERROR_READ        157 // Could not read supplied applet file
#define IHA_RET_E_APPLET_INVALID_FORMAT         158 // Format of the eApp file is invalid
#define IHA_RET_E_APPLET_APPID_MISMATCH         159 // AppID of eApp does not match what is
                                                    // specified in the function call
#define IHA_RET_E_MAX_APPLETS_INSTALLED         160 // Max eApp limit reached; please try
                                                    // to install the eApp later, or first
                                                    // uninstall an eApp
#define IHA_RET_E_APPLET_INSTALL_FAILED         161 // eApp install failed
#define IHA_RET_E_APPLET_UNINSTALL_FAILED       162 // eApp uninstall failed
#define IHA_RET_E_APPLET_SESSION_UNAVAILABLE    163 // no eApp session has been opened

// 3.0 errors
#define IHA_RET_E_INVALID_INSTANCEID            170 // instanceId has no mapped JHI session handle
#define IHA_RET_E_CLOSING_INSTANCEID            171 // session associated with instanceId is being closed
#define IHA_RET_E_INVALID_PROCESS               172 // a process attempted to use an instanceId
                                                    // that it did not create

// Returned by applet
#define IHA_RET_E_INVALID_HANDLE                201
#define IHA_RET_E_UNEXPECTED_CALL               202
#define IHA_RET_E_CRYPTO                        203
#define IHA_RET_E_VALIDATION_FAILED             204
#define IHA_RET_E_OTPS_FAILED                   205
#define IHA_RET_E_PROV_FAILED                   206
#define IHA_RET_E_INVALID_TOKEN                 207
#define IHA_RET_E_INVALID_MESSAGE               208
#define IHA_RET_E_INVALID_TYPE                  209
#define IHA_RET_E_INVALID_LENGTH                210
#define IHA_RET_E_OTPS_NOT_READY                211
#define IHA_RET_E_SYSTEM_BUSY                   212
#define IHA_RET_E_PROV_INCOMPLETE               213
#define IHA_RET_E_PIN_MISMATCH                  214
#define IHA_RET_E_USER_AUTH_FAILED              215
#define IHA_RET_E_PIN_REQUIRED                  216
#define IHA_RET_E_PIN_POLICY_LENGTH             217
#define IHA_RET_E_PIN_POLICY_DISTINCT           218
#define IHA_RET_E_PIN_POLICY_CONSECUTIVE        219
#define IHA_RET_E_VERSION_UNSUPPORTED           220
#define IHA_RET_E_MSG_VERIFICATION_FAILED       221
#define IHA_RET_E_APPLET_INTERNAL_ERROR         222
#define IHA_RET_E_VENDOR_DATA_SIGNED            223
#define IHA_RET_E_VENDOR_DATA_ENC               224


// For internal use only; will not be propogated externally
#define IHA_RET_E_IHA_PROTO_ERROR               301

// ---------------------------------------------------------------
// IHA Error Codes in IPT 1.0; not used in IPT 2.0, maintained for
// backward compatibility
// ---------------------------------------------------------------

#define OTP_RET_S_OK                            0

// Common to both applet and IHA DLL
#define OTP_RET_E_INTERNAL_ERROR                10
#define OTP_RET_E_OUTPUT_BUFFER_LENGTH_INSUFF   11
#define OTP_RET_E_MEMORY                        12
#define OTP_RET_E_INVALID_INPUT                 13
#define OTP_RET_E_NOT_IMPLEMENTED               14
#define OTP_RET_E_COMMS_ERROR                   15

// Used within IHA DLL
#define OTP_RET_E_LIBRARY_NOT_INITIALIZED       101

// Used within IHA DLL to return meta-level applet errors
#define OTP_RET_E_APPLET_FATAL                  150
#define OTP_RET_E_APPLET_MISSING_SRC            151
#define OTP_RET_E_APPLET_AUTH_FAILED            152
#define OTP_RET_E_APPLET_FILE_ERROR             153
#define OTP_RET_E_APPLET_NOT_INSTALLED          154
#define OTP_RET_E_INVALID_APPID                 155

// Returned by applet
#define OTP_RET_E_INVALID_HANDLE                201
#define OTP_RET_E_UNEXPECTED_CALL               202
#define OTP_RET_E_CRYPTO                        203
#define OTP_RET_E_VALIDATION_FAILED             204
#define OTP_RET_E_OTPS_FAILED                   205
#define OTP_RET_E_PROV_FAILED                   206
#define OTP_RET_E_INVALID_TOKEN                 207
#define OTP_RET_E_INVALID_MESSAGE               208
#define OTP_RET_E_INVALID_TYPE                  209
#define OTP_RET_E_INVALID_LENGTH                210
#define OTP_RET_E_OTPS_NOT_READY                211
#define OTP_RET_E_SYSTEM_BUSY                   212
#define OTP_RET_E_PROV_INCOMPLETE               213

// For internal use only; will not be propogated externally
#define OTP_RET_E_IHA_PROTO_ERROR               301

// IPT 3.0

// IHART error codes
#define IHART_RET_E_INTERNAL_ERROR             401      // unknown internal error
#define IHART_RET_E_NOT_INITIALIZED            402      // IHART component has not been initialized
#define IHART_RET_E_DRIVER_ACCESS_DENIED       403      // Could not open handle to IEXP driver
#define IHART_RET_E_DRIVER_NOT_FOUND           404      // Could not find IEXP driver in system
#define IHART_RET_E_INSTANCE_UNAVAILABLE       405      // No instance of embedded app has been opened yet
                                                        // with this instance of IhaRt
#define IHART_RET_E_INSTANCE_ALREADY_AVAILABLE 406      // An instance of embedded app is already open
                                                        // with this instance of IhaRt
#define IHART_RET_E_DRIVER_DEVICEID_NOT_SET    407      // The IEXP driver's platform device id is not set,
                                                        // please see IhaRtInit documentation
#define IHART_RET_E_CANNOT_DEINIT_WITH_OPEN_INSTANCE\
                                               408      // Applet instance is open, please close with
                                                        // IhaRtUnloadInstace() or IhaRtStopInstance()
                                                        // before calling IhaRtDeinit()

// IHA  UMD related macros
#define FACILITY_IHA    2
#define FACILITY_IHART  3

#define IHA_TO_HRESULT(code) \
    ((HRESULT) (((unsigned long)((code==IHA_RET_S_OK ? STATUS_SEVERITY_SUCCESS : STATUS_SEVERITY_ERROR))<<31) | ((unsigned long)CUSTOM_HRESULT) | ((unsigned long)(FACILITY_IHA)<<16) | ((unsigned long)(code))) )
#define IHART_TO_HRESULT(code) \
    ((HRESULT) (((unsigned long)((code==IHA_RET_S_OK ? STATUS_SEVERITY_SUCCESS : STATUS_SEVERITY_ERROR))<<31) | ((unsigned long)CUSTOM_HRESULT) | ((unsigned long)(FACILITY_IHART)<<16) | ((unsigned long)(code))) )

#define HRESULT_TO_IHA(hr)      ((hr) & 0xFFFF)

#define IHA_HR_OK(hr) (IHA_RET_S_OK == HRESULT_TO_IHA(hr))


#ifdef __cplusplus
} // extern "C"
#endif

#endif //_IHA_ERROR_H
