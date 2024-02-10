#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "elfloader.h"

#ifdef ANDROID
    const char* curlName = "libcurl.so";
#else
    const char* curlName = "libcurl.so.4";
#endif

#define ALTNAME "libcurl-gnutls.so.4"
#define LIBNAME curl

#include "generated/wrappedcurltypes.h"

#include "wrappercallback.h"

#define LONG          0
#define OBJECTPOINT   10000
#define STRINGPOINT   10000
#define FUNCTIONPOINT 20000
#define OFF_T         30000
#define BLOB          40000
#define CINIT(name,type,number) CURLOPT_ ## name = type + number
#define CURLOPT(na,t,nu) na = t + nu

typedef enum {
  CINIT(WRITEDATA, OBJECTPOINT, 1),
  CINIT(URL, STRINGPOINT, 2),
  CINIT(PORT, LONG, 3),
  CINIT(PROXY, STRINGPOINT, 4),
  CINIT(USERPWD, STRINGPOINT, 5),
  CINIT(PROXYUSERPWD, STRINGPOINT, 6),
  CINIT(RANGE, STRINGPOINT, 7),
  CINIT(READDATA, OBJECTPOINT, 9),
  CINIT(ERRORBUFFER, OBJECTPOINT, 10),
  CINIT(WRITEFUNCTION, FUNCTIONPOINT, 11),
  CINIT(READFUNCTION, FUNCTIONPOINT, 12),
  CINIT(TIMEOUT, LONG, 13),
  CINIT(INFILESIZE, LONG, 14),
  CINIT(POSTFIELDS, OBJECTPOINT, 15),
  CINIT(REFERER, STRINGPOINT, 16),
  CINIT(FTPPORT, STRINGPOINT, 17),
  CINIT(USERAGENT, STRINGPOINT, 18),
  CINIT(LOW_SPEED_LIMIT, LONG, 19),
  CINIT(LOW_SPEED_TIME, LONG, 20),
  CINIT(RESUME_FROM, LONG, 21),
  CINIT(COOKIE, STRINGPOINT, 22),
  CINIT(HTTPHEADER, OBJECTPOINT, 23),
  CINIT(HTTPPOST, OBJECTPOINT, 24),
  CINIT(SSLCERT, STRINGPOINT, 25),
  CINIT(KEYPASSWD, STRINGPOINT, 26),
  CINIT(CRLF, LONG, 27),
  CINIT(QUOTE, OBJECTPOINT, 28),
  CINIT(HEADERDATA, OBJECTPOINT, 29),
  CINIT(COOKIEFILE, STRINGPOINT, 31),
  CINIT(SSLVERSION, LONG, 32),
  CINIT(TIMECONDITION, LONG, 33),
  CINIT(TIMEVALUE, LONG, 34),
  CINIT(CUSTOMREQUEST, STRINGPOINT, 36),
  CINIT(STDERR, OBJECTPOINT, 37),
  CINIT(POSTQUOTE, OBJECTPOINT, 39),
  CINIT(OBSOLETE40, OBJECTPOINT, 40),
  CINIT(VERBOSE, LONG, 41),
  CINIT(HEADER, LONG, 42),
  CINIT(NOPROGRESS, LONG, 43),
  CINIT(NOBODY, LONG, 44),
  CINIT(FAILONERROR, LONG, 45),
  CINIT(UPLOAD, LONG, 46),
  CINIT(POST, LONG, 47),
  CINIT(DIRLISTONLY, LONG, 48),
  CINIT(APPEND, LONG, 50),
  CINIT(NETRC, LONG, 51),
  CINIT(FOLLOWLOCATION, LONG, 52),
  CINIT(TRANSFERTEXT, LONG, 53),
  CINIT(PUT, LONG, 54),
  CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),
  CINIT(PROGRESSDATA, OBJECTPOINT, 57),
