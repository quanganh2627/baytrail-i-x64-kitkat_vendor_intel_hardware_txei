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

using namespace android;

class SecurityService : public BnSecurityService {
public:
    SecurityService();
    virtual ~SecurityService();


    static  void instantiate();

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

    virtual uint32_t IHAGetAppInstId( const char * const sAppName ,
				      void **ppAppletInstId,
				      uint32_t *pFlags );

};

