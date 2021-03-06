/* s_logbf.c -- float version of s_logb.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include "cdefs-compat.h"
//__FBSDID("$FreeBSD: src/lib/msun/src/s_logbf.c,v 1.9 2008/02/22 02:30:35 das Exp $");

#include "openlibm.h"
#include "math_private.h"


DLLEXPORT float
logbf(float x)
{
	int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;			/* high |x| */
	if(ix==0) return (float)-1.0/fabsf(x);
	if(ix>=0x7f800000) return x*x;
	if(ix<0x00800000) {
#if _TMS320C6X
                float __fmpy_by_0x1p25(float);
                x = __fmpy_by_0x1p25(x);
#else
                static const float
                two25 = 3.355443200e+07;		/* 0x4c000000 */
                
                x *= two25; /* subnormal number, scale up x */
#endif

		GET_FLOAT_WORD(ix,x);
		ix &= 0x7fffffff;
		return (float) ((ix>>23)-127-25);
	} else
		return (float) ((ix>>23)-127);
}