#define CURLOPT_XFERINFODATA CURLOPT_PROGRESSDATA
  CINIT(AUTOREFERER, LONG, 58),
  CINIT(PROXYPORT, LONG, 59),
  CINIT(POSTFIELDSIZE, LONG, 60),
  CINIT(HTTPPROXYTUNNEL, LONG, 61),
  CINIT(INTERFACE, STRINGPOINT, 62),
  CINIT(KRBLEVEL, STRINGPOINT, 63),
  CINIT(SSL_VERIFYPEER, LONG, 64),
  CINIT(CAINFO, STRINGPOINT, 65),
  CINIT(MAXREDIRS, LONG, 68),
  CINIT(FILETIME, LONG, 69),
  CINIT(TELNETOPTIONS, OBJECTPOINT, 70),
  CINIT(MAXCONNECTS, LONG, 71),
  CINIT(OBSOLETE72, LONG, 72),
  CINIT(FRESH_CONNECT, LONG, 74),
  CINIT(FORBID_REUSE, LONG, 75),
  CINIT(RANDOM_FILE, STRINGPOINT, 76),
  CINIT(EGDSOCKET, STRINGPOINT, 77),
  CINIT(CONNECTTIMEOUT, LONG, 78),
  CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),
  CINIT(HTTPGET, LONG, 80),
  CINIT(SSL_VERIFYHOST, LONG, 81),
  CINIT(COOKIEJAR, STRINGPOINT, 82),
  CINIT(SSL_CIPHER_LIST, STRINGPOINT, 83),
  CINIT(HTTP_VERSION, LONG, 84),
  CINIT(FTP_USE_EPSV, LONG, 85),
  CINIT(SSLCERTTYPE, STRINGPOINT, 86),
  CINIT(SSLKEY, STRINGPOINT, 87),
  CINIT(SSLKEYTYPE, STRINGPOINT, 88),
  CINIT(SSLENGINE, STRINGPOINT, 89),
  CINIT(SSLENGINE_DEFAULT, LONG, 90),
  CINIT(DNS_USE_GLOBAL_CACHE, LONG, 91),
  CINIT(DNS_CACHE_TIMEOUT, LONG, 92),
  CINIT(PREQUOTE, OBJECTPOINT, 93),
  CINIT(DEBUGFUNCTION, FUNCTIONPOINT, 94),
  CINIT(DEBUGDATA, OBJECTPOINT, 95),
  CINIT(COOKIESESSION, LONG, 96),
  CINIT(CAPATH, STRINGPOINT, 97),
  CINIT(BUFFERSIZE, LONG, 98),
  CINIT(NOSIGNAL, LONG, 99),
  CINIT(SHARE, OBJECTPOINT, 100),
  CINIT(PROXYTYPE, LONG, 101),
  CINIT(ACCEPT_ENCODING, STRINGPOINT, 102),
  CINIT(PRIVATE, OBJECTPOINT, 103),
  CINIT(HTTP200ALIASES, OBJECTPOINT, 104),
  CINIT(UNRESTRICTED_AUTH, LONG, 105),
  CINIT(FTP_USE_EPRT, LONG, 106),
  CINIT(HTTPAUTH, LONG, 107),
  CINIT(SSL_CTX_FUNCTION, FUNCTIONPOINT, 108),
  CINIT(SSL_CTX_DATA, OBJECTPOINT, 109),
  CINIT(FTP_CREATE_MISSING_DIRS, LONG, 110),
  CINIT(PROXYAUTH, LONG, 111),
  CINIT(FTP_RESPONSE_TIMEOUT, LONG, 112),
