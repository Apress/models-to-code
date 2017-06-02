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
 *  pycca_portal.c -- functions to access pycca domain internals
 *
 * ABSTRACT:
 *  This file contains a set of function definitions used to access the
 *  internal data structures of pycca generated domains.
 **--
 */

/*
 * INCLUDE FILES
 */
#include <string.h>
#include <stdbool.h>

#include "mechs.h"
#include "pycca_portal.h"

/*
 * MACRO DEFINITIONS
 */

/*
 * TYPE DEFINITIONS
 */

/*
 * EXTERNAL FUNCTION REFERENCES
 */

/*
 * EXTERNAL INLINE FUNCTION REFERENCES
 */

/*
 * STATIC FUNCTION DECLARATIONS
 */

static int pycca_construct_event(struct pycca_domain_portal const *portal,
    ClassId_t class, InstId_t inst, MechEventType eventType, EventCode event,
    EventParamType *params, MechEcb *ecbRef) ;

/*
 * EXTERNAL DATA REFERENCES
 */

/*
 * EXTERNAL DATA DEFINITIONS
 */

/*
 * STATIC DATA DEFINITIONS
 */

/*
 * EXTERNAL FUNCTION DEFINITIONS
 */

int
pycca_read_attr(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    AttrId_t attr,
    void *dst,
    AttrSize_t size)
{
    int copied ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            if (attr < classes->numAttrs) {
                struct pycca_attr_portal const *attrs = classes->attrs + attr ;
                if (classes->hasCommon == false || instRef->alloc != 0) {
                    void *src = (void *)((char *)instRef + attrs->offset) ;
                    copied = size < attrs->size ? size : attrs->size ;
                    memcpy(dst, src, copied) ;
                } else {
                    copied = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                copied = PYCCA_PORTAL_NO_ATTR ;
            }
        } else {
            copied = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        copied = PYCCA_PORTAL_NO_CLASS ;
    }

    return copied ;
}

