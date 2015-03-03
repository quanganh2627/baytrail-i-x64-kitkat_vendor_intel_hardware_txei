/*************************************************************************
 **     Copyright (c) 2015 Intel Corporation. All rights reserved.      **
 **                                                                     **
 ** This Software is licensed pursuant to the terms of the INTEL        **
 ** MOBILE COMPUTING PLATFORM SOFTWARE LIMITED LICENSE AGREEMENT        **
 ** (OEM / IHV / ISV Distribution & End User)                           **
 **                                                                     **
 ** The above copyright notice and this permission notice shall be      **
 ** included in all copies or substantial portions of the Software.     **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,     **
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  **
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               **
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS **
 ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  **
 ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   **
 ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    **
 ** SOFTWARE.                                                           **
 **                                                                     **
 *************************************************************************/

#define LOG_TAG "SEPClient"

#include <stdlib.h>
#include <stdio.h>
#include "utils/RefBase.h"
#include "utils/Log.h"
#include "binder/TextOutput.h"
#include <cutils/log.h>

#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryHeapBase.h>

#include "debug.h"

// IPT
#include "ihaError.h"

//namespace intel {
//    namespace security {

using namespace android;

#include "ISecurity.h"



// Client
BpSecurity::BpSecurity(const sp<IBinder>& impl) :
   BpInterface<ISecurity>(impl) {
    ENTER("\n");
}

BpSecurity::~BpSecurity() {
    ENTER("\n");
}

// Helper function to get a hold of the "Demo" service.
sp<ISecurity> BpSecurity::getSecurityService() {
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0) {
        LOGERR("Unable to get handle for defaultServiceManager");
        return NULL;
    }

    sp<IBinder> binder = sm->getService(SEP_SERVICE_NAME);
    if (binder == 0) {
        LOGERR("Unable to get handle for service");
        return NULL;
    }
    sp<ISecurity> serv = interface_cast<ISecurity>(binder);
    if (serv == 0) {
        LOGERR("Unable to get handle for service");
        return NULL;
    }

    return serv;
}