#define CURLOPT_SERVER_RESPONSE_TIMEOUT CURLOPT_FTP_RESPONSE_TIMEOUT
  CINIT(IPRESOLVE, LONG, 113),
  CINIT(MAXFILESIZE, LONG, 114),
  CINIT(INFILESIZE_LARGE, OFF_T, 115),
  CINIT(RESUME_FROM_LARGE, OFF_T, 116),
  CINIT(MAXFILESIZE_LARGE, OFF_T, 117),
  CINIT(NETRC_FILE, STRINGPOINT, 118),
  CINIT(USE_SSL, LONG, 119),
  CINIT(POSTFIELDSIZE_LARGE, OFF_T, 120),
  CINIT(TCP_NODELAY, LONG, 121),
  CINIT(FTPSSLAUTH, LONG, 129),
  CINIT(IOCTLFUNCTION, FUNCTIONPOINT, 130),
  CINIT(IOCTLDATA, OBJECTPOINT, 131),
  CINIT(FTP_ACCOUNT, STRINGPOINT, 134),
  CINIT(COOKIELIST, STRINGPOINT, 135),
  CINIT(IGNORE_CONTENT_LENGTH, LONG, 136),
  CINIT(FTP_SKIP_PASV_IP, LONG, 137),
  CINIT(FTP_FILEMETHOD, LONG, 138),
  CINIT(LOCALPORT, LONG, 139),
  CINIT(LOCALPORTRANGE, LONG, 140),
  CINIT(CONNECT_ONLY, LONG, 141),
  CINIT(CONV_FROM_NETWORK_FUNCTION, FUNCTIONPOINT, 142),
  CINIT(CONV_TO_NETWORK_FUNCTION, FUNCTIONPOINT, 143),
  CINIT(CONV_FROM_UTF8_FUNCTION, FUNCTIONPOINT, 144),
  CINIT(MAX_SEND_SPEED_LARGE, OFF_T, 145),
  CINIT(MAX_RECV_SPEED_LARGE, OFF_T, 146),
  CINIT(FTP_ALTERNATIVE_TO_USER, STRINGPOINT, 147),
  CINIT(SOCKOPTFUNCTION, FUNCTIONPOINT, 148),
  CINIT(SOCKOPTDATA, OBJECTPOINT, 149),
  CINIT(SSL_SESSIONID_CACHE, LONG, 150),
  CINIT(SSH_AUTH_TYPES, LONG, 151),
  CINIT(SSH_PUBLIC_KEYFILE, STRINGPOINT, 152),
  CINIT(SSH_PRIVATE_KEYFILE, STRINGPOINT, 153),
  CINIT(FTP_SSL_CCC, LONG, 154),
  CINIT(TIMEOUT_MS, LONG, 155),
  CINIT(CONNECTTIMEOUT_MS, LONG, 156),
  CINIT(HTTP_TRANSFER_DECODING, LONG, 157),
  CINIT(HTTP_CONTENT_DECODING, LONG, 158),
  CINIT(NEW_FILE_PERMS, LONG, 159),
  CINIT(NEW_DIRECTORY_PERMS, LONG, 160),
  CINIT(POSTREDIR, LONG, 161),
  CINIT(SSH_HOST_PUBLIC_KEY_MD5, STRINGPOINT, 162),
  CINIT(OPENSOCKETFUNCTION, FUNCTIONPOINT, 163),
  CINIT(OPENSOCKETDATA, OBJECTPOINT, 164),
  CINIT(COPYPOSTFIELDS, OBJECTPOINT, 165),
  CINIT(PROXY_TRANSFER_MODE, LONG, 166),
  CINIT(SEEKFUNCTION, FUNCTIONPOINT, 167),
  CINIT(SEEKDATA, OBJECTPOINT, 168),
  CINIT(CRLFILE, STRINGPOINT, 169),
  CINIT(ISSUERCERT, STRINGPOINT, 170),
  CINIT(ADDRESS_SCOPE, LONG, 171),
  CINIT(CERTINFO, LONG, 172),
  CINIT(USERNAME, STRINGPOINT, 173),
  CINIT(PASSWORD, STRINGPOINT, 174),
  CINIT(PROXYUSERNAME, STRINGPOINT, 175),
  CINIT(PROXYPASSWORD, STRINGPOINT, 176),
  CINIT(NOPROXY, STRINGPOINT, 177),
  CINIT(TFTP_BLKSIZE, LONG, 178),
  CINIT(SOCKS5_GSSAPI_SERVICE, STRINGPOINT, 179),
  CINIT(SOCKS5_GSSAPI_NEC, LONG, 180),
  CINIT(PROTOCOLS, LONG, 181),
  CINIT(REDIR_PROTOCOLS, LONG, 182),
  CINIT(SSH_KNOWNHOSTS, STRINGPOINT, 183),
  CINIT(SSH_KEYFUNCTION, FUNCTIONPOINT, 184),
  CINIT(SSH_KEYDATA, OBJECTPOINT, 185),
  CINIT(MAIL_FROM, STRINGPOINT, 186),
  CINIT(MAIL_RCPT, OBJECTPOINT, 187),
  CINIT(FTP_USE_PRET, LONG, 188),
  CINIT(RTSP_REQUEST, LONG, 189),
  CINIT(RTSP_SESSION_ID, STRINGPOINT, 190),
  CINIT(RTSP_STREAM_URI, STRINGPOINT, 191),
  CINIT(RTSP_TRANSPORT, STRINGPOINT, 192),
  CINIT(RTSP_CLIENT_CSEQ, LONG, 193),
  CINIT(RTSP_SERVER_CSEQ, LONG, 194),
  CINIT(INTERLEAVEDATA, OBJECTPOINT, 195),
  CINIT(INTERLEAVEFUNCTION, FUNCTIONPOINT, 196),
  CINIT(WILDCARDMATCH, LONG, 197),
  CINIT(CHUNK_BGN_FUNCTION, FUNCTIONPOINT, 198),
  CINIT(CHUNK_END_FUNCTION, FUNCTIONPOINT, 199),
  CINIT(FNMATCH_FUNCTION, FUNCTIONPOINT, 200),
  CINIT(CHUNK_DATA, OBJECTPOINT, 201),
  CINIT(FNMATCH_DATA, OBJECTPOINT, 202),
  CINIT(RESOLVE, OBJECTPOINT, 203),
  CINIT(TLSAUTH_USERNAME, STRINGPOINT, 204),
  CINIT(TLSAUTH_PASSWORD, STRINGPOINT, 205),
  CINIT(TLSAUTH_TYPE, STRINGPOINT, 206),
  CINIT(TRANSFER_ENCODING, LONG, 207),
  CINIT(CLOSESOCKETFUNCTION, FUNCTIONPOINT, 208),
  CINIT(CLOSESOCKETDATA, OBJECTPOINT, 209),
  CINIT(GSSAPI_DELEGATION, LONG, 210),
  CINIT(DNS_SERVERS, STRINGPOINT, 211),
  CINIT(ACCEPTTIMEOUT_MS, LONG, 212),
  CINIT(TCP_KEEPALIVE, LONG, 213),
  CINIT(TCP_KEEPIDLE, LONG, 214),
  CINIT(TCP_KEEPINTVL, LONG, 215),
  CINIT(SSL_OPTIONS, LONG, 216),
  CINIT(MAIL_AUTH, STRINGPOINT, 217),
  CINIT(SASL_IR, LONG, 218),
  CINIT(XFERINFOFUNCTION, FUNCTIONPOINT, 219),
  CINIT(XOAUTH2_BEARER, STRINGPOINT, 220),
  CINIT(DNS_INTERFACE, STRINGPOINT, 221),
  CINIT(DNS_LOCAL_IP4, STRINGPOINT, 222),
  CINIT(DNS_LOCAL_IP6, STRINGPOINT, 223),
  CINIT(LOGIN_OPTIONS, STRINGPOINT, 224),
  CINIT(SSL_ENABLE_NPN, LONG, 225),
  CINIT(SSL_ENABLE_ALPN, LONG, 226),
  CINIT(EXPECT_100_TIMEOUT_MS, LONG, 227),
  CINIT(PROXYHEADER, OBJECTPOINT, 228),
  CINIT(HEADEROPT, LONG, 229),
  CINIT(PINNEDPUBLICKEY, STRINGPOINT, 230),
  CINIT(UNIX_SOCKET_PATH, STRINGPOINT, 231),
  CINIT(SSL_VERIFYSTATUS, LONG, 232),
  CINIT(SSL_FALSESTART, LONG, 233),
  CINIT(PATH_AS_IS, LONG, 234),
  CINIT(PROXY_SERVICE_NAME, STRINGPOINT, 235),
  CINIT(SERVICE_NAME, STRINGPOINT, 236),
  CINIT(PIPEWAIT, LONG, 237),
  CINIT(DEFAULT_PROTOCOL, STRINGPOINT, 238),
  CINIT(STREAM_WEIGHT, LONG, 239),
  CINIT(STREAM_DEPENDS, OBJECTPOINT, 240),
  CINIT(STREAM_DEPENDS_E, OBJECTPOINT, 241),
  CINIT(TFTP_NO_OPTIONS, LONG, 242),
  CINIT(CONNECT_TO, OBJECTPOINT, 243),
  CINIT(TCP_FASTOPEN, LONG, 244),
  CINIT(KEEP_SENDING_ON_ERROR, LONG, 245),
  CINIT(PROXY_CAINFO, STRINGPOINT, 246),
  CINIT(PROXY_CAPATH, STRINGPOINT, 247),
  CINIT(PROXY_SSL_VERIFYPEER, LONG, 248),
  CINIT(PROXY_SSL_VERIFYHOST, LONG, 249),
  CINIT(PROXY_SSLVERSION, LONG, 250),
  CINIT(PROXY_TLSAUTH_USERNAME, STRINGPOINT, 251),
  CINIT(PROXY_TLSAUTH_PASSWORD, STRINGPOINT, 252),
  CINIT(PROXY_TLSAUTH_TYPE, STRINGPOINT, 253),
  CINIT(PROXY_SSLCERT, STRINGPOINT, 254),
  CINIT(PROXY_SSLCERTTYPE, STRINGPOINT, 255),
  CINIT(PROXY_SSLKEY, STRINGPOINT, 256),
  CINIT(PROXY_SSLKEYTYPE, STRINGPOINT, 257),
  CINIT(PROXY_KEYPASSWD, STRINGPOINT, 258),
  CINIT(PROXY_SSL_CIPHER_LIST, STRINGPOINT, 259),
  CINIT(PROXY_CRLFILE, STRINGPOINT, 260),
  CINIT(PROXY_SSL_OPTIONS, LONG, 261),
  CINIT(PRE_PROXY, STRINGPOINT, 262),
  CINIT(PROXY_PINNEDPUBLICKEY, STRINGPOINT, 263),
  CINIT(ABSTRACT_UNIX_SOCKET, STRINGPOINT, 264),
  CINIT(SUPPRESS_CONNECT_HEADERS, LONG, 265),
  CINIT(REQUEST_TARGET, STRINGPOINT, 266),
  CINIT(SOCKS5_AUTH, LONG, 267),
  CINIT(SSH_COMPRESSION, LONG, 268),
  CINIT(MIMEPOST, OBJECTPOINT, 269),
  CINIT(TIMEVALUE_LARGE, OFF_T, 270),
  CINIT(HAPPY_EYEBALLS_TIMEOUT_MS, LONG, 271),
  CINIT(RESOLVER_START_FUNCTION, FUNCTIONPOINT, 272),
  CINIT(RESOLVER_START_DATA, OBJECTPOINT, 273),
  CINIT(HAPROXYPROTOCOL, LONG, 274),
  CINIT(DNS_SHUFFLE_ADDRESSES, LONG, 275),
  CINIT(TLS13_CIPHERS, STRINGPOINT, 276),
  CINIT(PROXY_TLS13_CIPHERS, STRINGPOINT, 277),
  CINIT(DISALLOW_USERNAME_IN_URL, LONG, 278),
  CINIT(DOH_URL, STRINGPOINT, 279),
  CINIT(UPLOAD_BUFFERSIZE, LONG, 280),
  CINIT(UPKEEP_INTERVAL_MS, LONG, 281),
  CINIT(CURLU, OBJECTPOINT, 282),
  CINIT(TRAILERFUNCTION, FUNCTIONPOINT, 283),
  CINIT(TRAILERDATA, OBJECTPOINT, 284),
  CINIT(HTTP09_ALLOWED, LONG, 285),
  CINIT(ALTSVC_CTRL, LONG, 286),
  CINIT(ALTSVC, STRINGPOINT, 287),
  CINIT(MAXAGE_CONN, LONG, 288),
  CINIT(SASL_AUTHZID, STRINGPOINT, 289),
  CINIT(MAIL_RCPT_ALLOWFAILS, LONG, 290),
  CINIT(SSLCERT_BLOB, BLOB, 291),
  CINIT(SSLKEY_BLOB, BLOB, 292),
  CINIT(PROXY_SSLCERT_BLOB, BLOB, 293),
  CINIT(PROXY_SSLKEY_BLOB, BLOB, 294),
  CINIT(ISSUERCERT_BLOB, BLOB, 295),
  CINIT(PROXY_ISSUERCERT, STRINGPOINT, 296),
  CINIT(PROXY_ISSUERCERT_BLOB, BLOB, 297),
  CINIT(SSL_EC_CURVES, STRINGPOINT, 298),
  CINIT(HSTS_CTRL, LONG, 299),
  CINIT(HSTS, STRINGPOINT, 300),
  CINIT(HSTSREADFUNCTION, FUNCTIONPOINT, 301),
  CINIT(HSTSREADDATA, OBJECTPOINT, 302),
  CINIT(HSTSWRITEFUNCTION, FUNCTIONPOINT, 303),
  CINIT(HSTSWRITEDATA, OBJECTPOINT, 304),
  CINIT(AWS_SIGV4, STRINGPOINT, 305),
  CINIT(DOH_SSL_VERIFYPEER, LONG, 306),
  CINIT(DOH_SSL_VERIFYHOST, LONG, 307),
  CINIT(DOH_SSL_VERIFYSTATUS, LONG, 308),
  CINIT(CAINFO_BLOB, BLOB, 309),
  CINIT(PROXY_CAINFO_BLOB, BLOB, 310),
  CINIT(SSH_HOST_PUBLIC_KEY_SHA256, STRINGPOINT, 311),
  CINIT(PREREQFUNCTION, FUNCTIONPOINT, 312),
  CINIT(PREREQDATA, OBJECTPOINT, 313),
  CINIT(MAXLIFETIME_CONN, LONG, 314),
  CINIT(MIME_OPTIONS, LONG, 315),
  CINIT(SSH_HOSTKEYFUNCTION, FUNCTIONPOINT, 316),
  CINIT(SSH_HOSTKEYDATA, OBJECTPOINT, 317),
  CINIT(PROTOCOLS_STR, STRINGPOINT, 318),
  CINIT(REDIR_PROTOCOLS_STR, STRINGPOINT, 319),
  CINIT(WS_OPTIONS, LONG, 320),
  CINIT(CA_CACHE_TIMEOUT, LONG, 321),
  CINIT(QUICK_EXIT, LONG, 322),
  CINIT(HAPROXY_CLIENT_IP, STRINGPOINT, 323),
  CINIT(SERVER_RESPONSE_TIMEOUT_MS, LONG, 324),
  CURLOPT_LASTENTRY /* the last unused */
} CURLoption;

