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
package com.intel.host.ipt.iha;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;


/**
 * @author nshuster
 *
 */
public class IHAManager {
    private static final String TAG = IHAManager.class.getSimpleName() + "N";
    private static boolean isInitialized = false;
    private final static int MAX_APPNAME_LENGTH = 300;
    private final static int MAX_SEND_RECV_DATA_LENGTH = 5000;
    private final static int MAX_ENCR_TOKEN_LENGTH = 300;
    private final static int MAX_SRC_FILE_LENGTH = 300;
    private final static int MAX_GET_CAPS_STRING_LENGTH = 300;
    private final static int MAX_SVP_MESSAGE_LENGTH = 5000;
    private final static int IHA_RET_S_OK = 0;
    private static int IHAManagerError = IHA_RET_S_OK;
    private final static int INVALID_INPUT = 13;

    public static IHAManager getInstance() {
        Log.i(TAG, "IHAManager.getInstance() -- ");
        return new IHAManager();
    }


    private IHAManager() {
    }


    /**
     * Initialize internal dependencies. Should be called first and only once
     * after loading the iha library. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param   none
     *
     * @return  void
     *****************************************************************************/
    public void IHAInit() {
        Log.v(TAG, "IHAInit");
        try {
            IhaManagerJni.IHAInit();
        }
        catch (IhaException e) {
            throw new RuntimeException("IHAInit failed", e);
        }
    }

    /**
     * Deinitialize IHA library. Should be called before unloading the DLL. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param   none
     *
     * @return  void
     *****************************************************************************/
    public void IHADeInit() {
        Log.v(TAG, "IHADeInit");

        try {
            IhaManagerJni.IHADeInit();
        }
        catch (IhaException e) {
            throw new RuntimeException("IHADeInit failed", e);
        }
    }

    public int IHAStartProvisioning(String strAppName) {
        Log.v(TAG, "IHAStartProvisioning");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }

