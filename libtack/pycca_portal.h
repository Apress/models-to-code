/*
 * This software is copyrighted 2011 - 2013 by G. Andrew Mangogna.
 * The following terms apply to all files associated with the software unless
 * explicitly disclaimed in individual files.
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
 *  pycca_portal.h -- Interface to domain data internals.
 *
 * ABSTRACT:
 *  This file contains a set of declarations for data structures and functions
 *  that allow access to the internal data of a pycca domain.  Domains built
 *  using the "-dataaccess" option of pycca will include this file and provide
 *  external definitions for data items that can be to access the class
 *  attributes of the domain. This facility is provided for bridging and
 *  testing purposes.
 **--
 */

#ifndef PYCCA_PORTAL_H_
#define PYCCA_PORTAL_H_

/*
 * INCLUDE FILES
 */
#include <stddef.h>
#include <stdbool.h>

#include "mechs.h"

/*
 * MACRO DEFINITIONS
 */
/*
 * Error return values.
 * These values are returned by the functions below upon an error.
 */
    /*
     * No such class.
     */
#define PYCCA_PORTAL_NO_CLASS       (-1)
    /*
     * No such instance.
     */
#define PYCCA_PORTAL_NO_INST        (-2)
    /*
     * No such attribute.
     */
#define PYCCA_PORTAL_NO_ATTR        (-3)
    /*
     * Instance slot is not in use.
     */
#define PYCCA_PORTAL_UNALLOC        (-4)
    /*
     * Attribute is not allowed to be written.
     */
#define PYCCA_PORTAL_NO_UPDATE      (-5)
    /*
     * Class does not have a state model.
     */
#define PYCCA_PORTAL_NO_STATES      (-6)
    /*
     * No such event for the class.
     */
#define PYCCA_PORTAL_NO_SUCH_EVENT  (-7)
    /*
     * Bad event type parameter
     */
#define PYCCA_PORTAL_BAD_EVENT_TYPE (-8)
    /*
     * Class does not support dynamic instances.
     */
#define PYCCA_PORTAL_NO_DYNAMIC     (-9)

/*
 * TYPE DECLARATIONS
 */
typedef unsigned short ClassId_t ;
typedef unsigned short InstId_t ;
typedef unsigned short AttrId_t ;
typedef unsigned short AttrOffset_t ;
typedef unsigned short AttrSize_t ;

struct pycca_attr_portal {
    AttrOffset_t offset ;
    AttrSize_t size ;
} ;

struct pycca_class_portal {
    void *storage ;
    struct pycca_attr_portal const *attrs ;
    struct mechclass const *mechClass ;
    AttrId_t numAttrs ;
    InstId_t numInsts ;
    size_t instSize ;
    size_t instOffset ;
    bool isConst ;
    bool hasCommon ;
    StateCode initialState ;
} ;

struct pycca_domain_portal {
    struct pycca_class_portal const *classes ;
    ClassId_t numClasses ;
} ;

/*
 * EXTERNAL DATA DECLARATIONS
 */

/*
 * INLINE FUNCTION DECLARATIONS
 */

/*
 * EXTERNAL FUNCTION DECLARATIONS
 */

/*
 * Read an attribute value from a domain. If the return value is non-negative,
 * then it represents the actual number of bytes read.  Negative numbers are
 * encoded as listed above.
 */
extern int
pycca_read_attr(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    AttrId_t attr,      /* The number of the attribute to be read. This number
                         * is generated by pycca and placed in the domain header
                         * file. */
    void *dst,          /* A pointer to memory where the attribute value is
                         * placed. */
    AttrSize_t size     /* The number of bytes pointed to by "dst". */
) ;

/*
 * Obtain a reference to an attribute for reading. This is used when we don't
 * want to copy an entire value around when reading.  Returns 0 upon success.
 * Negative numbers are encoded as listed above.
 */
extern int
pycca_get_attr_ref(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    AttrId_t attr,      /* The number of the attribute to be read. This number
                         * is generated by pycca and placed in the domain header
                         * file. */
    void const **ref,   /* A pointer to a pointer to memory where the address
                         * of the attribute is placed. */
    AttrSize_t *size    /* A pointer to a location that is filled on output
                         * to be the number of bytes of data that the attribute
                         * occupies. */
) ;