typedef enum {
  CURLOPT(CURLMOPT_SOCKETFUNCTION, FUNCTIONPOINT, 1),
  CURLOPT(CURLMOPT_SOCKETDATA, OBJECTPOINT, 2),
  CURLOPT(CURLMOPT_PIPELINING, LONG, 3),
  CURLOPT(CURLMOPT_TIMERFUNCTION, FUNCTIONPOINT, 4),
  CURLOPT(CURLMOPT_TIMERDATA, OBJECTPOINT, 5),
  CURLOPT(CURLMOPT_MAXCONNECTS, LONG, 6),
  CURLOPT(CURLMOPT_MAX_HOST_CONNECTIONS, LONG, 7),
  CURLOPT(CURLMOPT_MAX_PIPELINE_LENGTH, LONG, 8),
  CURLOPT(CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE, OFF_T, 9),
  CURLOPT(CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE, OFF_T, 10),
  CURLOPT(CURLMOPT_PIPELINING_SITE_BL, OBJECTPOINT, 11),
  CURLOPT(CURLMOPT_PIPELINING_SERVER_BL, OBJECTPOINT, 12),
  CURLOPT(CURLMOPT_MAX_TOTAL_CONNECTIONS, LONG, 13),
  CURLOPT(CURLMOPT_PUSHFUNCTION, FUNCTIONPOINT, 14),
  CURLOPT(CURLMOPT_PUSHDATA, OBJECTPOINT, 15),
  CURLOPT(CURLMOPT_MAX_CONCURRENT_STREAMS, LONG, 16),
  CURLMOPT_LASTENTRY /* the last unused */
} CURLMoption;

