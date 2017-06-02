/*
 * This software is copyrighted 2011 by G. Andrew Mangogna.
 * The following terms apply to all files associated with the software unless
 * explicitly disclaimed in individual files.
 * 
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors and
 * need not follow the licensing terms described here, provided that the
 * new terms are clearly indicated on the first page of each file where
 * they apply.
 * 
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING
 * OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES
 * THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
 * OR MODIFICATIONS.
 * 
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense,
 * the software shall be classified as "Commercial Computer Software"
 * and the Government shall have only "Restricted Rights" as defined in
 * Clause 252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing,
 * the authors grant the U.S. Government and others acting in its behalf
 * permission to use and distribute the software in accordance with the
 * terms specified in this license.
 *
 *++
 * PROJECT:
 *  tack
 *
 * MODULE:
 *  harness.c -- source code to test harness execution library
 *
 * ABSTRACT:
 *
 * $Revision: 1.3 $
 * $Date: 2011/12/24 23:26:58 $
 *--
 */

/*
 * INCLUDE FILES
 */
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include <time.h>
#include <sys/time.h>

#include "harness.h"
#include "pycca_portal.h"
#include "mechs.h"
#include "mechsIO.h"

/*
 * MACRO DEFINITIONS
 */
#define MAX_CMD_ARGS    25
#define MAX_REGISTERED_DOMAINS  10
#define MAX_DOMAIN_NAME_LEN     64

#define ASCII_NUL       '\0' ;
#define BEGIN_QUOTE     '{'
#define END_QUOTE       '}'
#define ESCAPE_CHAR     '\\'

#define OutOfWord   0
#define InWord      1
#define InEscape    2
#define InQuote     3

#ifndef COUNTOF
#   define  COUNTOF(a)  (sizeof(a) / sizeof(a[0]))
#endif /* COUNTOF */

/*
 * TYPE DEFINITIONS
 */
/*
 * Results returned across the "driver" channel are in the form
 * of whitespace separated name/value pairs. The "name" part
 * is a key. This is an enumeration of the keys.
 */
enum drvResultKey {
    drv_None,
    drv_Domain,
    drv_Category,
    drv_Operation,
    drv_Code,
    drv_Result,
    drv_Class,
    drv_Inst,
    drv_Attr,
    drv_Value,
    drv_Event,
    drv_Delay
} ;

enum drvCodeValue {
    code_Success,
    code_Error
} ;

typedef void (*CategoryCmd)(dportal_t const *portal, int argc,
        char const **argv) ;

/*
 * EXTERNAL DATA DEFINITIONS
 */

/*
 * STATIC INLINE FUNCTION DEFINITIONS
 */

/*
 * INLINE FUNCTION DECLARATIONS
 */

/*
 * FORWARD FUNCTION DECLARATIONS
 */
static void drv_connection(int closure, int sock) ;
static void drv_input(int closure, void *buf, size_t len) ;
static void drv_output(enum drvResultKey key, char const *value, ...) ;
static char const *drv_format(char const *fmt, ...) ;

static void stub_connection(int closure, int sock) ;
static void stub_input(int closure, void *buf, size_t len) ;
static void traceCallback(MechTraceInfo traceInfo) ;

static void dopCmd(dportal_t const *portal, int argc, char const **argv) ;
static void dataCmd(dportal_t const *portal, int argc, char const **argv) ;
static void eventCmd(dportal_t const *portal, int argc, char const **argv) ;
static void polyeventCmd(dportal_t const *portal, int argc, char const **argv) ;
static void delayEventCmd(dportal_t const *portal, int argc,
        char const **argv) ;
static void delayPolyEventCmd(dportal_t const *portal, int argc,
        char const **argv) ;

static int category_map_compare(void const *e1, void const *e2) ;

static dportal_t const *find_portal(char const *name) ;
static dop_map_t const *find_dop(dportal_t const *portal, char const *name) ;
static class_map_t const *find_class_map(dportal_t const *portal,
        char const *name) ;
static attr_map_t const *find_attr_map(class_map_t const *class_map,
        char const *name) ;
static bool get_inst_id(class_map_t const *cmap, int argc,
        char const **argv, unsigned *inst_id) ;
static inst_map_t const *find_inst_map(class_map_t const *class_map,
        char const *name) ;
static event_map_t const *find_event_map(class_map_t const *class_map,
        char const *name) ;
static polyevent_map_t const *find_polyevent_map(class_map_t const *class_map,
        char const *name) ;

static int wordparse(char *line, char *const end, char const **argv,
        int *pargc) ;

/*
 * STATIC DATA DEFINITIONS
 */
