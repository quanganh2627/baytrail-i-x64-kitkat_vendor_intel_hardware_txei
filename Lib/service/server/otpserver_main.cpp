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
#define LOG_NDEBUG 0
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

using namespace android;

// Interface (our AIDL) - Shared by server and client
#include "ISecurity.h"
#include "SecurityService.h"


int main() {

    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGV("ServiceManager: %p", sm.get());

    SecurityService::instantiate();

    android::ProcessState::self()->startThreadPool();
    ALOGD("%s  is now ready", SRV_NAME);
    IPCThreadState::self()->joinThreadPool();
    ALOGD("service thread joined");

    return EXIT_SUCCESS;
}