typedef enum {
  CURLSHOPT_NONE,
  CURLSHOPT_SHARE,
  CURLSHOPT_UNSHARE,
  CURLSHOPT_LOCKFUNC,
  CURLSHOPT_UNLOCKFUNC,
  CURLSHOPT_USERDATA,
  CURLSHOPT_LAST  /* never use */
} CURLSHoption;


#undef LONG
#undef OBJECTPOINT
#undef STRINGPOINT
#undef FUNCTIONPOINT
#undef OFF_T
#undef CURLOPT
#undef CINIT

#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

// write
#define GO(A)   \
static uintptr_t my_write_fct_##A = 0;   \
static size_t my_write_##A(char* ptr, size_t size, size_t nmemb, void* userdata)     \
{                                       \
    return (size_t)RunFunctionFmt(my_write_fct_##A, "pLLp", ptr, size, nmemb, userdata);\
}
SUPER()
#undef GO
static void* find_write_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_write_fct_##A == (uintptr_t)fct) return my_write_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_write_fct_##A == 0) {my_write_fct_##A = (uintptr_t)fct; return my_write_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl write callback\n");
    return NULL;
}

// read
#define GO(A)   \
static uintptr_t my_read_fct_##A = 0;   \
static size_t my_read_##A(char* buffer, size_t size, size_t nitems, void* userdata)     \
{                                       \
    return (size_t)RunFunctionFmt(my_read_fct_##A, "pLLp", buffer, size, nitems, userdata);\
}
SUPER()
#undef GO
static void* find_read_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_read_fct_##A == (uintptr_t)fct) return my_read_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_read_fct_##A == 0) {my_read_fct_##A = (uintptr_t)fct; return my_read_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl read callback\n");
    return NULL;
}