static int drvDataSock = -1 ;
static int stubDataSock = -1 ;
static bool isTracing = false ;
dportal_t const *mapRegistry[MAX_REGISTERED_DOMAINS] ;
dportal_t const **nextRegistryEntry = mapRegistry ;
dportal_t const **const endRegistry = mapRegistry + COUNTOF(mapRegistry) ;

static char const *const codeStrings[] = {
    "success",      // code_Success,
    "error",        // code_Error
} ;

static struct categoryMap {
    char const *name ;
    CategoryCmd cmd ;
} categories[] = {
    {.name = "data",        .cmd = dataCmd},
    {.name = "delay",       .cmd = delayEventCmd},
    {.name = "delaypoly",   .cmd = delayPolyEventCmd},
    {.name = "dop",         .cmd = dopCmd},
    {.name = "event",       .cmd = eventCmd},
    {.name = "polyevent",   .cmd = polyeventCmd},
} ;

/*
 * EXTERNAL FUNCTION DEFINITIONS
 */
void
harness_init(void)
{
    static char const host[] = "localhost" ;
    /*
     * Set up the server port for the Driver comm channel.
     */
    int status = mechRegisterIOService(host, DRIVER_PORT, drv_connection, 0) ;
    if (status < 0) {
        exit(EXIT_FAILURE) ;
    }
    drvDataSock = -1 ;

    /*
     * Set up the server port for the Stub comm channel.
     */
    status = mechRegisterIOService(host, STUB_PORT, stub_connection, 0) ;
    if (status < 0) {
        exit(EXIT_FAILURE) ;
    }
    stubDataSock = -1 ;
}

int
harness_register(
    dportal_t const *dportal)
{
    if (nextRegistryEntry < endRegistry) {
        *nextRegistryEntry++ = dportal ;
        return 0 ;
    } else {
        fprintf(stderr, "no available registry entries\n") ;
        return -1 ;
    }
}

void
harness_stub_printf(
    char const *type,
    char const *fmt,
    ...)
{
    va_list ap ;
    va_start(ap, fmt) ;
    harness_stub_vprintf(type, fmt, ap) ;
    va_end(ap) ;
}

void
harness_stub_vprintf(
    char const *type,
    char const *fmt,
    va_list ap)
{
    static char buf[BUFSIZ] ;

    /*
     * Put a timestamp on at the beginning.
     */
    struct timeval tv ;
    gettimeofday(&tv, NULL) ;

    char *place = buf ;
    int buflen = sizeof(buf) - 1 ; // allow for closing brace
    int nchars = snprintf(place, buflen,
    #ifdef __APPLE_CC__
            "%s {time %ld.%d ",
    #else
            "%s {time %ld.%ld ",
    #endif /*  __APPLE_CC__ */
            type, tv.tv_sec, tv.tv_usec / 1000) ;
    if (nchars >= 0 && nchars < buflen) {
        place += nchars ;
        buflen -= nchars ;
    } else {
        fprintf(stderr, "%s: buffer overflow: required %d characters\n",
                __func__, nchars) ;
        return ;
    }

    nchars = vsnprintf(place, buflen, fmt, ap) ;
    if (nchars >= 0 && nchars < buflen) {
        place += nchars ;
        buflen -= nchars ;
    } else {
        fprintf(stderr, "%s: buffer overflow: required %d characters\n",
                __func__, nchars) ;
        return ;
    }

    if (stubDataSock >= 0) {
        *place++ = '}' ;
        mechOutput(stubDataSock, buf, place - buf, false) ;
        mechOutput(stubDataSock, "\r\n", 2, true) ;
    } else {
        fputs(buf, stdout) ;
        fputs("}\n", stdout) ;
    }
}

/*
 * STATIC FUNCTION DEFINITIONS
 */
static void
drv_connection(
    int closure,
    int sock)
{
    /*
     * Prevent multiple connections.
     */
    if (drvDataSock == -1) {
        drvDataSock = sock ;
        mechRegisterInput(drvDataSock, drv_input, 0) ;
    } else {
        close(sock) ;
    }
}

/*
 * When commands are received they are of the form:
 *      <category> <domain> ....
 * where <category> is one of:
 *      dop
 *      data
 *      event
 *      polyevent
 *      delayed
 *      delayedpoly
 * and <domain> is the name of a domain.
 *
 * dop <domain> <opname> ?<arg1> <arg2> ...?
 * data <domain> <class> <inst> <attr> ?<value>?
 * event <domain> <class> <inst> <event> ?<param1> <param2> ...?
 * polyevent <domain> <class> <inst> <event> ?<param1> <param2> ...?
 * delay <domain> <class> <inst> <delay> <event> ?<param1> <param2> ...?
 * delaypoly <domain> <class> <inst> <delay> <event> ?<param1> <param2> ...?
 *
 * So here we divide the processing up into functions that are
 * associated with each category of request.
 */