int
pycca_get_attr_ref(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    AttrId_t attr,
    void const **ref,
    AttrSize_t *size)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            if (attr < classes->numAttrs) {
                struct pycca_attr_portal const *attrs = classes->attrs + attr ;
                if (classes->hasCommon == false || instRef->alloc != 0) {
                    if (ref) {
                        *ref = (void const *)((char *)instRef + attrs->offset) ;
                    }
                    if (size) {
                        *size = attrs->size ;
                    }
                    result = 0 ;
                } else {
                    result = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                result = PYCCA_PORTAL_NO_ATTR ;
            }
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_update_attr(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    AttrId_t attr,
    void const *src,
    AttrSize_t size)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            if (attr < classes->numAttrs) {
                struct pycca_attr_portal const *attrs = classes->attrs + attr ;
                if (classes->hasCommon == false || instRef->alloc != 0) {
                    if (!classes->isConst) {
                        void *dst = (void *)((char *)instRef + attrs->offset) ;
                        result = size < attrs->size ? size : attrs->size ;
                        memcpy(dst, src, result) ;
                    } else {
                        result = PYCCA_PORTAL_NO_UPDATE ;
                    }
                } else {
                    result = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                result = PYCCA_PORTAL_NO_ATTR ;
            }
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_generate_event(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    MechEventType eventType,
    EventCode event,
    EventParamType *params)
{
    int result ;
    MechEcb ecb ;

    result = pycca_construct_event(portal, class, inst, eventType, event,
            params, &ecb) ;
    if (result >= 0) {
        mechEventPost(ecb) ;
    }

    return result ;
}

int
pycca_generate_delayed_event(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    MechEventType eventType,
    EventCode event,
    EventParamType *params,
    MechDelayTime time)
{
    int result ;
    MechEcb ecb ;

    result = pycca_construct_event(portal, class, inst, eventType, event,
            params, &ecb) ;
    if (result >= 0) {
        mechEventPostDelay(ecb, time) ;
    }

    return result ;
}

int
pycca_generate_creation(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    EventCode event,
    EventParamType *params)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        MechClass classData = classes->mechClass ;
        if (classes->hasCommon == true && classData != NULL &&
                classData->odb != NULL) {
            if (event < classData->odb->eventCount) {
                MechEcb ecb = mechCreationEventNew(event, classData, NULL) ;
                if (params) {
                    ecb->eventParameters = *params ;
                }
                mechEventPost(ecb) ;
                result = 0 ;
            } else {
                result = PYCCA_PORTAL_NO_SUCH_EVENT ;
            }
        } else {
            result = PYCCA_PORTAL_NO_STATES ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_cancel_delayed_event(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    EventCode event)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            mechEventDelayCancel(event, instRef, NULL) ;
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_reset_machine(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            MechClass classData = classes->mechClass ;
            if (classes->hasCommon == true && classData != NULL) {
                if (instRef->alloc != 0) {
                    instRef->currentState =  classes->initialState ;
                    result = 0 ;
                } else {
                    result = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                result = PYCCA_PORTAL_NO_STATES ;
            }
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_create_instance(
    struct pycca_domain_portal const *portal,
    ClassId_t class)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        MechClass classData = classes->mechClass ;
        if (classes->hasCommon == true && classData != NULL &&
                classData->iab != NULL) {
            MechInstance instRef = mechInstCreate(classData,
                    classes->initialState) ;
            /*
             * Convert the pointer value back to an instance id.
             */
            result = ((char *)instRef - (char *)classes->storage -
                    classes->instOffset) / classes->instSize ;
        } else {
            result = PYCCA_PORTAL_NO_DYNAMIC ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

int
pycca_destroy_instance(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            MechClass classData = classes->mechClass ;
            if (classes->hasCommon == true && classData != NULL &&
                    classData->iab != NULL) {
                if (instRef->alloc != 0) {
                    mechInstDestroy(instRef) ;
                    result = 0 ;
                } else {
                    result = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                result = PYCCA_PORTAL_NO_STATES ;
            }
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}

/*
 * STATIC FUNCTION DEFINITIONS
 */

static int
pycca_construct_event(
    struct pycca_domain_portal const *portal,
    ClassId_t class,
    InstId_t inst,
    MechEventType eventType,
    EventCode event,
    EventParamType *params,
    MechEcb *ecbRef)
{
    int result ;

    if (class < portal->numClasses) {
        struct pycca_class_portal const *classes = portal->classes + class ;
        if (inst < classes->numInsts) {
            MechInstance instRef = (MechInstance)((char *)classes->storage +
                    classes->instSize * inst + classes->instOffset) ;
            MechClass classData = classes->mechClass ;
            if (classes->hasCommon == true && classData != NULL) {
                if (instRef->alloc != 0) {
                    MechEcb ecb = NULL ;
                    switch (eventType) {
                    case NormalEvent:
                        if (classData->odb != NULL) {
                            if (event < classData->odb->eventCount) {
                                ecb = mechEventNew(event, instRef, NULL) ;
                            } else {
                                result = PYCCA_PORTAL_NO_SUCH_EVENT ;
                            }
                        } else {
                            result = PYCCA_PORTAL_NO_STATES ;
                        }
                        break ;

                    case PolymorphicEvent:
                        if (classData->pdb != NULL) {
                            if (event < classData->pdb->eventCount) {
                                ecb = mechPolyEventNew(event, instRef, NULL) ;
                            } else {
                                result = PYCCA_PORTAL_NO_SUCH_EVENT ;
                            }
                        } else {
                            result = PYCCA_PORTAL_NO_STATES ;
                        }
                        break ;

                    default:
                        result = PYCCA_PORTAL_BAD_EVENT_TYPE ;
                        break ;
                    }
                    if (ecb) {
                        if (params) {
                            ecb->eventParameters = *params ;
                        }
                        if (ecbRef) {
                            *ecbRef = ecb ;
                        }
                        result = 0 ;
                    }
                } else {
                    result = PYCCA_PORTAL_UNALLOC ;
                }
            } else {
                result = PYCCA_PORTAL_NO_STATES ;
            }
        } else {
            result = PYCCA_PORTAL_NO_INST ;
        }
    } else {
        result = PYCCA_PORTAL_NO_CLASS ;
    }

    return result ;
}


/* vim: set sw=4 ts=4 sts=4 autoindent expandtab */