// ioctl
#define GO(A)   \
static uintptr_t my_ioctl_fct_##A = 0;   \
static size_t my_ioctl_##A(void* handle, int32_t fnc, void* userdata)     \
{                                       \
    return (size_t)RunFunctionFmt(my_ioctl_fct_##A, "pip", handle, fnc, userdata);\
}
SUPER()
#undef GO
static void* find_ioctl_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ioctl_fct_##A == (uintptr_t)fct) return my_ioctl_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ioctl_fct_##A == 0) {my_ioctl_fct_##A = (uintptr_t)fct; return my_ioctl_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl ioctl callback\n");
    return NULL;
}

// seek
#define GO(A)   \
static uintptr_t my_seek_fct_##A = 0;   \
static int32_t my_seek_##A(void* userdata, int64_t off, int32_t origin)     \
{                                       \
    return (int32_t)RunFunctionFmt(my_seek_fct_##A, "pIi", userdata, off, origin);\
}
SUPER()
#undef GO
static void* find_seek_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_seek_fct_##A == (uintptr_t)fct) return my_seek_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_seek_fct_##A == 0) {my_seek_fct_##A = (uintptr_t)fct; return my_seek_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl seek callback\n");
    return NULL;
}

// header
#define GO(A)   \
static uintptr_t my_header_fct_##A = 0;   \
static size_t my_header_##A(char* buffer, size_t size, size_t nitems, void* userdata)     \
{                                       \
    return (size_t)RunFunctionFmt(my_header_fct_##A, "pLLp", buffer, size, nitems, userdata);\
}
SUPER()
#undef GO
static void* find_header_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_header_fct_##A == (uintptr_t)fct) return my_header_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_header_fct_##A == 0) {my_header_fct_##A = (uintptr_t)fct; return my_header_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl header callback\n");
    return NULL;
}