static void
drv_input(
    int closure,
    void *buf,
    size_t len)
{
    static char drvBuffer[BUFSIZ] ;
    static char *drvBufEnd = drvBuffer ;
    static char const *drvCmdArgs[MAX_CMD_ARGS] ;

    if (buf == NULL) {
        exit(EXIT_SUCCESS) ;
    }
    /*
     * Handle the fact that things can arrive in drips and drabs by
     * copying to our own buffer on each chuck that arrives.
     */
    ptrdiff_t bufCnt = drvBufEnd - drvBuffer ;
    /*
     * -1 to allow for a NUL terminator in case a word sits right at the end.
     */
    if (len + bufCnt < sizeof(drvBuffer) - 1) {
        memcpy(drvBufEnd, buf, len) ;
        drvBufEnd += len ;
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, "input buffer overflow",
                drv_None, NULL) ;
        drvBufEnd = drvBuffer ;
        return ;
    }

    char *endline ;
    char *parseend ;
    for (char *line = drvBuffer ; line < drvBufEnd ; line = endline) {
        /*
         * Find the end of the line.
         * Allow either NL terminated or CR/NL terminated lines.
         * Since this comes across sockets, CR/NL (a la HTTP) is most common.
         */
        endline = strchr(line, '\r') ;
        if (endline != NULL) {
            parseend = endline ;
            *endline++ = '\0' ;
            if (*endline == '\n') {
                *endline++ = '\0' ;
            }
        } else {
            endline = strchr(line, '\n') ;
            if (endline != NULL) {
                parseend = endline ;
                *endline++ = '\0' ;
            } else {
                /*
                 * Didn't find record terminator. Shuffle things along
                 * and wait for more input to arrive.
                 */
                ptrdiff_t residue = drvBufEnd - line ;
                memmove(drvBuffer, line, residue) ;
                drvBufEnd = drvBuffer + residue ;
                return ;
            }
        }
        int nargs = COUNTOF(drvCmdArgs) ;
        int result = wordparse(line, parseend, drvCmdArgs, &nargs) ;
        if (result == 0) {
            if (nargs >= 2) {
                struct categoryMap key = {
                    .name = drvCmdArgs[0],
                    .cmd = NULL,
                } ;

                struct categoryMap *ctmap = (struct categoryMap *)bsearch(&key,
                    categories, COUNTOF(categories), sizeof(categories[0]),
                    category_map_compare) ;
                if (ctmap) {
                    dportal_t const *portal = find_portal(drvCmdArgs[1]) ;
                    if (portal) {
                        ctmap->cmd(portal, nargs, drvCmdArgs) ;
                    } else {
                        drv_output(
                                drv_Code, codeStrings[code_Error],
                                drv_Result, "no such domain",
                                drv_Category, drvCmdArgs[0],
                                drv_Domain, drvCmdArgs[1],
                                drv_None, NULL) ;
                    }
                } else {
                    drv_output(
                            drv_Code, codeStrings[code_Error],
                            drv_Result, "no such category",
                            drv_Category, drvCmdArgs[0],
                            drv_None, NULL) ;
                }
            } else {
                drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, drv_format("too few arguments %d", nargs),
                        drv_None, NULL) ;
            }
        } else if (result == -1) {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "exceeded maximum number of arguments",
                    drv_None, NULL) ;
        } else if (result == -2) {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "syntax error",
                    drv_None, NULL) ;
        }
    }
    drvBufEnd = drvBuffer ;
}

static void
drv_output(
    enum drvResultKey key,
    char const *value,
    ...)
{
    static char buf[BUFSIZ] ;
    static char const *const keyStrings[] = {
        NULL,           // drv_None
        "domain",       // drv_Domain
        "category",     // drv_Category
        "operation",    // drv_Operation
        "code",         // drv_Code
        "result",       // drv_Result
        "class",        // drv_Class
        "inst",         // drv_Inst
        "attr",         // drv_Attr
        "value",        // drv_Value
        "event",        // drv_Event
        "delay",        // drv_Delay
    } ;

    static char const *const fmts[] = {
        "%s %s ",
        "%s {%s} ",
    } ;

    char *pbuf = buf ;
    size_t buflen = sizeof(buf) - 2 ; // leave space for \r\n
    va_list ap ;
    va_start(ap, value) ;
    do {
        assert(key < COUNTOF(keyStrings)) ;
        if (key >= COUNTOF(keyStrings) || value == NULL) {
            break ;
        }
        char const *keyValue = keyStrings[key] ;
        if (keyValue == NULL) {
            break ;
        }
        char const *fmt = fmts[
                strchr(value, ' ') == NULL && strlen(value) != 0 ?
                0 : 1] ;
        int bufchars = snprintf(pbuf, buflen, fmt, keyValue, value) ;
        if (bufchars < buflen) {
            pbuf += bufchars ; // point at the terminating NULL
            buflen -= bufchars ;
            key = va_arg(ap, enum drvResultKey) ;
            value = va_arg(ap, char const *) ;
        } else {
            pbuf += buflen ;
            buflen = 0 ;
        }
    } while (buflen != 0) ;

    va_end(ap) ;

    if (pbuf > buf && *(pbuf - 1) == ' ') {
        --pbuf ;    // skip trailing blank if it there
    }
    /*
     * Add record terminator.
     */
    *pbuf++ = '\r' ;
    *pbuf++ = '\n' ;
    mechOutput(drvDataSock, buf, pbuf - buf, true) ;
}