        try {
            return IhaManagerJni.IHAStartProvisioning( strAppName );
        }
        catch (IhaException e) {
            throw new RuntimeException("IHAStartProvisioning failed", e);
        }
    }

    public byte[] IHAEndProvisioning( String strAppName, int iSessionHandle,
                                      short[] saExpectedTokenLen ) {
        Log.v(TAG, "IHAEndProvisioning");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (saExpectedTokenLen.length == 0 || saExpectedTokenLen.length > MAX_ENCR_TOKEN_LENGTH) {
            throw new RuntimeException("Invalid ExpectedTokenLenApp array length");
        }
        byte[] orig_encrToken;
        try {
            orig_encrToken = IhaManagerJni.IHAEndProvisioning( strAppName, iSessionHandle, saExpectedTokenLen);
            Log.d(TAG, "IHAEndProvisioning returning token len: " + saExpectedTokenLen[0]);
            return orig_encrToken;

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAEndProvisioning failed", e);
        }
    }

    /**
     * This function is used to send data to the embedded app in the FW. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence. Optional.
     * @param [in]      sDataType: Type of data being sent.
     * @param [in]      baInData: Input data bugger to be sent to the embedded app
     *                  in the FW.
     *
     * @return          void
     *****************************************************************************/
    public void IHASendData( String strAppName,
                             int iSessionHandle,
                             short sDataType,
                             byte[] baInData) {


        Log.v(TAG, "IHASendData");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sDataType < 0) {
            throw new RuntimeException("Invalid DataType value");
        }
        if (baInData.length == 0 || baInData.length > MAX_SEND_RECV_DATA_LENGTH) {
            throw new RuntimeException("Invalid InData array length");
        }

        try {

            IhaManagerJni.IHASendData( strAppName, iSessionHandle, sDataType, baInData );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHASendData failed", e);
        }

    }


    /**
     * This function is used to obtain data from the embedded app in the FW. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence. Optional.
     * @param [in]      sDataType: Type of data being requested.
     * @param [in, out] saExpectedDataLen: Short 1 byte array holding the expected
     *                  length of the output data.  This field holds the actual
     *                  length of the data returned if expected length is
     *                  sufficient, and required length if expected length is
     *                  insufficient.
     *
     * @return          Byte array containing the data received from the FW.
     *****************************************************************************/
    public byte[] IHAReceiveData( String strAppName, int iSessionHandle,
                                  short sDataType, short[] saExpectedDataLen) {

        Log.v(TAG, "IHAReceiveData");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sDataType < 0) {
            throw new RuntimeException("Invalid DataType value");
        }
        if (saExpectedDataLen.length == 0 || saExpectedDataLen.length > MAX_SEND_RECV_DATA_LENGTH) {
            throw new RuntimeException("Invalid ExpectedDataLen array length");
        }

        try {
            byte[] recvData =  IhaManagerJni.IHAReceiveData( strAppName, iSessionHandle,
                                                             sDataType, saExpectedDataLen );
            return recvData;

        }
        catch (IhaException e) {
            throw new RuntimeException("IHASendData failed", e);
        }
    }


    /**
     * This function is used to send IPT-specific provisioning data to the
     * embedded app in the FW. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence.
     * @param [in]      baInData: Byte array of data to be sent to the FW.
     *
     * @return          void
     *****************************************************************************/
    public void IHAProcessSVPMessage(String strAppName, int iSessionHandle,
                                     byte[] baInData) {
        Log.v(TAG, "IHAProcessSVPMessage");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (baInData.length == 0 || baInData.length > MAX_SVP_MESSAGE_LENGTH) {
            throw new RuntimeException("Invalid InData array length");
        }

        try {

            IhaManagerJni.IHAProcessSVPMessage( strAppName, iSessionHandle, baInData);

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAProcessSVPMessage failed", e);
        }
    }


    /**
     * This function is used to obtain IPT-specific provisioning data from the
     * embedded app in the FW. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence.
     * @param [in, out] saExpectedDataLen: Short 1 byte array holding the expected
     *                  length of the output data.  This field holds the actual
     *                  length of the data returned if expected length is
     *                  sufficient, and required length if expected length is
     *                  insufficient.
     *
     * @return          Byte array containing the data returned from the FW.
     *****************************************************************************/
    public byte[] IHAGetSVPMessage( String strAppName, int iSessionHandle,
                                    short[] saExpectedDataLen) {

        Log.v(TAG, "IHAGetSVPMessage");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (saExpectedDataLen.length == 0 || saExpectedDataLen.length > MAX_SVP_MESSAGE_LENGTH) {
            throw new RuntimeException("Invalid ExpectedDataLen array length");
        }

        try {
            byte[] getSVPMsg =  IhaManagerJni.IHAGetSVPMessage( strAppName, iSessionHandle, saExpectedDataLen);
            return getSVPMsg;
        }
        catch (IhaException e) {
            throw new RuntimeException("IHAGetSVPMessage failed", e);
        }
    }

    /**
     * This function is used to send and receive data to and from the embedded app
     * in the FW. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence. Optional.
     * @param [in]      sDataType: Type of action that is being requested.
     * @param [in]      baInData: Input data bugger to be sent to the embedded app
     *                  in the FW.
     * @param [in, out] saExpectedOutDataLen: Short 1 byte array holding the
     *                  expected length of the output data.  This field holds the
     *                  actual length of the data returned if expected length is
     *                  sufficient, and required length if expected length is
     *                  insufficient.
     *
     * @return          Byte array containing the data returned from the FW.
     *****************************************************************************/
    public byte[] IHASendAndReceiveData(String strAppName,
                                        int iSessionHandle, short sDataType, byte[] baInData,
                                        short[] saExpectedOutDataLen) {

        Log.v(TAG, "IHASendAndReceiveData");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sDataType < 0) {
            throw new RuntimeException("Invalid DataType value");
        }
        if (baInData.length == 0 || baInData.length > MAX_SEND_RECV_DATA_LENGTH) {
            throw new RuntimeException("Invalid InData array length");
        }
        if (saExpectedOutDataLen.length == 0 || saExpectedOutDataLen. length > MAX_SEND_RECV_DATA_LENGTH) {
            throw new RuntimeException("Invalid ExpectedOutDataLen array length");
        }


        try {
            byte[] sendRecvData = IhaManagerJni.IHASendAndReceiveData( strAppName, iSessionHandle, sDataType,
                                                                       baInData, saExpectedOutDataLen);
            return sendRecvData;

        }
        catch (IhaException e) {
            throw new RuntimeException("IHASendAndReceiveData failed", e);
        }
    }

    /**
     * This function is used to obtain capabilities of embedded app, including
     * version information. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      sType: Type of capability information requested. Can be: \n
     1: Embedded App Version \n
     2: Embedded App Security Version.
     * @param [in, out] saExpectedDataLen: Short 1 byte array holding the
     *                  expected length of the output data.  This field holds the
     *                  actual length of the data returned if expected length is
     *                  sufficient, and required length if expected length is
     *                  insufficient.
     *                  For sType 1: Output is a string containing the version.
     *                      saExpectedDataLen[0] should be set to 6.
     *                  For sType 2. Output is a string containing the version.
     *                      saExpectedDataLen[0]should be set to 6.
     *
     * @return          Byte array containing the data returned from the FW.
     *****************************************************************************/
    public byte[] IHAGetCapabilities( String strAppName, short sType, short[] saExpectedDataLen) {

        Log.v(TAG, "IHAGetCapabilities");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sType < 0) {
            throw new RuntimeException("Invalid Type value");
        }
        if (saExpectedDataLen.length == 0 || saExpectedDataLen.length > MAX_GET_CAPS_STRING_LENGTH) {
            throw new RuntimeException("Invalid ExpectedDataLen array length");
        }

        try {
            byte[] getCaps =  IhaManagerJni.IHAGetCapabilities( strAppName, sType, saExpectedDataLen);
            return getCaps;

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAGetCapabilities failed", e);
        }
    }


    /**
     * This function is used to retrieve the version of the IHA DLL. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param   none
     *
     * @return  Version of the IHA DLL. Please see the IHA C interface
     *          documentation for details on the parsing this int.
     *****************************************************************************/
    public int IHAGetVersion() {

        Log.v(TAG, "IHAGetVersion");
        try {

            return IhaManagerJni.IHAGetVersion();

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAGetVersion failed", e);
        }
    }

    /**
     * This function is used to load the embedded app into the FW. This is
     * typically called the very first time the embedded app is used, if it has not
     * been loaded yet. It is also used every time the embedded app needs to be
     * updated. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      strSrcFile: Unicode string containing the full path,
     *                  including filename of the embedded app that needs to be
     *                  installed in the chipset.  Once this command returns
     *                  successfully, the folder and file that this path points to
     *                  can be deleted.
     *
     * @return          void
     *****************************************************************************/
    public static native void jni_IHAInstall( String strAppName,
                                              String strSrcFile) throws IhaException;
    void IHAInstall( String strAppName, String strSrcFile) {

        Log.v(TAG, "IHAInstall");
        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (strSrcFile.length() == 0 || strSrcFile.length() > MAX_SRC_FILE_LENGTH) {
            throw new RuntimeException("Invalid Source File Name argument");
        }

        try {

            jni_IHAInstall( strAppName, strSrcFile);

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAInstall failed", e);
        }
    }


    /**
     * This function is used to unload the embedded app from the FW. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     *
     * @return          void
     *****************************************************************************/
    void IHAUninstall( String strAppName) {
        Log.v(TAG, "IHAUninstall");
        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }

        try {

            IhaManagerJni.IHAUninstall( strAppName );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAUninstall failed", e);
        }
    }

    /**
     * This function is used to trigger a FW update when an EPID group key
     * revocation is detected. \n
     * This API was added in IPT 2.0, and is only to be used by those apps that do
     * not need to run on 2011 platforms. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param   none
     *
     * @return  void
     *****************************************************************************/
    void IHADoFWUpdate() {
        Log.v(TAG, "IHADoFWUpdate");
        try {

            IhaManagerJni.IHADoFWUpdate( );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHADoFWUpdate failed", e);
        }
    }


    /**
     * This function is used to retrieve an OTP from the embedded OTP App. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence. Must be 0 if called post-provisioning. If a valid
     *                  session handle is passed in, then this is to be used in the
     *                  context of a provisioning sequence, and no encrypted Token
     *                  should be passed in.
     * @param [in]      baEncToken: Byte array containing the encrypted Token.
     *                  Must be NULL if session handle is used.
     * @param [in]      baVendorData: Byte array containing the vendor data.
     *                  Optional.
     * @param [in, out] saExpectedOtpLength: Short 1 byte array holding the
     *                  expected length of the OTP.  This field holds the actual
     *                  length of the data returned if the call is successful,
     *                  and required length if expected length is insufficient.
     * @param [in, out] saExpectedEncTokenLength: Short 1 byte array holding the
     *                  expected length of the encrypted token record. This is
     *                  optional; if the caller does not require the encrypted
     *                  token to be sent back, this 1st byte can be set to 0.  If
     *                  it isset, and if this buffer is insufficient, the call will
     *                  return with an error indicating the same and this field
     *                  will specify the required buffer length.  The function will
     *                  need to be called again with the correct buffer size.  This
     *                  field must be 0 if session handle is used.
     *
     * @return          Byte array containing the data returned by the FW.  The
     *                  first part contains the OTP; if saOutEncTokenLength[0] is
     *                  greater than 0, the rest of the data in this buffer is the
     *                  output encrypted token.
     *****************************************************************************/
    public byte[] IHAGetOTP(  String strAppName, int iSessionHandle, byte[] baEncToken,
                              byte[] baVendorData, short[] saExpectedOtpLength,
                              short[] saExpectedEncTokenLength) {

        Log.v(TAG, "IHAGetOTP");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }

        try {
            byte[] getOTP = IhaManagerJni.IHAGetOTP( strAppName, iSessionHandle, baEncToken,
                                                     baVendorData, saExpectedOtpLength,
                                                     saExpectedEncTokenLength );
            return getOTP;

        }
        catch (IhaException e) {
            throw new RuntimeException("Interface1 IHAGetOTP failed", e);
        }
    }

    /**
     * This function is used to obtain status from the embedded app. \n
     * This is a legacy API, kept for backward compatibility with version 1.x.
     * Should not be used from version 2.0 onwards. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      iSessionHandle: Session handle identifying a provisioning
     *                  sequence. Can be NULL if session-specific status is not
     *                  required.
     * @param [in]      sType: Type of status requested.
     *
     * @return          Status returned by the FW.
     *****************************************************************************/
    public int IHAGetOTPSStatus( String strAppName, int iSessionHandle, short sType) {

        Log.v(TAG, "IHAGetOTPSStatus");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sType < 0) {
            throw new RuntimeException("Invalid DataType value");
        }

        try {
            return IhaManagerJni.IHAGetOTPSStatus( strAppName, iSessionHandle, sType );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAGetOTPSStatus failed", e);
        }
    }

    /**
     * This function is used to obtain capabilities of embedded app, including
     * version information. \n
     * This is a legacy API, kept for backward compatibility with version 1.x.
     * Has been replaced by IHA_GetCapabilities() in version 2.0 onwards. \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      sType: Type of capability information requested. Can be: \n
     1: Embedded App Version \n
     2: Embedded App Security Version.
     * @param [in, out] saExpectedDataLen: Short 1 byte array holding the
     *                  expected length of the output data.  This field holds the
     *                  actual length of the data returned if expected length is
     *                  sufficient, and required length if expected length is
     *                  insufficient.
     *                  For sType 1: Output is a string containing the version.
     *                      saExpectedDataLen[0] should be set to 6.
     *                  For sType 2. Output is a string containing the version.
     *                      saExpectedDataLen[0]should be set to 6.
     *
     * @return          Byte array containing the data returned from the FW.
     *****************************************************************************/
    public byte[] IHAGetOTPCapabilities( String strAppName, short sType, short[] saExpectedDataLen) {

        Log.v(TAG, "IHAGetOTPCapabilities");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (sType < 0) {
            throw new RuntimeException("Invalid DataType value");
        }
        if (saExpectedDataLen.length == 0 || saExpectedDataLen.length > MAX_GET_CAPS_STRING_LENGTH) {
            throw new RuntimeException("Invalid ExpectedDataLen array length");
        }

        try {
            byte[] getOtpCaps =  IhaManagerJni.IHAGetOTPCapabilities( strAppName, sType, saExpectedDataLen);
            return getOtpCaps;

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAGetOTP failed", e);
        }
    }

    /**
     * This function is used to load the embedded app into the FW. This is
     * typically called the very first time the embedded app is used, if it has not
     * been loaded yet. It is also used every time the embedded app needs to be
     * updated. \n
     * This is a legacy API, kept for backward compatibility with version 1.x.
     * Has been replaced by IHA_GetCapabilities() in version 2.0 onwards.  \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     * @param [in]      strSrcFile: Unicode string containing the full path,
     *                  including filename of the embedded app that needs to be
     *                  installed in the chipset.  Once this command returns
     *                  successfully, the folder and file that this path points to
     *                  can be deleted.
     *
     * @return          void
     *****************************************************************************/
    void IHAInstallOTPS( String strAppName, String strSrcFile) {
        Log.v(TAG, "IHAInstallOTPS");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        if (strSrcFile.length() == 0 || strSrcFile.length() > MAX_SRC_FILE_LENGTH) {
            throw new RuntimeException("Invalid Source File Name argument");
        }

        try {

            IhaManagerJni.IHAInstallOTPS( strAppName, strSrcFile);

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAInstallOTPS failed", e);
        }
    }

    /**
     * This function is used to unload the embedded app from the FW. \n
     * This is a legacy API, kept for backward compatibility with version 1.x.
     * Has been replaced by IHA_GetCapabilities() in version 2.0 onwards.  \n
     * Please see the IHA C interface documentation for error codes thrown via
     * IhaException.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     *
     * @return          void
     *****************************************************************************/
    public void IHAUninstallOTPS( String strAppName) {
        Log.v(TAG, "IHAUninstallOTPS");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }

        try {

            IhaManagerJni.IHAUninstallOTPS( strAppName );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHAUninstallOTPS failed", e);
        }
    }

    /*****************************************************************************
     * This function is used retrieve the embedded app's instance id. \n
     * This API was added in IPT 2.1, and is meant to be used by client applications
     * that need to reuse a non-shared embedded app instance id between multiple
     * IPT dll-s. Non-shared app instances are only created when the
     * IHA_RegisterEventCb() API is used to register an event callback with the app.
     *
     * @param [in]      strAppName: String identifying the embedded app in the
     *                  chipset.
     *
     * @return          Instance Id to use for communication with embedded app
     *****************************************************************************/
    public long IHA_GetAppInstId( String strAppName ) {
        Log.v(TAG, "IHA_GetAppInstId");

        if (strAppName.length() == 0 || strAppName.length() > MAX_APPNAME_LENGTH) {
            throw new RuntimeException("Invalid App Name argument");
        }
        try {

            return IhaManagerJni.IHA_GetAppInstId( strAppName );

        }
        catch (IhaException e) {
            throw new RuntimeException("IHA_GetAppInstId failed", e);
        }
    }

    public int IHAGetLastError() {
        Log.v(TAG, "IHAGetLastError");
        return 0;
    }
}