// progress
#define GO(A)   \
static uintptr_t my_progress_fct_##A = 0;   \
static int my_progress_##A(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)     \
{                                       \
    return (int)RunFunctionFmt(my_progress_fct_##A, "pdddd", clientp, dltotal, dlnow, ultotal, ulnow);\
}
SUPER()
#undef GO
static void* find_progress_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progress_fct_##A == (uintptr_t)fct) return my_progress_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progress_fct_##A == 0) {my_progress_fct_##A = (uintptr_t)fct; return my_progress_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl progress callback\n");
    return NULL;
}

// progress_int
#define GO(A)   \
static uintptr_t my_progress_int_fct_##A = 0;   \
static int my_progress_int_##A(void *clientp, uint64_t dltotal, uint64_t dlnow, uint64_t ultotal, uint64_t ulnow)     \
{                                       \
    return (int)RunFunctionFmt(my_progress_int_fct_##A, "pUUUU", clientp, dltotal, dlnow, ultotal, ulnow);\
}
SUPER()
#undef GO
static void* find_progress_int_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_progress_int_fct_##A == (uintptr_t)fct) return my_progress_int_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_progress_int_fct_##A == 0) {my_progress_int_fct_##A = (uintptr_t)fct; return my_progress_int_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl progress_int callback\n");
    return NULL;
}

