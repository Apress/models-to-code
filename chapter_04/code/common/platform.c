/*
 *++
 * PROJECT:
 *
 * MODULE:
 *
 * ABSTRACT:
 *
 *--
 */

/*
 * INCLUDE FILES
 */
#include "harness.h"
#include "mechs.h"
#include "atcharness.h"

/*
 * MACRO DEFINITIONS
 */

/*
 * TYPE DEFINITIONS
 */

/*
 * EXTERNAL DATA DEFINITIONS
 */

/*
 * STATIC DATA DEFINITIONS
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

/*
 * EXTERNAL FUNCTION DEFINITIONS
 */
void
sysDeviceInit(void)
{
}

void
sysDomainInit(void)
{
    harness_init() ;
    atctrl_harness_init() ;
}


/*
 * STATIC FUNCTION DEFINITIONS
 */
