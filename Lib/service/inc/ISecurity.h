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

#ifndef __ISECURITY_HDR__
#define  __ISECURITY_HDR__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryHeapBase.h>

//namespace intel {
//namespace security {
/*
 * Service name definitions
 */
static const char * SRV_NAME = {"com.intel.security.service.ISEPService"};
static const android::String16 SEP_SERVICE_NAME = android::String16(SRV_NAME);
static const char * SEP_SERVICE_FULLNAME = {"com.intel.security.service.ISEPService"};

// ----------------------------------------------------------------------------
// The Security Service Interface (AIDL) - Shared by server and client
class ISecurity: public IInterface {
public:
    DECLARE_META_INTERFACE(Security);

    //
    // IPT host Agent Interface
    //
    /**************************************************************************
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
    virtual uint32_t IHAInit() = 0;

    /**************************************************************************
     * Deinitialize IHA library. Should be called before unloading the DLL.
     *
     * @param   none
     *
     * @return  UINT32 value as defined in ihaError.h. \n
     *          IHA_RET_S_OK: Initialization completed successfully. \n
     *          IHA_RET_E_INTERNAL_ERROR: Internal error occured. \n
     *****************************************************************************/
    virtual uint32_t IHADeInit() = 0;

    /**************************************************************************
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
    virtual uint32_t IHAStartProvisioning(const char * const sAppName,
                      uint32_t *pSessionHandle) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAEndProvisioning( const char * const sAppName,
                     const uint32_t hSessionHandle,
                     uint16_t *pExpectedDataLen,
                     uint8_t * pEncToken ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHASendData( const char * const sAppName,
                  const uint32_t hSessionHandle,
                  const uint16_t dataType,
                  const uint16_t dataLength,
                  const uint8_t * const pData) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAReceiveData( const char * const sAppName,
                     const uint32_t hSessionHandle,
                     const uint16_t dataType,
                     uint16_t * const pDataLength,
                     uint8_t * const pData ) = 0;
    /**************************************************************************
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
    virtual uint8_t IHAProcessSVPMessage( const char * const sAppName,
                      const uint32_t hSessionHandle,
                      const uint16_t inDataLength,
                      const uint8_t *const pData ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAGetSVPMessage( const char * const sAppName,
                       const uint32_t hSessionHandle,
                       uint16_t *pDataLength,
                       uint8_t *pOutData ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHASendAndReceiveData( const char * const sAppName,
                        const uint32_t hSessionHandle,
                        const uint16_t dataType,
                        const uint16_t inDataLength,
                        const uint8_t *pInData,
                        uint16_t *pOutDataLength,
                        uint8_t *pOutData ) = 0;

    /**************************************************************************
     * This function is used to obtain capabilities of embedded app, including
     * version information. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms.
     *
     * @param [in]      sAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      type: Type of capability information requested. Can be: \n
     *                      1: Embedded App Version \n
     *                      2: Embedded App Security Version.
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
    virtual uint32_t IHAGetCapabilities( const char * const sAppName,
                     const uint16_t type,
                     uint16_t *pDataLength,
                     uint8_t *pData ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAGetVersion( uint32_t *pVersion ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAInstall( const char * const sAppName,
                 const wchar_t * const strSrcFile) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAUninstall( const char * const sAppName ) = 0;

    /**************************************************************************
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
    virtual uint32_t IHADoFWUpdate() = 0;

#ifdef NOT_USED
    /**************************************************************************
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
    virtual uint32_t IHA_RegisterEventCb( const char *sAppName,
                                          const IHA_EventCbFunc EventCbFunc) = 0;

    /*************************************************************************
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
    virtual uint32_t IHA_UnregisterEventCb(const char *sAppName) = 0;

#endif // NOT_USED

    /**************************************************************************
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
    virtual uint32_t IHAGetOTP( const char * const sAppName,
                const uint32_t hSessionHandle,
                const uint16_t encTokenLength,
                const uint8_t  *pEncToken,
                const uint16_t vendorDataLength,
                const uint8_t  *pVendorData,
                uint16_t *pOTPLength,
                uint8_t *pOTP,
                uint16_t *pOutEncTokenLength,
                uint8_t *pOutEncToken) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAGetOTPSStatus( const char * const sAppName,
                       const uint32_t hSessionHandle,
                       const uint16_t type,
                       uint32_t *pStatus ) = 0;

    /**************************************************************************
     * This function is used to obtain capabilities of embedded app, including
     * version information. \n
     * This is a legacy API, kept for backward compatibility with version 1.x.
     * Has been replaced by IHA_GetCapabilities() in version 2.0 onwards.
     *
     * @param [in]      sAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      type: Type of capability information requested. Can be: \n
     *                      1: Embedded App Version \n
     *                      2: Embedded App Security Version.
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
    virtual uint32_t IHAGetOTPCapabilities( const char * const sAppName,
                        const uint16_t sType,
                        uint16_t *pDataLength,
                        uint8_t *pData) = 0;

    /**************************************************************************
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
    virtual uint32_t IHAInstallOTPS( const char * const sAppName,
                     const wchar_t * const strSrcFile) = 0;


    /**************************************************************************
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
    virtual uint32_t IHAUninstallOTPS( const char * const sAppName ) = 0;

    /**************************************************************************
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
     *               This is an optional parameter and can be set to NULL.
     *
     * @return  UINT32 value as defined in ihaError.h. \n
     *          IHA_RET_S_OK: Retrieved app instance id successfully. \n
     *          IHA_RET_E_APPLET_SESSION_UNAVAILABLE: No session is opened on this
     *              applet \n
     *          IHA_RET_E_COMMS_ERROR: Communication error with the underlying
     *              stack; should re-try operation. \n
     *          IHA_RET_E_INTERNAL_ERROR: Internal error. \n
     *****************************************************************************/
    virtual uint32_t IHAGetAppInstId( const char * const sAppName,
                      void **ppAppletInstId,
                      uint32_t *pFlags ) = 0;