//
// IPT host Agent Interface
//
uint32_t BpSecurity::IHAInit() {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    ENTER("\n");

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAInit, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);
    return result;
}
uint32_t BpSecurity::IHADeInit() {
    uint32_t result = IHA_RET_S_OK;

    Parcel data, reply;

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHADeInit, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);
    return result;
}
uint32_t BpSecurity::IHAStartProvisioning(const char *const sAppName,
                                          uint32_t *pSessionHandle) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL || pSessionHandle == NULL) {
        LOGERR("Invalid Input: AppName or SessionHandle is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAStartProvisioning, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pSessionHandle = reply.readInt32();
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}
uint32_t BpSecurity::IHAEndProvisioning(const char *const sAppName,
                                        const uint32_t hSessionHandle,
                                        uint16_t *pExpectedDataLen,
                                        uint8_t *pEncToken) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pExpectedDataLen == NULL || pEncToken == NULL) {
        LOGERR("Invalid Input: pExpectedDataLen or pEncToken is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(*pExpectedDataLen);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAEndProvisioning, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);
    if (result == IHA_RET_S_OK) {
        *pExpectedDataLen = reply.readInt32() & USHRT_MAX;
        //TODO: If the Token is typically > 1K maybe shared memory should be used.
        reply.read(pEncToken, *pExpectedDataLen);
    }
    return result;
}

uint32_t BpSecurity::IHASendData(const char *const sAppName,
                                 const uint32_t hSessionHandle,
                                 const uint16_t dataType,
                                 const uint16_t dataLength,
                                 const uint8_t *const pData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pData == NULL) {
        LOGERR("Invalid Input: pData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(dataType);
    data.writeInt32(dataLength);
    data.write(pData, dataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHASendData, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAReceiveData(const char *const sAppName,
                                    const uint32_t hSessionHandle,
                                    const uint16_t dataType,
                                    uint16_t *const pDataLength,
                                    uint8_t *const pData) {
    uint32_t result = IHA_RET_S_OK;

    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pDataLength == NULL) {
        LOGERR("Invalid Input: pDataLength is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pData == NULL) {
        LOGERR("Invalid Input: pData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(dataType);
    data.writeInt32(*pDataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAReceiveData, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pDataLength = reply.readInt32() & USHRT_MAX;
        reply.read(pData, *pDataLength);
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint8_t BpSecurity::IHAProcessSVPMessage(const char *const sAppName,
                                         const uint32_t hSessionHandle,
                                         const uint16_t inDataLength,
                                         const uint8_t *const pData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    ENTER("sAppName=%s hSessionHandle=%u, inDataLength=%d \n", sAppName, hSessionHandle, inDataLength);

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(inDataLength);
    data.write(pData, inDataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAProcessSVPMessage, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAGetSVPMessage(const char *const sAppName,
                                      const uint32_t hSessionHandle,
                                      uint16_t *const pDataLength,
                                      uint8_t *const pOutData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pDataLength == NULL) {
        LOGERR("Invalid Input: pDataLength is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pOutData == NULL) {
        LOGERR("Invalid Input: pOutData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(*pDataLength);

    LOGDBG("sAppName=%s hSessionHandle=%u, inDataLength=%d \n", sAppName, hSessionHandle, *pDataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetSVPMessage, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pDataLength = reply.readInt32() & USHRT_MAX;
        reply.read(pOutData, *pDataLength);
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHASendAndReceiveData(const char *const sAppName,
                                           const uint32_t hSessionHandle,
                                           const uint16_t dataType,
                                           const uint16_t inDataLength,
                                           const uint8_t *const pInData,
                                           uint16_t *const pOutDataLength,
                                           uint8_t *const pOutData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pInData == NULL) {
        LOGERR("Invalid Input: pInData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pOutDataLength == NULL) {
        LOGERR("Invalid Input: pOutDataLength is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pOutData == NULL) {
        LOGERR("Invalid Input: pOutData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(dataType);
    data.writeInt32(inDataLength);
    data.write(pInData, inDataLength);
    data.writeInt32(*pOutDataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHASendAndReceiveData, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pOutDataLength = reply.readInt32() & USHRT_MAX;
        reply.read(pOutData, *pOutDataLength);
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAGetCapabilities(const char *const sAppName,
                                        const uint16_t type,
                                        uint16_t *const pDataLength,
                                        uint8_t *const pData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pDataLength == NULL || *pDataLength == 0) {
        LOGERR("Invalid Input: pDataLength is NULL or is zero\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pData == NULL) {
        LOGERR("Invalid Input: pData is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(type);
    data.writeInt32(*pDataLength);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetCapabilities, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pDataLength = reply.readInt32() & USHRT_MAX;
        reply.read(pData, *pDataLength);
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAGetVersion(uint32_t *const pVersion) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (pVersion == NULL) {
        LOGERR("Invalid Input: pVersion is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetVersion, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pVersion = reply.readInt32();
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAInstall(const char *const sAppName,
                                const wchar_t *const strSrcFile) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;
    /*
     * This function is not implemented for Android in the IHA
     * library, return NOT_IMPLEMENTED here instead of making the
     * call.
     */
    LOGDBG("Not Implemented!\n");
    //TODO: When implemented how is wchar_t handled?
    return IHA_RET_E_NOT_IMPLEMENTED;
}

uint32_t BpSecurity::IHAUninstall(const char *const sAppName) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    /*
     * This function is not implemented for Android in the IHA
     * library, return NOT_IMPLEMENTED here instead of making the
     * call.
     */
    LOGDBG("Not Implemented!\n");
    return IHA_RET_E_NOT_IMPLEMENTED;
}



uint32_t BpSecurity::IHADoFWUpdate() {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHADoFWUpdate, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAGetOTP(const char *const sAppName,
                               const uint32_t hSessionHandle,
                               const uint16_t encTokenLength,
                               const uint8_t  *const pEncToken,
                               const uint16_t vendorDataLength,
                               const uint8_t  *const pVendorData,
                               uint16_t *const pOTPLength,
                               uint8_t *const pOTP,
                               uint16_t *const pOutEncTokenLength,
                               uint8_t *const pOutEncToken) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (encTokenLength == 0) {
        LOGERR("Invalid Input: encTokenLength is 0\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    //TODO If this is NULL is the length 0?
    if (pEncToken == NULL) {
        LOGDBG("pEncToken is NULL\n");
    }
    if (vendorDataLength == 0) {
        LOGERR("Invalid Input: VendorDataLength is 0\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    //TODO If this is NULL is the length 0?
    if (pVendorData == NULL) {
        LOGDBG("pVendorData is NULL\n");
    }
    if (pOTPLength == NULL || *pOTPLength == 0) {
        LOGERR("Invalid Input: pOTPLength is NULL or 0\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pOTP == NULL) {
        LOGERR("Invalid Input: pOTP is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    if (pEncToken == NULL) {
        data.writeInt32(0);
    } else {
        data.writeInt32(encTokenLength);
        data.write(pEncToken, encTokenLength);
    }
    if (pVendorData == NULL) {
        data.writeInt32(0);
    } else {
        data.writeInt32(vendorDataLength);
        data.write(pVendorData, vendorDataLength);
    }
    data.writeInt32(*pOTPLength);
    if (pOutEncTokenLength == NULL) {
        data.writeInt32(0);
    } else {
        data.writeInt32(*pOutEncTokenLength);
    }

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetOTP, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        // Copy the OTP length and data back to the caller
        *pOTPLength = reply.readInt32() & USHRT_MAX;
        reply.read(pOTP, *pOTPLength);

        if (pOutEncTokenLength != NULL) {
            // Copy the encoded token length and data back to the caller
            *pOutEncTokenLength = reply.readInt32() & USHRT_MAX;
            reply.read(pOutEncToken, *pOutEncTokenLength);
        }
    }

    LEAVE("result=0x%0X\n", result);
    return result;
}

uint32_t BpSecurity::IHAGetOTPSStatus(const char *const sAppName,
                                      const uint32_t hSessionHandle,
                                      const uint16_t type,
                                      uint32_t *const pStatus) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    if (pStatus == NULL) {
        LOGERR("Invalid Input: pStatus is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }

    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);
    data.writeInt32(hSessionHandle);
    data.writeInt32(type);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetOTPSStatus, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();
    if (result == IHA_RET_S_OK) {
        *pStatus = reply.readInt32();
    }

    LEAVE("result=0x%0X\n", result);

    return result;
}

uint32_t BpSecurity::IHAGetOTPCapabilities(const char *const sAppName,
                                           const uint16_t sType,
                                           uint16_t *const pDataLength,
                                           uint8_t *const pData) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    if (sAppName == NULL) {
        LOGERR("Invalid Input: AppName is NULL\n");
        return IHA_RET_E_INVALID_INPUT;
    }
    // Insert parameters into the Parcel
    data.writeInterfaceToken(ISecurity::getInterfaceDescriptor());
    data.writeCString(sAppName);

    PRINTPARCEL(" parcel to be sent: \n", data);
    remote()->transact(TRANSACTION_IHAGetOTPCapabilities, data, &reply);
    PRINTPARCEL(" Reply parcel: \n", reply);

    result = reply.readInt32();

    LEAVE("result=0x%0X\n", result);

    return result;
}

uint32_t BpSecurity::IHAInstallOTPS(const char *const sAppName,
                                    const wchar_t *const strSrcFile) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    /*
     * This function is not implemented for Android in the IHA
     * library, return NOT_IMPLEMENTED here instead of making the
     * call.
     */
    LOGDBG("Not Implemented!\n");
    return IHA_RET_E_NOT_IMPLEMENTED;
}


uint32_t BpSecurity::IHAUninstallOTPS(const char *const sAppName) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    /*
     * This function is not implemented for Android in the IHA
     * library, return NOT_IMPLEMENTED here instead of making the
     * call.
     */
    LOGDBG("Not Implemented!\n");
    return IHA_RET_E_NOT_IMPLEMENTED;
}

uint32_t BpSecurity::IHAGetAppInstId(const char *const sAppName,
                                     void **ppAppletInstId,
                                     uint32_t *pFlags) {
    uint32_t result = IHA_RET_S_OK;
    Parcel data, reply;

    /*
     * This function is not implemented for Android in the IHA
     * library, return NOT_IMPLEMENTED here instead of making the
     * call.
     */
    LOGDBG("Not Implemented!\n");
    return IHA_RET_E_NOT_IMPLEMENTED;
}

IMPLEMENT_META_INTERFACE(Security, SEP_SERVICE_FULLNAME);


//--------------------------------------------------

status_t BnSecurityService::onTransact(
   uint32_t code, const Parcel& data, Parcel *const reply, uint32_t flags) {
    LOGDBG("code = %u\n", code);

    switch (code) {
    case TRANSACTION_IHAInit:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            uint32_t result = IHAInit();

            // Insert the result into the reply parcel
            reply->writeInt32(result);

            return OK;
        }
        break;

    case TRANSACTION_IHADeInit:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            uint32_t result = IHADeInit();
            // Insert the result into the reply parcel
            reply->writeInt32(result);

            return OK;
        }
        break;

    case TRANSACTION_IHAStartProvisioning:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            uint32_t sessionHandle = 0;
            const char *const sAppName = data.readCString();

            uint32_t result = IHAStartProvisioning(sAppName, &sessionHandle);
            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(sessionHandle);
            }

            return OK;
        }
        break;

    case TRANSACTION_IHAEndProvisioning:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            uint16_t expectedDataLen = data.readInt32() & USHRT_MAX;
            if (expectedDataLen == 0) {
                LOGERR("expectedDataLen == 0\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pEncToken = new uint8_t[expectedDataLen];
            if (pEncToken == NULL) {
                LOGERR("Unable to allocate %d bytes of memory for EncToken\n", expectedDataLen);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }

            uint32_t result = IHAEndProvisioning(sAppName, hSessionHandle, &expectedDataLen,
                                                 pEncToken);
            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(expectedDataLen);
                reply->write(pEncToken, expectedDataLen);
            }

            delete[] pEncToken;

            return OK;
        }
        break;

    case TRANSACTION_IHASendData:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            uint16_t dataType = data.readInt32() & USHRT_MAX;
            uint16_t dataLength = data.readInt32() & USHRT_MAX;
            if (dataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[dataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", dataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }

            data.read(pData, dataLength);

            uint32_t result = IHASendData(sAppName, hSessionHandle, dataType, dataLength, pData);

            reply->writeInt32(result);

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHAReceiveData:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            uint16_t dataType = data.readInt32() & USHRT_MAX;
            uint16_t dataLength = data.readInt32() & USHRT_MAX;
            if (dataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[dataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", dataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }

            uint32_t result = IHAReceiveData(sAppName, hSessionHandle, dataType, &dataLength, pData);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(dataLength);
                reply->write(pData, dataLength);
            }

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHAProcessSVPMessage:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            const uint16_t inDataLength = data.readInt32() & USHRT_MAX;
            if (inDataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[inDataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", inDataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }
            data.read(pData, inDataLength);

            LOGDBG("sAppName=%s hSessionHandle=%u, inDataLength=%d \n", sAppName, hSessionHandle, inDataLength);

            uint32_t result = IHAProcessSVPMessage(sAppName, hSessionHandle, inDataLength, pData);

            LOGDBG("result=0x%0X\n", result);
            reply->writeInt32(result);

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHAGetSVPMessage:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            uint16_t dataLength = data.readInt32() & USHRT_MAX;
            if (dataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[dataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", dataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }
            uint32_t result = IHAGetSVPMessage(sAppName, hSessionHandle, &dataLength, pData);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(dataLength);
                reply->write(pData, dataLength);
            }

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHASendAndReceiveData:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            const uint16_t dataType = data.readInt32() & USHRT_MAX;
            const uint16_t inDataLength = data.readInt32() & USHRT_MAX;
            if (inDataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pInData = new uint8_t[inDataLength];
            if (pInData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", inDataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }
            data.read(pInData, inDataLength);
            uint16_t outDataLength = data.readInt32() & USHRT_MAX;

            uint8_t *pOutData = new uint8_t[outDataLength];
            if (pOutData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", outDataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                delete[] pInData;
                return OK;
            }

            uint32_t result = IHASendAndReceiveData(sAppName, hSessionHandle, dataType, inDataLength, pInData,
                                                    &outDataLength, pOutData);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(outDataLength);
                reply->write(pOutData, outDataLength);
            }

            delete[] pInData;
            delete[] pOutData;

            return OK;
        }
        break;

    case TRANSACTION_IHAGetCapabilities:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint16_t type = data.readInt32() & USHRT_MAX;
            uint16_t dataLength = data.readInt32() & USHRT_MAX;
            if (dataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[dataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", dataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }
            uint32_t result = IHAGetCapabilities(sAppName, type, &dataLength, pData);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(dataLength);
                reply->write(pData, dataLength);
            }

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHAGetVersion:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            uint32_t version;

            uint32_t result = IHAGetVersion(&version);
            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(version);
            }

            return OK;
        }
        break;

    case TRANSACTION_IHAInstall:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            LOGDBG("IHAInstall is not supported\n");
            reply->writeInt32(IHA_RET_E_NOT_IMPLEMENTED);

            return OK;
        }
        break;

    case TRANSACTION_IHAUninstall:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            LOGDBG("IHAUnInstall is not supported\n");
            reply->writeInt32(IHA_RET_E_NOT_IMPLEMENTED);

            return OK;
        }
        break;

    case TRANSACTION_IHADoFWUpdate:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            uint32_t result = IHADoFWUpdate();
            reply->writeInt32(result);

            return OK;
        }
        break;

    case TRANSACTION_IHAGetOTP:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            uint8_t *pEncToken = NULL;

            // Read the encoded token length, allocate memory and then
            // read the encoded token from the parcel
            const uint16_t encTokenLength = data.readInt32() & USHRT_MAX;
            if (encTokenLength == 0) {
                LOGDBG("encTokenLength = %d \n", encTokenLength);
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            } else {
                pEncToken = new uint8_t[encTokenLength];
                if (pEncToken == NULL) {
                    LOGERR("Unable to allocate %d bytes of memory\n", encTokenLength);
                    reply->writeInt32(IHA_RET_E_MEMORY);
                    return OK;
                }
                data.read(pEncToken, encTokenLength);
            }

            // Read the vendor data length, allocate memory and then read
            // the vendor data from the parcel
            const uint16_t vendorDataLength = data.readInt32() & USHRT_MAX;
            if (vendorDataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                delete[] pEncToken;
                return OK;
            }
            uint8_t *pVendorData = new uint8_t[vendorDataLength];
            if (pVendorData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", vendorDataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                delete[] pEncToken;
                return OK;
            }
            data.read(pVendorData, vendorDataLength);

            // Read the OTP length and allocate memory for the OTP.
            uint16_t OTPLength = data.readInt32() & USHRT_MAX;
            if (OTPLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                delete[] pEncToken;
                delete[] pVendorData;
                return OK;
            }
            uint8_t *pOTP = new uint8_t[OTPLength];
            if (pOTP == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", OTPLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                delete[] pEncToken;
                delete[] pVendorData;
                return OK;
            }

            // Read the out encoded token length and allocate memory for
            // the out encoded token.
            uint16_t outEncTokenLength = data.readInt32() & USHRT_MAX;
            uint8_t *pOutEncToken = NULL;
            if (outEncTokenLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                delete[] pEncToken;
                delete[] pVendorData;
                delete[] pOTP;
                return OK;
            } else {
                pOutEncToken = new uint8_t[outEncTokenLength];
                if (pOutEncToken == NULL) {
                    LOGERR("Unable to allocate %d bytes of memory\n", outEncTokenLength);
                    reply->writeInt32(IHA_RET_E_MEMORY);
                    delete[] pEncToken;
                    delete[] pVendorData;
                    delete[] pOTP;
                    return OK;
                }
            }

            uint32_t result = IHAGetOTP(sAppName, hSessionHandle, encTokenLength, pEncToken,
                                        vendorDataLength, pVendorData, &OTPLength, pOTP,
                                        &outEncTokenLength, pOutEncToken);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                // Write the OTP to the parcel
                reply->writeInt32(OTPLength);
                reply->write(pOTP, OTPLength);
                // Write the out encoded token to the parcel
                reply->writeInt32(outEncTokenLength);
                reply->write(pOutEncToken, outEncTokenLength);
            }

            // Clean up
            delete[] pEncToken;
            delete[] pVendorData;
            delete[] pOTP;
            delete[] pOutEncToken;

            return OK;
        }
        break;

    case TRANSACTION_IHAGetOTPSStatus:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint32_t hSessionHandle = data.readInt32();
            const uint16_t type = data.readInt32() & USHRT_MAX;
            uint32_t status = 0;

            uint32_t result = IHAGetOTPSStatus(sAppName, hSessionHandle, type, &status);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                // Write the status
                reply->writeInt32(status);
            }

            return OK;
        }
        break;

    case TRANSACTION_IHAGetOTPCapabilities:
        {
            CHECK_INTERFACE(ISecurity, data, reply);
            const char *const sAppName = data.readCString();
            const uint16_t sType = data.readInt32() & USHRT_MAX;
            uint16_t dataLength = data.readInt32() & USHRT_MAX;
            if (dataLength == 0) {
                LOGERR("cannot create 0 length array\n");
                reply->writeInt32(IHA_RET_E_INVALID_INPUT);
                return OK;
            }

            uint8_t *pData = new uint8_t[dataLength];
            if (pData == NULL) {
                LOGERR("Unable to allocate %d bytes of memory\n", dataLength);
                reply->writeInt32(IHA_RET_E_MEMORY);
                return OK;
            }
            uint32_t result = IHAGetOTPCapabilities(sAppName, sType, &dataLength, pData);

            reply->writeInt32(result);
            if (result == IHA_RET_S_OK) {
                reply->writeInt32(dataLength);
                reply->write(pData, dataLength);
            }

            delete[] pData;

            return OK;
        }
        break;

    case TRANSACTION_IHAInstallOTPS:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            LOGDBG("IHAInstall is not supported\n");
            reply->writeInt32(IHA_RET_E_NOT_IMPLEMENTED);

            return OK;
        }
        break;

    case TRANSACTION_IHAUninstallOTPS:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            LOGDBG("IHAInstall is not supported\n");
            reply->writeInt32(IHA_RET_E_NOT_IMPLEMENTED);

            return OK;
        }
        break;

    case TRANSACTION_IHAGetAppInstId:
        {
            CHECK_INTERFACE(ISecurity, data, reply);

            LOGDBG("IHAInstall is not supported\n");
            reply->writeInt32(IHA_RET_E_NOT_IMPLEMENTED);

            return OK;
        }
        break;

    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}




//} // namespace security
//} // namespace intel