// socket
#define GO(A)   \
static uintptr_t my_socket_fct_##A = 0;                                         \
static int my_socket_##A(void *a, int b, int c, void* d, void* e)               \
{                                                                               \
    return (int)RunFunctionFmt(my_socket_fct_##A, "piipp", a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_socket_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_socket_fct_##A == (uintptr_t)fct) return my_socket_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_socket_fct_##A == 0) {my_socket_fct_##A = (uintptr_t)fct; return my_socket_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl socket callback\n");
    return NULL;
}

// timer
#define GO(A)   \
static uintptr_t my_timer_fct_##A = 0;                                  \
static int my_timer_##A(void *a, long b, void* c)                       \
{                                                                       \
    return (int)RunFunctionFmt(my_timer_fct_##A, "plp", a, b, c);  \
}
SUPER()
#undef GO
static void* find_timer_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_timer_fct_##A == (uintptr_t)fct) return my_timer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_timer_fct_##A == 0) {my_timer_fct_##A = (uintptr_t)fct; return my_timer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl timer callback\n");
    return NULL;
}

// push
#define GO(A)   \
static uintptr_t my_push_fct_##A = 0;                                       \
static int my_push_##A(void *a, void* b, size_t c, void* d, void* e)        \
{                                                                           \
    return (int)RunFunctionFmt(my_push_fct_##A, "ppLpp", a, b, c, d, e); \
}
SUPER()
#undef GO
static void* find_push_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_push_fct_##A == (uintptr_t)fct) return my_push_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_push_fct_##A == 0) {my_push_fct_##A = (uintptr_t)fct; return my_push_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl push callback\n");
    return NULL;
}

// debug
#define GO(A)   \
static uintptr_t my_debug_fct_##A = 0;                                          \
static int my_debug_##A(void *a, int b, void* c, size_t d, void* e)             \
{                                                                               \
    return (int)RunFunctionFmt(my_debug_fct_##A, "pipLp", a, b, c, d, e);    \
}
SUPER()
#undef GO
static void* find_debug_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_debug_fct_##A == (uintptr_t)fct) return my_debug_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_debug_fct_##A == 0) {my_debug_fct_##A = (uintptr_t)fct; return my_debug_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl debug callback\n");
    return NULL;
}

// lockcb
#define GO(A)   \
static uintptr_t my_lockcb_fct_##A = 0;                     \
static void my_lockcb_##A(void *a, int b, int c, void* d)   \
{                                                           \
    RunFunctionFmt(my_lockcb_fct_##A, "piip", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_lockcb_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_lockcb_fct_##A == (uintptr_t)fct) return my_lockcb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lockcb_fct_##A == 0) {my_lockcb_fct_##A = (uintptr_t)fct; return my_lockcb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl lockcb callback\n");
    return NULL;
}

// unlockcb
#define GO(A)   \
static uintptr_t my_unlockcb_fct_##A = 0;                   \
static void my_unlockcb_##A(void *a, int b, void* c)        \
{                                                           \
    RunFunctionFmt(my_unlockcb_fct_##A, "pip", a, b, c);    \
}
SUPER()
#undef GO
static void* find_unlockcb_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_unlockcb_fct_##A == (uintptr_t)fct) return my_unlockcb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_unlockcb_fct_##A == 0) {my_unlockcb_fct_##A = (uintptr_t)fct; return my_unlockcb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl unlockcb callback\n");
    return NULL;
}

// ssl_ctx_callback
#define GO(A)   \
static uintptr_t my_ssl_ctx_callback_fct_##A = 0;                               \
static int my_ssl_ctx_callback_##A(void *a, void* b, void* c)                   \
{                                                                               \
    return (int)RunFunctionFmt(my_ssl_ctx_callback_fct_##A, "ppp", a, b, c);    \
}
SUPER()
#undef GO
static void* find_ssl_ctx_callback_Fct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ssl_ctx_callback_fct_##A == (uintptr_t)fct) return my_ssl_ctx_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ssl_ctx_callback_fct_##A == 0) {my_ssl_ctx_callback_fct_##A = (uintptr_t)fct; return my_ssl_ctx_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for curl ssl_ctx_callback callback\n");
    return NULL;
}

#undef SUPER

EXPORT uint32_t my_curl_easy_setopt(x64emu_t* emu, void* handle, uint32_t option, void* param)
{
    (void)emu;

    switch(option) {
        case CURLOPT_WRITEFUNCTION:
            return my->curl_easy_setopt(handle, option, find_write_Fct(param));
        case CURLOPT_READFUNCTION:
            return my->curl_easy_setopt(handle, option, find_read_Fct(param));
        case CURLOPT_IOCTLFUNCTION:
            return my->curl_easy_setopt(handle, option, find_ioctl_Fct(param));
        case CURLOPT_SEEKFUNCTION:
            return my->curl_easy_setopt(handle, option, find_seek_Fct(param));
        case CURLOPT_HEADERFUNCTION:
            return my->curl_easy_setopt(handle, option, find_header_Fct(param));
        case CURLOPT_PROGRESSFUNCTION:
            return my->curl_easy_setopt(handle, option, find_progress_Fct(param));
        case CURLOPT_XFERINFOFUNCTION:
            return my->curl_easy_setopt(handle, option, find_progress_int_Fct(param));
        case CURLOPT_DEBUGFUNCTION:
            return my->curl_easy_setopt(handle, option, find_debug_Fct(param));
        case CURLOPT_SSL_CTX_FUNCTION:
            return my->curl_easy_setopt(handle, option, find_ssl_ctx_callback_Fct(param));
        case CURLOPT_SSL_CTX_DATA:
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_SOCKOPTFUNCTION:
        case CURLOPT_OPENSOCKETFUNCTION:
        case CURLOPT_CLOSESOCKETFUNCTION:
        case CURLOPT_CONV_TO_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_UTF8_FUNCTION:
        case CURLOPT_INTERLEAVEFUNCTION:
        case CURLOPT_CHUNK_BGN_FUNCTION:
        case CURLOPT_CHUNK_END_FUNCTION:
        case CURLOPT_FNMATCH_FUNCTION:
        case CURLOPT_RESOLVER_START_FUNCTION:
        case CURLOPT_TRAILERFUNCTION:
        case CURLOPT_HSTSREADFUNCTION:
        case CURLOPT_HSTSWRITEFUNCTION:
        case CURLOPT_PREREQFUNCTION:
        case CURLOPT_SSH_HOSTKEYFUNCTION:
            printf_log(LOG_NONE, "Error: unimplemented option %u in curl_easy_setopt\n", option);
            return 48; //unknown option...
        default:
            return my->curl_easy_setopt(handle, option, param);
    }
}

EXPORT uint32_t my_curl_multi_setopt(x64emu_t* emu, void* handle, uint32_t option, void* param)
{
    (void)emu;

    switch(option) {
        case CURLMOPT_SOCKETFUNCTION:
            return my->curl_multi_setopt(handle, option, find_socket_Fct(param));
        case CURLMOPT_TIMERFUNCTION:
            return my->curl_multi_setopt(handle, option, find_timer_Fct(param));
        case CURLMOPT_PUSHFUNCTION:
            return my->curl_multi_setopt(handle, option, find_push_Fct(param));
        default:
            return my->curl_multi_setopt(handle, option, param);
    }
}

EXPORT uint32_t my_curl_share_setopt(x64emu_t* emu, void* handle, CURLSHoption option, void* param)
{
    switch(option) {
        case CURLSHOPT_LOCKFUNC:
            return my->curl_share_setopt(handle, option, find_lockcb_Fct(param));
        case CURLSHOPT_UNLOCKFUNC:
            return my->curl_share_setopt(handle, option, find_unlockcb_Fct(param));
        default:
            return my->curl_share_setopt(handle, option, param);
    }
}

#include "wrappedlib_init.h"
