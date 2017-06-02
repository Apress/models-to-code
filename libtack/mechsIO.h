/*
 * This software is copyrighted 2008 by G. Andrew Mangogna.  The following
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
 *  mechsIO.h -- interface to I/O Bridging functions
 * ABSTRACT:
 *
 * $RCSfile: mechsIO.h,v $
 * $Revision: 1.2 $
 * $Date: 2011/07/16 17:52:20 $
 **--
 */

#ifndef _MECHS_IO_H_
#define _MECHS_IO_H_

#include "mechs.h"
#include <sys/types.h>

typedef void MechsIOConn(int, int) ;
typedef MechsIOConn *MechsIOConnFunc ;

/*
 * Open a socket used for I/O and associate functions to it that will handle
 * the I/O. This function creates a passive TCP socket at the given address and
 * registers with the mechanisms. The "connect" function is called when a new
 * connection is accepted and the argument to the "connect" function is the
 * file descriptor for the newly accepted connection.
 */
extern
int                         /* Returns the socket file descriptor for the
                             * passive socket. Returns -1 on error. */
mechRegisterIOService(
    char const *host,       /* Name of host to bind to. This is usually
                             * "localhost", but if NULL, the the socket
                             * is bound to INADDR_ANY and input may be received
                             * from across the network. */
    int port,               /* port number to bind to */
    MechsIOConnFunc connect,/* Function to call when a new connection is
                             * accepted. */
    int closure             /* User data passed to "connect" function. */
) ;

/*
 * Close the service socket.
 */
extern
void
mechUnregisterIOService(
    int sock                /* service socket number */
) ;

/*
 * Connects to a service.
 */
extern
int
mechConnectIOService(
    char const *host,
    int port
) ;

extern
void
mechDisconnectIOService(
    int sock
) ;

typedef void MechsInput(int, void *, size_t) ;
typedef MechsInput *MechsInputFunc ;

/*
 * Register a function to be called when input is available on a file
 * descriptor.
 */
extern
void
mechRegisterInput(
    int fd,
    MechsInputFunc ifunc,
    int closure
) ;


extern
ssize_t
mechOutput(
    int fd,
    void const *msg,
    size_t len,
    bool final
) ;

#endif /* _MECHS_IO_H_ */