    enum {
    TRANSACTION_IHAInit = IBinder::FIRST_CALL_TRANSACTION,
    TRANSACTION_IHADeInit,
    TRANSACTION_IHAStartProvisioning,
    TRANSACTION_IHAEndProvisioning,
    TRANSACTION_IHASendData,
    TRANSACTION_IHAReceiveData,
    TRANSACTION_IHAProcessSVPMessage,
    TRANSACTION_IHAGetSVPMessage,
    TRANSACTION_IHASendAndReceiveData,
    TRANSACTION_IHAGetCapabilities,
    TRANSACTION_IHAGetVersion,
    TRANSACTION_IHAInstall,
    TRANSACTION_IHAUninstall,
    TRANSACTION_IHADoFWUpdate,
    TRANSACTION_IHAGetOTP,
    TRANSACTION_IHAGetOTPSStatus,
    TRANSACTION_IHAGetOTPCapabilities,
    TRANSACTION_IHAInstallOTPS,
    TRANSACTION_IHAUninstallOTPS,
    TRANSACTION_IHAGetAppInstId,
    };
};

// ----------------------------------------------------------------------------
/*
 * The Security client class definition
 */
class BpSecurity: public BpInterface<ISecurity> {
public:
    BpSecurity(const sp<IBinder>& impl);
    ~BpSecurity();

    static sp<ISecurity> getSecurityService();

    //
    // IPT host Agent Interface
    //
    virtual uint32_t IHAInit();
    virtual uint32_t IHADeInit();
    virtual uint32_t IHAStartProvisioning(const char * const sAppName,
                      uint32_t *pSessionHandle);
    virtual uint32_t IHAEndProvisioning( const char * const sAppName,
                     const uint32_t hSessionHandle,
                     uint16_t *pExpectedDataLen,
                     uint8_t * pEncToken );
    virtual uint32_t IHASendData( const char * const sAppName,
                  const uint32_t hSessionHandle,
                  const uint16_t dataType,
                  const uint16_t dataLength,
                  const uint8_t * const pData);

    virtual uint32_t IHAReceiveData( const char * const sAppName,
                     const uint32_t hSessionHandle,
                     const uint16_t dataType,
                     uint16_t *pDataLength,
                     uint8_t *pData);

    virtual uint8_t IHAProcessSVPMessage( const char * const sAppName,
                      const uint32_t hSessionHandle,
                      const uint16_t inDataLength,
                      const uint8_t *const pData );

    virtual uint32_t IHAGetSVPMessage( const char * const sAppName,
                       const uint32_t hSessionHandle,
                       uint16_t *pDataLength,
                       uint8_t *pOutData );

    virtual uint32_t IHASendAndReceiveData( const char * const sAppName,
                        const uint32_t hSessionHandle,
                        const uint16_t dataType,
                        const uint16_t inDataLength,
                        const uint8_t *pInData,
                        uint16_t *pOutDataLength,
                        uint8_t *pOutData );

    virtual uint32_t IHAGetCapabilities( const char * const sAppName,
                     const uint16_t type,
                     uint16_t *pDataLength,
                     uint8_t *pData );

    virtual uint32_t IHAGetVersion( uint32_t *pVersion );

    virtual uint32_t IHAInstall( const char * const sAppName,
                 const wchar_t * const strSrcFile);

    virtual uint32_t IHAUninstall( const char * const sAppName );

    virtual uint32_t IHADoFWUpdate();

    virtual uint32_t IHAGetOTP( const char * const sAppName,
                const uint32_t hSessionHandle,
                const uint16_t encTokenLength,
                const uint8_t  *pEncToken,
                const uint16_t vendorDataLength,
                const uint8_t  *pVendorData,
                uint16_t *pOTPLength,
                uint8_t *pOTP,
                uint16_t *pOutEncTokenLength,
                uint8_t *pOutEncToken );

    virtual uint32_t IHAGetOTPSStatus( const char * const sAppName,
                       const uint32_t hSessionHandle,
                       const uint16_t type,
                       uint32_t *pStatus );

    virtual uint32_t IHAGetOTPCapabilities( const char * const sAppName,
                        const uint16_t sType,
                        uint16_t *pDataLength,
                        uint8_t *pData );

    virtual uint32_t IHAInstallOTPS( const char * const sAppName,
                     const wchar_t * const strSrcFile);


    virtual uint32_t IHAUninstallOTPS( const char * const sAppName );

    virtual uint32_t IHAGetAppInstId( const char * const sAppName,
                      void **ppAppletInstId,
                      uint32_t *pFlags );

private:
};

// ----------------------------------------------------------------------------

class BnSecurityService: public android::BnInterface<ISecurity>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* const reply,
                                    uint32_t flags = 0);
};

//}; // namespace
//}; // namespace intel


#endif // __ISECURITY_HDR__
