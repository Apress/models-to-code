/*
 * This software is copyrighted 2008 -2013 by G. Andrew Mangogna.  The following
 * terms apply to all files associated with the software unless explicitly
 * disclaimed in individual files.
 * 
 * The author hereby grants permission to use, copy, modify, distribute,
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
 * *++
 * MODULE:
 *  mechsIO.c -- implementation of I/O bridging operations
 * ABSTRACT:
 *  Linking POSIX I/O via the mechanisms.
 *
 **--
 */

#include "mechsIO.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

#define COUNTOF(a)  (sizeof(a) / sizeof(a[0]))

static void readInput(int) ;
static void acceptConnection(int) ;

/*
 * This is a map of file descriptors that are associated with a particular
 * passive socket where connection requests are accepted.  The index into
 * "mechsService" is the file descriptor of the passive socket. The value of
 * each "mechsService" element is a structure which gives a function to
 * call and data pointer to round trip along.
 */
typedef struct serviceMap {
    MechsIOConnFunc connect ;
    int closure ;
} *ServiceMap ;

static struct serviceMap mechIOServices[FD_SETSIZE] ;

int
mechRegisterIOService(
    char const *host,
    int port,
    MechsIOConnFunc connect,
    int closure)
{
    int sock = socket(PF_INET, SOCK_STREAM, 0) ;
    if (sock == -1) {
        perror("unable to open PF_INET stream socket") ;
        return sock ;
    }

    struct sockaddr_in bindAddr ;
    memset(&bindAddr, 0, sizeof(bindAddr)) ;
#   ifdef __APPLE_CC__
    bindAddr.sin_len = sizeof(bindAddr) ;
#   endif /* __APPLE_CC__ */
    bindAddr.sin_family = AF_INET ;
    bindAddr.sin_port = htons(port) ;

    if (host) {
        struct hostent *hostAddr = gethostbyname(host) ;

        if (hostAddr == NULL) {
            perror(host) ;
            return -1 ;
        }

        assert(hostAddr->h_addrtype == AF_INET) ;
        memcpy(&bindAddr.sin_addr, hostAddr->h_addr_list[0],
                hostAddr->h_length) ;
    } else {
        bindAddr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    }

    int err = bind(sock, (struct sockaddr const *)&bindAddr,
            sizeof(bindAddr)) ;
    if (err == -1) {
        perror ("bind()") ;
        return err ;
    }

    err = listen(sock, SOMAXCONN) ;
    if (err == -1) {
        perror ("listen()") ;
        return err ;
    }

    assert(sock < COUNTOF(mechIOServices)) ;
    ServiceMap m = mechIOServices + sock ;
    assert(m->connect == NULL) ;
    m->connect = connect ;
    m->closure = closure ;

    mechRegisterFDService(sock, acceptConnection, NULL, NULL) ;

    return sock ;
}

void
mechUnregisterIOService(
    int sock)
{
    assert(sock >= 0 && sock < COUNTOF(mechIOServices)) ;
    ServiceMap m = mechIOServices + sock ;
    if (close(sock) == -1) {
        perror("close()") ;
    }
    m->connect = NULL ;
    mechRemoveFDService(sock, true, false, false) ;
}

int
mechConnectIOService(
    char const *host,
    int port)
{
    struct hostent *hostAddr = gethostbyname(host) ;

    if (hostAddr == NULL) {
        perror(host) ;
        return -1 ;
    }

    assert(hostAddr->h_addrtype == AF_INET) ;

    int sock = socket(PF_INET, SOCK_STREAM, 0) ;
    if (sock == -1) {
        perror("socket()") ;
        return sock ;
    }

    struct sockaddr_in connectAddr ;
    connectAddr.sin_family = AF_INET ;
    connectAddr.sin_port = htons(port) ;
    memcpy(&connectAddr.sin_addr, hostAddr->h_addr_list[0],
            sizeof(connectAddr.sin_addr)) ;

    if (connect(sock, (struct sockaddr *)&connectAddr,
            sizeof(connectAddr)) == -1) {
        perror("connect()") ;
        close(sock) ;
        return -1 ;
    }
    return sock ;
}

void
mechDisconnectIOService(
    int sock)
{
    if (close(sock) == -1) {
        perror("close()") ;
    }
}

/*=========================================================================*/

typedef struct inputMap {
    MechsInputFunc input ;
    int closure ;
} *InputMap ;
static struct inputMap inputServices[FD_SETSIZE] ;

void
mechRegisterInput(
    int fd,
    MechsInputFunc ifunc,
    int closure)
{
    assert(fd >= 0 && fd < COUNTOF(inputServices)) ;
    InputMap inf = inputServices + fd ;
    inf->input = ifunc ;
    inf->closure = closure ;
    if (ifunc) {
        /*
         * "readInput" assumes non blocking I/O.
         */
        int stat = fcntl(fd, F_SETFL, O_NONBLOCK) ;
        if (stat == -1) {
            perror("fcntl()") ;
            return ;
        }
        mechRegisterFDService(fd, readInput, NULL, NULL) ;
    } else {
        mechRemoveFDService(fd, true, false, false) ;
    }
}

ssize_t
mechOutput(
    int fd,
    void const *msg,
    size_t len,
    bool final)
{
#       ifdef __linux
    size_t flags = final ? (MSG_NOSIGNAL | MSG_MORE) : MSG_NOSIGNAL ;
#       else
    size_t flags = 0 ;
#       endif  /* __linux */
    ssize_t n = send(fd, msg, len, flags) ;
    if (n == -1) {
        perror("send()") ;
    }

    return n ;
}

/*======================================================================*/

static void
readInput(
    int sock)
{
    char buf[BUFSIZ] ;

    assert(sock >= 0 && sock < COUNTOF(inputServices)) ;
    InputMap iof = inputServices + sock ;
    assert(iof->input != NULL) ;

    for (;;) {
        ssize_t n = recv(sock, buf, sizeof(buf), 0) ;
        if (n == -1) {
            if (errno != EAGAIN) {
                perror("recv()") ;
                if (close(sock) == -1) {
                    perror("close()") ;
                }
                mechRegisterInput(sock, NULL, -1) ;
            }
            break ;
        } else if (n == 0) {
            /*
             * EOF
             */
            if (close(sock) == -1) {
                perror("close()") ;
            }
            iof->input(iof->closure, NULL, 0) ;
            mechRegisterInput(sock, NULL, -1) ;
            break ;
        } else {
            iof->input(iof->closure, buf, n) ;
        }
    }
}

static void
acceptConnection(
    int sock)
{
    int conn = accept(sock, NULL, 0) ;

    if (conn == -1) {
        perror("accept()") ;
        return ;
    }

    ServiceMap m = mechIOServices + sock ;
    assert(m->connect != NULL) ;
    m->connect(m->closure, conn) ;
}