/*
 * Update an attribute value in a domain.  If the return value is non-negative,
 * then it represents the actual number of bytes copied into the attribute
 * storage location.  Negative numbers are encoded as listed above.
 */
extern int
pycca_update_attr(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    AttrId_t attr,      /* The number of the attribute to be read. This number
                         * is generated by pycca and placed in the domain header
                         * file. */
    void const *src,    /* A pointer to memory from where the attibute value
                         * is taken. */
    AttrSize_t size     /* The number of bytes pointed to by "src". */
) ;

/*
 * Generate an ordinary or polymorphic event to the given instance.  The return
 * value is 0 upon success. Negative return values are encoded as listed above.
 */
extern int
pycca_generate_event(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    MechEventType
        eventType,      /* The type of the event to generate. Only values of
                         * NormalEvent and PolymorphicEvent are allowed
                         * (creation events are generated another way). */
    EventCode event,    /* The event code for the event to generate. */
    EventParamType
        *params         /* A pointer to the supplemental event parameters.
                         * This argument may be NULL for events that pass
                         * no parameters. */
) ;

/*
 * Generate an ordinary or polymorphice event to the given instance at some
 * time in the future. The return value is 0 upon success. Negative return
 * values are encoded as listed above.
 */
extern int
pycca_generate_delayed_event(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    MechEventType
        eventType,      /* The type of the event to generate. Only values of
                         * NormalEvent and PolymorphicEvent are allowed
                         * (creation events are generated another way). */
    EventCode event,    /* The event code for the event to generate. */
    EventParamType
        *params,        /* A pointer to the supplemental event parameters.
                         * This argument may be NULL for events that pass
                         * no parameters. */
    MechDelayTime time  /* The number of milliseconds to delay posting the
                         * event. This number may be 0, in which case the
                         * event will be posted immediately and this
                         * degenerates to a call to "pycca_generate_event()".*/
) ;

/*
 * Generate a creation event to the given class.  The return value is 0 upon
 * success. Negative return values are encoded as listed above.
 */
extern int
pycca_generate_creation(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    EventCode event,    /* The event code for the event to generate. */
    EventParamType
        *params         /* A pointer to the supplemental event parameters.
                         * This argument may be NULL for events that pass
                         * no parameters. */
) ;

/*
 * Cancel a previously posted delayed event.  The return value is 0 upon
 * success. Negative return values are encoded as listed above.
 */
int
pycca_cancel_delayed_event(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst,      /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
    EventCode event     /* The event code for the event to cancel. */
) ;

/*
 * Reset an instance's state machine to its default initial state. Returns
 * 0 upon success and negative return values are encoded as listed above.
 *
 * N.B. resetting a state machine to its default initial state is, in general,
 * not a well behaved operation. Knowledge of the detailed internals of the
 * state model are required to know if the reset machine can continue to
 * function properly.
 */
extern int
pycca_reset_machine(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst       /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
) ;

/*
 * Create an instance of a class. The return value is the instance id that can
 * later be used in other portal functions.  Negative return values indicate
 * failure and are encoded as listed above.
 */
extern int
pycca_create_instance(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class     /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
) ;

/*
 * Destroy an instance of a class. The return value is 0 upon success.
 * Negative return values indicate failure and are encoded as listed above.
 */
extern int
pycca_destroy_instance(
    struct pycca_domain_portal
        const *portal,  /* A pointer to the portal structure for the domain.
                         * This structure is generated by when the -dataportal
                         * option is given */
    ClassId_t class,    /* The number of the class. This number is generated
                         * by pycca and placed in the domain header file. */
    InstId_t inst       /* The number of the instance. Instance numbers are
                         * consecutive non-negative integers up to the maximum
                         * number of instances defined for the class. */
) ;

#endif /* PYCCA_PORTAL_H_ */

/* vim: set sw=4 ts=4 sts=4 expandtab */
