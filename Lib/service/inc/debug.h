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

#ifndef __SEP_PROXY_DEBUG_HDR__
#define  __SEP_PROXY_DEBUG_HDR__

#include <log/log.h>

// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#ifdef DEBUG
#define ENTER(msg, arg...)             \
    do {                        \
    ALOGD("%s(): ENTER - " msg, __func__, ##arg);   \
    } while(0)

#define LEAVE(msg, arg...)             \
    do {                        \
    ALOGD("%s(): LEAVE - " msg, __func__, ##arg);   \
    } while(0)

#define LOGDBG(fmt, arg...)                    \
    do {                            \
    ALOGD( "%s():%d: " fmt, __func__, __LINE__, ##arg );    \
    } while(0)

#define LOGERR(fmt, arg...)                        \
    do {                                \
    ALOGE( "%s():%d: ERROR: " fmt, __func__, __LINE__, ##arg ); \
    } while(0)

#define PLOG aout
#define PRINTPARCEL( msg, parcel)          \
    do {                        \
        aout << __func__ << msg;        \
        parcel.print(aout);         \
        endl(aout);             \
    } while (0)

inline void PRINTMEM(const char *msg, const uint8_t *const buf, const uint16_t len) {
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define HEX_BYTES_PER_LINE (16)
#define CHARACTERS_PER_HEX_BYTE (3)  // 2 chars + 1 space
    // 16 bytes * (2 chars + 1 space) + newline
    char line[HEX_BYTES_PER_LINE * CHARACTERS_PER_HEX_BYTE + 1];
    uint8_t *ptr = const_cast<uint8_t *>(buf);
    int lc = (len + (HEX_BYTES_PER_LINE - 1)) / HEX_BYTES_PER_LINE;
    int i = 0, j = 0, nc = 0;
    uint16_t remaining = len;
    ALOGD("%s\n", (msg));
    for (i = 0; i < lc; i++) {
        uint8_t linelen = min(remaining, HEX_BYTES_PER_LINE);
        remaining -= HEX_BYTES_PER_LINE;
        for (j = 0; j < linelen; j++) {
            nc += snprintf(line + nc, sizeof(line) - nc,
                           "%s%2.2X", j ? " " : "", *(ptr + j));
        }
        ALOGD("%s\n", line);
        nc = 0;
        ptr += HEX_BYTES_PER_LINE;
    }
}

#else // !DEBUG

#define ENTER(msg, arg...)         \
    do {                    \
    } while(0)

#define LEAVE(msg, arg...)         \
    do {                    \
    } while(0)

#define LOGDBG(fmt, arg...)            \
    do {                    \
    } while(0)

#define LOGERR(fmt, arg...)            \
    do {                                \
    ALOGE( "%s():%d: ERROR: " fmt, __func__, __LINE__, ##arg ); \
    } while(0)

#define PRINTPARCEL( msg, parcel)          \
    do {                        \
    } while (0)

inline void PRINTMEM(const char *msg, const uint8_t *const buf, const uint16_t len) {
}
#endif // DEBUG

#endif //  __SEP_PROXY_DEBUG_HDR__