static char const *
drv_format(
    char const *fmt,
    ...)
{
    static char buf[BUFSIZ] ;

    va_list ap ;
    va_start(ap, fmt) ;
    vsnprintf(buf, sizeof(buf), fmt, ap) ;
    va_end(ap) ;

    return buf ;
}

static void
stub_connection(
    int closure,
    int sock)
{
    /*
     * Prevent multiple connections.
     */
    if (stubDataSock == -1) {
        stubDataSock = sock ;
        mechRegisterInput(stubDataSock, stub_input, 0) ;
    } else {
        close(sock) ;
    }
}

static void
stub_input(
    int closure,
    void *buf,
    size_t len)
{
    if (buf == NULL) {
        mechRegisterTrace(NULL) ;
        isTracing = false ;
    } else {
        char const *cmd = buf ;
        if (strncmp(cmd, "trace", strlen("trace")) == 0) {
            mechRegisterTrace(traceCallback) ;
            isTracing = true ;
        } else if (strncmp(cmd, "!trace", strlen("!trace")) == 0) {
            mechRegisterTrace(NULL) ;
            isTracing = false ;
        }
    }
}

/*
 * These mechanism trace facilities provide for transmitting the trace
 * information via a set of key / value pairs separated by spaces, output as a
 * single line.  The encoding depends upon the type of the event as given
 * below:
 *
 * Normal Events:
 *	eventType N	-- Normal event type
 *	eventNumber %u	-- Event number
 *	srcInst %p	-- Source instance
 *	dstInst %p	-- Destination instance
 *	currState %u	-- Current state
 *	newState %u	-- New state
 *	time %s	        -- Timestamp
 *
 * Polymorphic Events:
 *	eventType P	-- Polymorphic event type
 *	eventNumber %u	-- Event number
 *	srcInst %p	-- Source instance
 *	dstInst %p	-- Destination instance
 *	subcode %u	-- Subtype code
 *	hierarch %u	-- Hierarchy number
 *	mappedNumber %u	-- Mapped event number
 *	mappedType %u	-- Mapped event type
 *	time %s	        -- Timestamp
 *
 * Creation Events:
 *	eventType N	-- Normal event type
 *	eventNumber %u	-- Event number
 *	srcInst %p	-- Source instance
 *	dstInst %p	-- Destination instance
 *	dstClass %u	-- Class of the destination instance
 *	time %s	        -- Timestamp
 */
static void
traceCallback(
    MechTraceInfo traceInfo)
{
    static char const traceLabel[] = "trace" ;

    if (!isTracing) {
        return ;
    }

    struct timeval tv ;
    gettimeofday(&tv, NULL) ;

    switch (traceInfo->eventType) {
    case NormalEvent:
        harness_stub_printf(traceLabel, 
            "eventType Normal eventNumber %u srcInst %p dstInst %p "
            "currState %u newState %u",
            traceInfo->eventNumber, traceInfo->srcInst, traceInfo->dstInst,
            traceInfo->info.normalTrace.currState,
            traceInfo->info.normalTrace.newState) ;
        break ;

    case PolymorphicEvent:
        harness_stub_printf(traceLabel, 
	    "eventType Polymorphic eventNumber %u srcInst %p dstInst %p "
            "subcode %u hierarchy %u mappedNumber %u mappedType %u",
            traceInfo->eventNumber, traceInfo->srcInst, traceInfo->dstInst,
            traceInfo->info.polyTrace.subcode,
            traceInfo->info.polyTrace.hierarchy,
            traceInfo->info.polyTrace.mappedNumber,
            traceInfo->info.polyTrace.mappedType) ;
        break ;

    case CreationEvent:
        harness_stub_printf(traceLabel, 
	    "eventType Creation eventNumber %u srcInst %p dstInst %p "
            "dstClass %p",
            traceInfo->eventNumber, traceInfo->srcInst, traceInfo->dstInst,
            traceInfo->info.creationTrace.dstClass) ;
        break ;

    default:
        harness_stub_printf(traceLabel,
	    "eventType %d eventNumber %u srcInst %p dstInst %p",
            traceInfo->eventType, traceInfo->eventNumber,
            traceInfo->srcInst, traceInfo->dstInst) ;
        break ;
    }
}

