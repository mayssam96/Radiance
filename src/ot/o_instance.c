/* Copyright (c) 1988 Regents of the University of California */

#ifndef lint
static char SCCSid[] = "$SunId$ LBL";
#endif

/*
 *  o_instance.c - routines for creating octrees for other octrees.
 *
 *     11/11/88
 */

#include  "standard.h"

#include  "object.h"

#include  "instance.h"

#include  "plocate.h"

/*
 *	To determine if two cubes intersect:
 *
 *	1) Check to see if any vertices of first cube are inside the
 *	   second (intersection).
 *
 *	2) Check to see if all vertices of first are to one side of
 *	   second (no intersection).
 *
 *	3) Perform 1 and 2 with roles reversed.
 *
 *	4) Check to see if any portion of any edge of second is inside
 *	   first (intersection).
 *
 *	5) If test 4 fails, we have no intersection.
 *
 *	Note that if we were testing two boxes, we would need
 *  to check that neither had any edges inside the other to be sure.
 *	Since an octree is a volume rather than a surface, we will
 *  return a value of 2 if the cube is entirely within the octree.
 */


o_instance(o, cu)			/* determine if cubes intersect */
OBJREC  *o;
CUBE  *cu;
{
	static char  vstart[4] = {0, 3, 5, 6};
	FVECT  cumin, cumax;
	FVECT  vert[8];
	FVECT  v1, v2;
	register INSTANCE  *in;
	int  vloc, vout;
	register int  i, j;
					/* get octree arguments */
	in = getinstance(o, GET_BOUNDS);
					/* check if cube vertex in octree */
	for (j = 0; j < 3; j++)
		cumax[j] = (cumin[j] = in->obj->scube.cuorg[j]) +
				in->obj->scube.cusize;
	vloc = ABOVE | BELOW;
	vout = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 3; j++) {
			v1[j] = cu->cuorg[j];
			if (i & 1<<j)
				v1[j] += cu->cusize;
		}
		multp3(v2, v1, in->b.xfm);
		if (j = plocate(v2, cumin, cumax))
			vout++;
		vloc &= j;
	}
	if (vout == 0)			/* all inside */
		return(2);
	if (vout < 8)			/* some inside */
		return(1);
	if (vloc)			/* all to one side */
		return(0);
					/* octree vertices in cube? */
	for (j = 0; j < 3; j++)
		cumax[j] = (cumin[j] = cu->cuorg[j]) + cu->cusize;
	vloc = ABOVE | BELOW;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 3; j++) {
			v1[j] = in->obj->scube.cuorg[j];
			if (i & 1<<j)
				v1[j] += in->obj->scube.cusize;
		}
		multp3(vert[i], v1, in->f.xfm);
		if (j = plocate(vert[i], cumin, cumax))
			vloc &= j;
		else
			return(1);	/* vertex inside */
	}
	if (vloc)			/* all to one side */
		return(0);
					/* check edges */
	for (i = 0; i < 4; i++)
		for (j = 0; j < 3; j++) {
						/* clip modifies vertices! */
			VCOPY(v1, vert[vstart[i]]);
			VCOPY(v2, vert[vstart[i] ^ 1<<j]);
			if (clip(v1, v2, cumin, cumax))
				return(1);		/* edge inside */
		}

	return(0);			/* no intersection */
}
