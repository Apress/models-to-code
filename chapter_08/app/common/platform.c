/*
 *++
 * Copyright 2017 by Leon Starr, Andrew Mangogna and Stephen Mellor
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * PROJECT:
 *      Models to Code
 *
 * MODULE:
 *      Platform code for lube / sio integration application.
 *
 *--
 */

/*
 * INCLUDE FILES
 */
#include "harness.h"
#include "mechs.h"
#include "alsharness.h"
#include "lube.h"
#include "sio.h"

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
    lube_harness_init() ;
    sio_harness_init() ;

    lube_init() ;
    sio_init() ;
}


/*
 * STATIC FUNCTION DEFINITIONS
 */