static dportal_t const *
find_portal(
    char const *name)
{
    for (dportal_t const **iter = mapRegistry ; iter < nextRegistryEntry ;
            ++iter) {
        dportal_t const *dp = *iter ;
        if (strcmp(name, dp->name) == 0) {
            return dp ;
        }
    }
    return NULL ;
}

static int
category_map_compare(
    void const *e1,
    void const *e2)
{
    struct categoryMap const *m1 = e1 ;
    struct categoryMap const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

/*
 * Domain operation driver.
 *
 * dop <domain> <opname> ?<arg1> <arg2> ...?
 *
 * Find the operation and execute it.
 */
static void
dopCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    assert(argc >= 2) ;

    if (argc >= 3) {
        dop_map_t const *dop = find_dop(portal, argv[2]) ;
        if (dop) {
            char const *opresult ;
            bool success = dop->dop_func(argc - 2, argv + 2, &opresult) ;
            drv_output(
                drv_Code, codeStrings[
                    success ? code_Success : code_Error],
                drv_Result, opresult,
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_Operation, argv[2],
                drv_None, NULL) ;
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "domain operation not found",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Operation, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "too few arguments: %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

/*
 * data <domain> <class> <inst> <attr> ?<value>?
 */
static void
dataCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    if (argc >= 5) {
        class_map_t const *cmap = find_class_map(portal, argv[2]) ;
        if (cmap) {
            unsigned inst_id ;
            if (get_inst_id(cmap, argc, argv, &inst_id)) {
                attr_map_t const *amap = find_attr_map(cmap, argv[4]) ;
                if (amap) {
                    char const *dresult ;
                    bool success ;
                    if (argc == 5) {
                        success = amap->attr_read(portal->dportal, cmap->id,
                                inst_id, amap->id, &dresult) ;
                        drv_output(
                                drv_Code, codeStrings[success ?
                                    code_Success : code_Error],
                                drv_Result, dresult,
                                drv_Category, argv[0],
                                drv_Domain, argv[1],
                                drv_Class, argv[2],
                                drv_Inst, argv[3],
                                drv_Attr, argv[4],
                                drv_None, NULL) ;
                    } else if (argc == 6) {
                        success = amap->attr_update(portal->dportal, cmap->id,
                                inst_id, amap->id, argv[5], &dresult) ;
                        drv_output(
                                drv_Code, codeStrings[success ?
                                    code_Success : code_Error],
                                drv_Result, dresult,
                                drv_Category, argv[0],
                                drv_Domain, argv[1],
                                drv_Class, argv[2],
                                drv_Inst, argv[3],
                                drv_Attr, argv[4],
                                drv_Value, argv[5],
                                drv_None, NULL) ;
                    } else {
                        drv_output(
                                drv_Code, codeStrings[code_Error],
                                drv_Result, drv_format("too many arguments %d",
                                    argc),
                                drv_Category, argv[0],
                                drv_Domain, argv[1],
                                drv_None, NULL) ;
                    }
                } else {
                    drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, "unknown attribute",
                        drv_Category, argv[0],
                        drv_Domain, argv[1],
                        drv_Class, argv[2],
                        drv_Inst, argv[3],
                        drv_Attr, argv[4],
                        drv_None, NULL) ;
                }
            }
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "unknown class",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "too few arguments %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

static void
genEvent(
    struct pycca_domain_portal const *dportal,
    unsigned class_id,
    unsigned inst_id,
    name_param_map_t const *emap,
    MechEventType eventType,
    MechDelayTime delay,
    int argc,
    char const **argv,
    char const **cmdv)
{
    EventParamType params ;
    EventParamType *pp ;
    int status ;

    if (argc != emap->pcount) {
        drv_output(
            drv_Code, codeStrings[code_Error],
            drv_Result, drv_format(
                "wrong number of event parameters %d, should have been %d",
                argc, emap->pcount),
            drv_Category, cmdv[0],
            drv_Domain, cmdv[1],
            drv_Class, cmdv[2],
            drv_Inst, cmdv[3],
            drv_Event, cmdv[4],
            drv_None, NULL) ;
        return ;
    }
    if (emap->paramFmt) {
        pp = &params ;
        status = emap->paramFmt(pp, argc, argv) ;
        if (status >= 0) {
            drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "event parameter %d conversion failed: %s",
                    status, argv[status]),
                drv_Category, cmdv[0],
                drv_Domain, cmdv[1],
                drv_Class, cmdv[2],
                drv_Inst, cmdv[3],
                drv_Event, cmdv[4],
                drv_None, NULL) ;
            return ;
        }
    } else {
        pp = NULL ;
    }

    status = delay == 0 ?
        pycca_generate_event(dportal, class_id, inst_id, eventType,
                emap->id, pp) :
        pycca_generate_delayed_event(dportal, class_id, inst_id, eventType,
                emap->id, pp, delay) ;
    if (status == 0) {
        drv_output(
            drv_Code, codeStrings[code_Success],
            drv_Result, "event generation successful",
            drv_Category, cmdv[0],
            drv_Domain, cmdv[1],
            drv_Class, cmdv[2],
            drv_Inst, cmdv[3],
            drv_Event, cmdv[4],
            drv_None, NULL) ;
    } else {
        drv_output(
            drv_Code, codeStrings[code_Error],
            drv_Result, drv_format("event generation failed: %d", status),
            drv_Category, cmdv[0],
            drv_Domain, cmdv[1],
            drv_Class, cmdv[2],
            drv_Inst, cmdv[3],
            drv_Event, cmdv[4],
            drv_None, NULL) ;
    }
}

/*
 * event <domain> <class> <inst> <event> ?<param1> <param2> ...?
 */
static void
eventCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    if (argc >= 5) {
        class_map_t const *cmap = find_class_map(portal, argv[2]) ;
        if (cmap) {
            unsigned inst_id ;
            if (get_inst_id(cmap, argc, argv, &inst_id)) {
                event_map_t const *emap = find_event_map(cmap, argv[4]) ;
                if (emap) {
                    genEvent(portal->dportal, cmap->id, inst_id, emap,
                            NormalEvent, 0, argc - 5, argv + 5, argv) ;
                } else {
                    drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, "unknown event",
                        drv_Category, argv[0],
                        drv_Domain, argv[1],
                        drv_Class, argv[2],
                        drv_Inst, argv[3],
                        drv_Event, argv[4],
                        drv_None, NULL) ;
                }
            }
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "unknown class",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "wrong number of arguments %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

/*
 * polyevent <domain> <class> <inst> <event> ?<param1> <param2> ...?
 */
static void
polyeventCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    if (argc >= 5) {
        class_map_t const *cmap = find_class_map(portal, argv[2]) ;
        if (cmap) {
            unsigned inst_id ;
            if (get_inst_id(cmap, argc, argv, &inst_id)) {
                polyevent_map_t const *emap = find_polyevent_map(cmap,
                        argv[4]) ;
                if (emap) {
                    genEvent(portal->dportal, cmap->id, inst_id, emap,
                            PolymorphicEvent, 0, argc - 5, argv + 5, argv) ;
                } else {
                    drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, "unknown event",
                        drv_Category, argv[0],
                        drv_Domain, argv[1],
                        drv_Class, argv[2],
                        drv_Inst, argv[3],
                        drv_Event, argv[4],
                        drv_None, NULL) ;
                }
            }
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "unknown class",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "wrong number of arguments %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

/*
 * delay <domain> <class> <inst> <delay> <event> ?<param1> <param2> ...?
 */
static void
delayEventCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    if (argc >= 6) {
        class_map_t const *cmap = find_class_map(portal, argv[2]) ;
        if (cmap) {
            unsigned inst_id ;
            if (get_inst_id(cmap, argc, argv, &inst_id)) {
                MechDelayTime delay ;
                if (sscanf(argv[4], "%lu", &delay) == 1) {
                    event_map_t const *emap = find_event_map(cmap, argv[5]) ;
                    if (emap) {
                        genEvent(portal->dportal, cmap->id, inst_id, emap,
                                NormalEvent, delay, argc - 6, argv + 6, argv) ;
                    } else {
                        drv_output(
                            drv_Code, codeStrings[code_Error],
                            drv_Result, "unknown event",
                            drv_Category, argv[0],
                            drv_Domain, argv[1],
                            drv_Class, argv[2],
                            drv_Inst, argv[3],
                            drv_Delay, argv[4],
                            drv_Event, argv[5],
                            drv_None, NULL) ;
                    }
                } else {
                    drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, "bad delay value",
                        drv_Category, argv[0],
                        drv_Domain, argv[1],
                        drv_Class, argv[2],
                        drv_Inst, argv[3],
                        drv_Delay, argv[4],
                        drv_Event, argv[5],
                        drv_None, NULL) ;
                }
            }
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "unknown class",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "wrong number of arguments %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

/*
 * delaypoly <domain> <class> <inst> <delay> <event> ?<param1> <param2> ...?
 */
static void
delayPolyEventCmd(
    dportal_t const *portal,
    int argc,
    char const **argv)
{
    if (argc >= 6) {
        class_map_t const *cmap = find_class_map(portal, argv[2]) ;
        if (cmap) {
            unsigned inst_id ;
            if (get_inst_id(cmap, argc, argv, &inst_id)) {
                MechDelayTime delay ;
                if (sscanf(argv[4], "%lu", &delay) == 1) {
                    event_map_t const *emap = find_event_map(cmap, argv[5]) ;
                    if (emap) {
                        genEvent(portal->dportal, cmap->id, inst_id, emap,
                                PolymorphicEvent, delay, argc - 6, argv + 6,
                                argv) ;
                    } else {
                        drv_output(
                            drv_Code, codeStrings[code_Error],
                            drv_Result, "unknown event",
                            drv_Category, argv[0],
                            drv_Domain, argv[1],
                            drv_Class, argv[2],
                            drv_Inst, argv[3],
                            drv_Delay, argv[4],
                            drv_Event, argv[5],
                            drv_None, NULL) ;
                    }
                } else {
                    drv_output(
                        drv_Code, codeStrings[code_Error],
                        drv_Result, "bad delay value",
                        drv_Category, argv[0],
                        drv_Domain, argv[1],
                        drv_Class, argv[2],
                        drv_Inst, argv[3],
                        drv_Delay, argv[4],
                        drv_Event, argv[5],
                        drv_None, NULL) ;
                }
            }
        } else {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "unknown class",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_None, NULL) ;
        }
    } else {
        drv_output(
                drv_Code, codeStrings[code_Error],
                drv_Result, drv_format(
                    "wrong number of arguments %d", argc),
                drv_Category, argv[0],
                drv_Domain, argv[1],
                drv_None, NULL) ;
    }
}

static int
dop_map_compare(
    void const *e1,
    void const *e2)
{
    dop_map_t const *m1 = e1 ;
    dop_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static dop_map_t const *
find_dop(
    dportal_t const *portal,
    char const *name)
{
    dop_map_t key = {
        .name = name,
        .dop_func = NULL,
    } ;
    return (dop_map_t const *)bsearch(&key, portal->dops, portal->dop_count,
            sizeof(*portal->dops), dop_map_compare) ;
}

static int
class_map_compare(
    void const *e1,
    void const *e2)
{
    class_map_t const *m1 = e1 ;
    class_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static class_map_t const *
find_class_map(
    dportal_t const *portal,
    char const *name)
{
    class_map_t key = {
        .name = name,
        .id = 0,
        .attrs = NULL,
        .attr_count = 0
    } ;
    return (class_map_t const *)bsearch(&key, portal->classes,
            portal->class_count, sizeof(*portal->classes), class_map_compare) ;
}

static int
attr_map_compare(
    void const *e1,
    void const *e2)
{
    attr_map_t const *m1 = e1 ;
    attr_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static attr_map_t const *
find_attr_map(
    class_map_t const *class_map,
    char const *name)
{
    attr_map_t key = {
        .name = name,
        .id = 0,
        .attr_read = NULL,
        .attr_update = NULL
    } ;
    return (attr_map_t const *)bsearch(&key, class_map->attrs,
            class_map->attr_count, sizeof(*class_map->attrs),
            attr_map_compare) ;
}

static bool
get_inst_id(
    class_map_t const *cmap,
    int argc,
    char const **argv,
    unsigned *inst_id)
{
    assert(inst_id != NULL) ;

    if (isdigit(*argv[3])) {
        if (sscanf(argv[3], "%u", inst_id) != 1) {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "bad instance id",
                    drv_Category, argv[0],
                    drv_Domain, argv[1],
                    drv_Class, argv[2],
                    drv_Inst, argv[3],
                    drv_None, NULL) ;
            return false ;
        }
    } else {
        inst_map_t const *imap = find_inst_map(cmap, argv[3]) ;
        if (!imap) {
            drv_output(
                    drv_Code, codeStrings[code_Error],
                    drv_Result, "bad instance name",
                    drv_Domain, argv[0],
                    drv_Category, argv[1],
                    drv_Class, argv[2],
                    drv_Inst, argv[3],
                    drv_None, NULL) ;
            return false ;
        } else {
            *inst_id = imap->id ;
        }
    }

    return true ;
}

static int
inst_map_compare(
    void const *e1,
    void const *e2)
{
    inst_map_t const *m1 = e1 ;
    inst_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static inst_map_t const *
find_inst_map(
    class_map_t const *class_map,
    char const *name)
{
    inst_map_t key = {
        .name = name,
        .id = 0,
    } ;
    return (inst_map_t const *)bsearch(&key, class_map->insts,
            class_map->inst_count, sizeof(*class_map->insts),
            inst_map_compare) ;
}

static int
event_map_compare(
    void const *e1,
    void const *e2)
{
    event_map_t const *m1 = e1 ;
    event_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static event_map_t const *
find_event_map(
    class_map_t const *class_map,
    char const *name)
{
    event_map_t key = {
        .name = name,
        .id = 0,
    } ;
    return (event_map_t const *)bsearch(&key, class_map->events,
            class_map->event_count, sizeof(*class_map->events),
            event_map_compare) ;
}

static int
polyevent_map_compare(
    void const *e1,
    void const *e2)
{
    polyevent_map_t const *m1 = e1 ;
    polyevent_map_t const *m2 = e2 ;

    return strcmp(m1->name, m2->name) ;
}

static polyevent_map_t const *
find_polyevent_map(
    class_map_t const *class_map,
    char const *name)
{
    polyevent_map_t key = {
        .name = name,
        .id = 0,
    } ;
    return (polyevent_map_t const *)bsearch(&key, class_map->polyevents,
            class_map->polyevent_count, sizeof(*class_map->polyevents),
            polyevent_map_compare) ;
}

static int
wordparse(
    char *line,
    char *const end,
    char const **argv,
    int *pargc)
{
    char *dst = line ;
    char const **argvPlace = argv ;
        // -1 to leave space for a NULL terminator of the argv array
    char const ** const argvEnd = argv + *pargc - 1 ;
    int quoteCount = 0 ;
    uint8_t parseState = OutOfWord ;
    uint8_t parseStackStorage[2] ;
    uint8_t *parseStackTop = parseStackStorage ;

    for ( ; line < end ; ++line) {
        char c = *line ;

        switch (parseState) {
        case OutOfWord:
            if (isgraph(c)) {
                if (c == ESCAPE_CHAR) {
                    *parseStackTop++ = InWord ;
                    parseState = InEscape ;
                } else if (c == BEGIN_QUOTE) {
                    *parseStackTop++ = OutOfWord ;
                    parseState = InQuote ;
                    ++quoteCount ;
                    if (argvPlace < argvEnd) {
                        *argvPlace++ = ++dst ;
                    } else {
                        return -1 ;
                    }
                } else {
                    // new word beginning
                    if (argvPlace < argvEnd) {
                        *argvPlace++ = dst ;
                    } else {
                        return -1 ;
                    }
                    *dst++ = c ;
                    parseState = InWord ;
                }
            } // else the character is just space to skip
            break ;

        case InWord:
            if (c == ESCAPE_CHAR) {
                *parseStackTop++ = parseState ;
                parseState = InEscape ;
            } else if (c == BEGIN_QUOTE) {
                *parseStackTop++ = parseState ;
                parseState = InQuote ;
                ++quoteCount ;
            } else if (isspace(c)) {
                // end of word
                parseState = OutOfWord ;
                *dst++ = ASCII_NUL ;
            } else {
                *dst++ = c ;
            }
            break ;

        case InEscape:
            switch (c) {
                case 'a':
                    *dst++ = '\a' ;
                    break ;
                case 'b':
                    *dst++ = '\b' ;
                    break ;
                case 't':
                    *dst++ = '\t' ;
                    break ;
                case 'n':
                    *dst++ = '\n' ;
                    break ;
                case 'v':
                    *dst++ = '\v' ;
                    break ;
                case 'f':
                    *dst++ = '\f' ;
                    break ;
                case 'r':
                    *dst++ = '\r' ;
                    break ;
                default:
                    *dst++ = c ;
                    break ;
            }
            parseState = *--parseStackTop ;
            break ;

        case InQuote:
            if (c == BEGIN_QUOTE) {
                ++quoteCount ;
                *dst++ = c ;
            } else if (c == END_QUOTE) {
                if (--quoteCount == 0) {
                    parseState = *--parseStackTop ;
                    // check if ending the quote also ended the word
                    if (parseState == OutOfWord) {
                        *dst++ = ASCII_NUL ;
                    }
                } else {
                    *dst++ = c ;
                }
            } else if (c == ESCAPE_CHAR) {
                *parseStackTop++ = parseState ;
                parseState = InEscape ;
            } else {
                *dst++ = c ;
            }
            break ;
        }
    }

    *dst = ASCII_NUL ;
    *argvPlace = NULL ;
    *pargc = argvPlace - argv ;
    return quoteCount == 0 && parseState != InEscape ? 0 : -2 ;
}
