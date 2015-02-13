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
// This enables ALOGV output
//#define LOG_NDEBUG 0
#define LOG_TAG "SEPService"

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

#include "debug.h"
// IPT header files
#include "iha.h"
#include "ihaError.h"

//namespace intel {
//    namespace security {

using namespace android;

// Interface - Shared by server and client
#include "ISecurity.h"
#include "SecurityService.h"


static inline int getCallingPid() {
    return IPCThreadState::self()->getCallingPid();
}

static inline int getCallingUid() {
    return IPCThreadState::self()->getCallingUid();
}

void SecurityService::instantiate() {
    defaultServiceManager()->addService(SEP_SERVICE_NAME, new SecurityService());
}

SecurityService::SecurityService() {
    ALOGV("%s started (pid=%d)", SRV_NAME, getpid());
}

SecurityService::~SecurityService() {
}

//
// IPT host Agent Interface
//
uint32_t SecurityService::IHAInit() {

    return IHA_Init();
}

uint32_t SecurityService::IHADeInit() {

    return IHA_DeInit();

}

uint32_t SecurityService::IHAStartProvisioning(const char * const sAppName,
                           uint32_t *pSessionHandle){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_StartProvisioning( const_cast<char *>(sAppName), pSessionHandle );

    return result;
}

uint32_t SecurityService::IHAEndProvisioning( const char * const sAppName,
                          const uint32_t hSessionHandle,
                          uint16_t * const pExpectedDataLen,
                          uint8_t * const pEncToken ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_EndProvisioning( const_cast<char *>(sAppName), hSessionHandle, pExpectedDataLen, pEncToken );
    LEAVE("result = %d\n", result);

    return result;
}

uint32_t SecurityService::IHASendData( const char * const sAppName,
                       const uint32_t hSessionHandle,
                       const uint16_t dataType,
                       const uint16_t dataLength,
                       const uint8_t * const pData){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_SendData( const_cast<char *>(sAppName), hSessionHandle, dataType, dataLength,
               const_cast<uint8_t*>(pData) );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAReceiveData( const char * const sAppName,
                      const uint32_t hSessionHandle,
                      const uint16_t dataType,
                      uint16_t * const pDataLength,
                      uint8_t * const pData){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_ReceiveData( const_cast<char *>(sAppName), hSessionHandle, dataType, pDataLength,
                  pData);

    LEAVE("result = %d\n", result);
    return result;
}

uint8_t SecurityService::IHAProcessSVPMessage( const char * const sAppName,
                           const uint32_t hSessionHandle,
                           const uint16_t inDataLength,
                           const uint8_t *const pData ) {
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    ENTER("sAppName=%s hSessionHandle=%u, inDataLength=%d \n", sAppName, hSessionHandle, inDataLength);
    result = IHA_ProcessSVPMessage( const_cast<char *>(sAppName), hSessionHandle,
                    inDataLength, const_cast<uint8_t *>(pData) );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetSVPMessage( const char * const sAppName,
                        const uint32_t hSessionHandle,
                        uint16_t * const pDataLength,
                        uint8_t * const pOutData ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetSVPMessage( const_cast<char *>(sAppName), hSessionHandle,
                pDataLength, pOutData );
    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHASendAndReceiveData( const char * const sAppName,
                         const uint32_t hSessionHandle,
                         const uint16_t dataType,
                         const uint16_t inDataLength,
                         const uint8_t * const pInData,
                         uint16_t * const pOutDataLength,
                         uint8_t * const pOutData ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_SendAndReceiveData( const_cast<char *>(sAppName), hSessionHandle,
                     dataType, inDataLength, const_cast<uint8_t *>(pInData),
                     pOutDataLength, pOutData );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetCapabilities( const char * const sAppName,
                          const uint16_t type,
                          uint16_t * const pDataLength,
                          uint8_t * const pData ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetCapabilities( const_cast<char *>(sAppName),
                  type, pDataLength, pData );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetVersion( uint32_t * const pVersion ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetVersion( pVersion );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAInstall( const char * const sAppName,
                      const wchar_t * const strSrcFile){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_Install( const_cast<char *>(sAppName), const_cast<wchar_t*>(strSrcFile) );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAUninstall( const char * const sAppName ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_Uninstall( const_cast<char *>(sAppName) );

    LEAVE("result = %d\n", result);
    return result;
}



uint32_t SecurityService::IHADoFWUpdate(){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_DoFWUpdate();

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetOTP( const char * const sAppName,
                     const uint32_t hSessionHandle,
                     const uint16_t encTokenLength,
                     const uint8_t  * const pEncToken,
                     const uint16_t vendorDataLength,
                     const uint8_t  * const pVendorData,
                     uint16_t * const pOTPLength,
                     uint8_t * const pOTP,
                     uint16_t * const pOutEncTokenLength,
                     uint8_t * const pOutEncToken ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetOTP( const_cast<char *>(sAppName), hSessionHandle,
             encTokenLength, const_cast<uint8_t*>(pEncToken),
             vendorDataLength, const_cast<uint8_t*>(pVendorData),
             pOTPLength, pOTP, pOutEncTokenLength, pOutEncToken );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetOTPSStatus( const char * const sAppName,
                        const uint32_t hSessionHandle,
                        const uint16_t type,
                        uint32_t * const pStatus ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetOTPSStatus( const_cast<char *>(sAppName), hSessionHandle,
                type, pStatus);

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetOTPCapabilities( const char * const sAppName,
                         const uint16_t sType,
                         uint16_t * const pDataLength,
                         uint8_t * const pData ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetOTPCapabilities( const_cast<char *>(sAppName),
                     sType, pDataLength, pData );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAInstallOTPS( const char * const sAppName,
                      const wchar_t * const strSrcFile) {
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_InstallOTPS( const_cast<char *>(sAppName),
                  const_cast<wchar_t *>(strSrcFile) );

    LEAVE("result = %d\n", result);
    return result;
}


uint32_t SecurityService::IHAUninstallOTPS( const char * const sAppName ) {
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_UninstallOTPS( const_cast<char *>(sAppName) );

    LEAVE("result = %d\n", result);
    return result;
}

uint32_t SecurityService::IHAGetAppInstId( const char * const sAppName,
                       void **ppAppletInstId,
                       uint32_t * const pFlags ){
    uint32_t result = IHA_RET_E_INTERNAL_ERROR;

    result = IHA_GetAppInstId( const_cast<char *>(sAppName),
                   ppAppletInstId, pFlags );

    LEAVE("result = %d\n", result);
    return result;
}
