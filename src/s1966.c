/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/* (c) Copyright 1989,1990,1991,1992 by                                      */
/*     Senter for Industriforskning, Oslo, Norway                            */
/*     All rights reserved. See the copyright.h for more details.            */
/*                                                                           */
/*****************************************************************************/

#include "copyright.h"


#define S1966

#include "sislP.h"

#if defined(SISLNEEDPROTOTYPES)
void s1966(double ep[],int im1,int im2,int idim,int ipar,double epar1[],
	   double epar2[],double eeps[],int nend[],int iopen1,int iopen2,
	   double edgeps[],double afctol,int iopt,int itmax,
	   int ik1,int ik2,SISLSurf **rs,double emxerr[],int *jstat)
#else
void s1966(ep,im1,im2,idim,ipar,epar1,epar2,eeps,nend,iopen1,iopen2,
           edgeps,afctol,iopt,itmax,ik1,ik2,
           rs,emxerr,jstat)
     double ep[];
     int    im1;
     int    im2;
     int    idim;
     int    ipar;
     double epar1[];
     double epar2[];
     double eeps[];
     int    nend[];
     int    iopen1;
     int    iopen2;
     double edgeps[];
     double afctol;
     int    iopt;
     int    itmax;
     int    ik1;
     int    ik2;
     SISLSurf   **rs;
     double emxerr[];
     int    *jstat;
#endif
/*
********************************************************************
*
* Purpose: To compute a tensor-product spline-approximation of order
*          (ik1,ik2) to the rectangular array of idim-dimensional
*          points given by ep.
*
* Input : Ep     - Array (length idim*im1*im2) containing the points
*                  to be approximated.
*         Im1    - The no. of points in first parameter-direction.
*         Im2    - The no. of points in second parameter-direction.
*         Idim   - The no. of components of each input-point.
*                  The approximation will be a parametric surface
*                  situated in the idim-dimensional euclidean space
*                  (usually 3).
*         Ipar   - Flag determining the parametrization of the data points:
*                   = 1: Mean accumulated cord-length parameterization.
*                   = 2: Uniform parametrization.
*                   = 3: Parametrization given by epar1 and epar2.
*         Epar1  - Array (length im1) containing a parametrization
*                  in the first parameter-direction. (Will only
*                  be used if ipar=3).
*         Epar2  - Array (length im2) containing a parametrization
*                  in the second parameter-direction. (Will only
*                  be used if ipar=3).
*         Eeps   - Array (length idim) containing the max. permissible
*                  deviation of the approximation from the given data
*                  points, in each of the components. More specifically,
*                  the approximation will not deviate more than eeps(kdim)
*                  in component no. kdim, from the bilinear approximation
*                  to the data.
*         Nend   - Array (length 4) giving the no. of derivatives to be
*                  kept fixed along each edge of the bilinear interpolant.
*                  The numbering of the edges is the same as for edgeps below.
*                  All the derivatives of order < (nend(i)-1) will be kept
*                  fixed along the edge i. Hence nend(i)=0 indicates that
*                  nothing is to be kpet fixed along edge i. (Used by the
*                  data reduction routine.)
*                  To be kept fixed here means to have error less than edgeps.
*                  In general, it is impossible to remove any knots and keep
*                  an edge completely fixed.
*         iopen1 - Open/closed parameter in first parameter direction.
*                      =  1 : Produce open surface.
*                      =  0 : Produce closed, non-periodic surface if possible.
*                      = -1 : Produce closed, periodic surface if possible.
*                  NB! The surface will be closed/periodic only if the first 
*                      and last column of data points are (approximately) equal.
*         iopen2 - Open/closed parameter in second parameter direction.
*                      =  1 : Produce open surface.
*                      =  0 : Produce closed, non-periodic surface if possible.
*                      = -1 : Produce closed, periodic surface if possible.
*                  NB! The surface will be closed/periodic only if the first 
*                      and last row of data points are (approximately) equal.
*         Edgeps - Array (length idim*4) containing the max. deviation from
*                  the bilinear interpolant which is acceptable along the
*                  edges of the surface.
*                  Edgeps(1,i):edgeps(idim,i) gives the tolerance along
*                  the edge corresponding to the i-th parameter having
*                  one of it`s extremal-values.
*                   i=1: min value of first parameter.
*                   i=2: max value of first parameter.
*                   i=3: min value of second parameter.
*                   i=4: max value of second parameter.
*                  (Used by the data-reduction routine.)
*                  Edgeps(kp,i) will only have significance if nend(i)>0.
*         Afctol - 0.0 >= afctol <= 1.0.
*                  Afctol indicates how the tolerance is to be shared
*                  between the two data-reduction stages. For the linear
*                  reduction, a tolerance of afctol*eeps will be used,
*                  while a tolerance of (1.0-afctol)*eeps will be used
*                  during the final data reduction (similarly for edgeps.)
*                  Default is 0.
*          Iopt  - Flag indicating the order in which the data-reduction
*                  is to be performed:
*                   = 1: Remove knots inparameter-direction 1 only.
*                   = 2: Remove knots inparameter-direction 2 only.
*                   = 3: Remove knots first in parameter-direction 1 and
*                        then in parameter-direction 2.
*                   = 4: Remove knots first in parameter-direction 2 and
*                        then in parameter-direction 1.
*         Itmax  - Max. no. of iterations in the data-reduction.
*         Ik1    - The order of the approximation in first
*                  parameter-directon.
*         Ik2    - The order of the approximation in second
*                  parameter-directon.
*
* Output:
*         Jstat  - Output status:
*                   < 0 : Error.
*                   = 0 : Ok.
*                   > 0 : Warning:
*         Rs     - Pointer to surface.
*         Emxerr - Array (length idim) (allocated outside this routine.)
*                  containing the error in the approximation to the data.
*                  This is guaranteed upper bound on the max. deviation
*                  in each component, between the final approximation
*                  and the bilinear spline-approximation to the original data.
*
* Method:
*        First the bilinear interpolant to the data is computed, using the
*        parameterization given by ipar, and knots are removed from this
*        initial approximation by a call to the data-reduction routine for
*        surfaces. Then the order is raised to (ik1,ik2) and the final data
*        reduction is performed.
*-
* Calls: s1965, s1350, s6chpar, s6err.
*
* Written by: C.R.Birkeland, Si, April 1993.
* Changed by: Per OEyvind, SINTEF, 1994-11.
*             Removed following memory leaks:
*              1) Improper use of copy flag to newSurf()
*              2) Forgetting to free temp array after using icopy == 1
* Changed and renamed by : Vibeke Skytt, SINTEF Oslo, 02.95. Introduced
*                                                            periodicity.
**********************************************************************
*/
{
  int in1,in2;                /* Number of vertices                   */
  int newin1, newin2;
  int fouridim=4*idim;
  int i;                      /* Loop control parameters              */
  int stat=0, kpos=0;         /* Error message parameters             */
  double *par1 = NULL;
  double *par2 = NULL;
  double *knot1 = NULL;       /* Knot vectors in 1 and 2. par.dir.    */
  double *knot2 = NULL;
  double *error1 = NULL;      /* Arrays for error storage             */
  double *error2 = NULL;
  double *maxerr = NULL;
  double *newcoeff = NULL;    /* Coefficients array                   */
  SISLCurve *ocurve1 = NULL;  /* Used to store local curves           */
  SISLCurve *ocurve2 = NULL;
  SISLSurf *osurf1 = NULL;    /* Used to store local surfaces         */
  SISLSurf *osurf2 = NULL;

  /* Check Input */

  if (im1 < 2 || im2 < 2 || ik1 < 1 || ik2 < 1 || idim < 1)
    goto err103;
  if (ipar < 1 || ipar > 3) ipar = 1;

  if (ipar != 3)
    {
      /* Generate parametrization */

      s1528(idim, im1, im2, ep, ipar, SISL_CRV_OPEN, SISL_CRV_OPEN,
	    &par1, &par2, &stat);
      if (stat<0) goto error;
    }
  else
    {
      /* Parametrization is passed as parameter */

      par1 = epar1;
      par2 = epar2;
    }

  /* Represent input (points) as a surface of
   * order 2 (linear) in both directions.
   * First, generate knot vectors */

  knot1 = newarray(im1+2, DOUBLE);
  knot2 = newarray(im2+2, DOUBLE);
  if(knot1 == NULL || knot2 == NULL) goto err101;
  memcopy(&knot1[1],par1,im1,DOUBLE);
  memcopy(&knot2[1],par2,im2,DOUBLE);
  knot1[0] = knot1[1];
  knot2[0] = knot2[1];
  knot1[im1+1] = knot1[im1];
  knot2[im2+1] = knot2[im2];
  osurf1 = newSurf(im1, im2, 2, 2, knot1, knot2, ep,
		   1,idim, 1);
  if (osurf1 == NULL) goto err101;
  if (knot1 != NULL) freearray(knot1); knot1 = NULL;
  if (knot2 != NULL) freearray(knot2); knot2 = NULL;

  /* Compute tolerance vectors for linear reduction
   * Both max deviation of surface and max dev. of edges */

  maxerr = newarray(idim, DOUBLE);
  error1 = newarray(idim, DOUBLE);
  error2 = newarray(fouridim, DOUBLE);
  if (error1 == NULL || error2 == NULL || maxerr == NULL)
    goto err101;
  for (i=0; i<fouridim; i++)
    {
      edgeps[i] = MIN(edgeps[i], eeps[(i+idim)%idim]);
      error2[i] = afctol * edgeps[i];
    }
  for (i=0; i<idim; i++)
    error1[i] = afctol*eeps[i];

  /* Perform datareduction on the bilinear interpolant */

  s1965(osurf1, error1, nend, SISL_CRV_OPEN, SISL_CRV_OPEN, error2,
	iopt, itmax, &osurf2, maxerr, &stat);
  if (stat<0) goto error;

  in1 = osurf2->in1;
  in2 = osurf2->in2;

  /* Free surface osurf1 */

  if(osurf1 != NULL)
    {
      freeSurf(osurf1);
      osurf1 = NULL;
    }

  /* Piecewise linear interpolant to the reduced
   * bilinear interpolant expressed as a surface
   * of orders ik1 and ik2 */

  /* Second parameter direction */

  s1350(osurf2->ecoef,&(osurf2->et2)[1], in2,
	in1 * idim, ik2, &ocurve1, &stat);
  if (stat<0) goto error;

  newin2 = ocurve1->in;

  /* Transpose result, store new coefficients in
   * array newcoeff */

  if( (newcoeff = newarray(idim * in1 * newin2, DOUBLE)) == NULL )
    goto err101;
  s6chpar(ocurve1->ecoef, in1, newin2, idim, newcoeff);

  /* First parameter direction */

  s1350(newcoeff, &(osurf2->et1)[1], in1,
	idim*newin2, ik1, &ocurve2, &stat);
  if (stat<0) goto error;
  newin1 = ocurve2->in;

  /* Free surface osurf2 */

  if(osurf2 != NULL)
    {
      freeSurf(osurf2);
      osurf2 = NULL;
    }

  /* Transpose back and get coefficients of bilinear
   * approximatoin surface of orders ik1 and ik2     */

  newcoeff = increasearray(newcoeff,
			   idim * newin1 * newin2, DOUBLE);
  if (newcoeff == NULL) goto err101;
  s6chpar(ocurve2->ecoef, newin2, newin1, idim, newcoeff);

  /* Store results as a surface */

  osurf1 = newSurf(newin1, newin2, ik1, ik2, ocurve2->et,
		   ocurve1->et, newcoeff, 1, idim, 1);

  if (newcoeff != NULL) freearray(newcoeff); newcoeff = NULL;

  if (osurf1 == NULL) goto err101;

  /* Set periodicity flag. */

  osurf1->cuopen_1 = ocurve2->cuopen;
  osurf1->cuopen_2 = ocurve1->cuopen;

  /* Compute tolerance for final datareduction */

  for (i=0; i<fouridim; i++)
    error2[i] = edgeps[i]-error2[i];
  for (i=0; i<idim; i++)
    error1[i] = eeps[i]-maxerr[i];

  /* Perform final datareduction step */

  s1965(osurf1, error1, nend, iopen1, iopen2, error2, iopt, itmax,
	rs, emxerr, &stat);
  if (stat<0) goto error;

  /* Compute total (and final) error */

  for(i=0; i<idim; i++)
    emxerr[i] += maxerr[i];

  /* Success */

  *jstat = 0;
  goto out;

  /* Empty array. */

 err101:
  *jstat = -101;
  s6err("s1966",*jstat,kpos);
  goto out;

  /* Error in input */

 err103:
  *jstat = -103;
  s6err("s1966",*jstat,kpos);
  goto out;

  /* Error in lower level routine. */

 error:
  *jstat = stat;
  s6err("s1966",*jstat,kpos);
  goto out;

  /* Exit */

 out:
  /* Free SISL-curves allocated in this routine */

  if(ocurve1 != NULL) freeCurve(ocurve1);
  if(ocurve2 != NULL) freeCurve(ocurve2);

  /* Free SISL-surfaces allocated in this routine */

  if(osurf1 != NULL) freeSurf(osurf1);

  /* Free arrays */

  if(error1 != NULL) freearray(error1);
  if(error2 != NULL) freearray(error2);
  if(maxerr != NULL) freearray(maxerr);

  if (ipar != 3)
    {
      freearray(par1);
      freearray(par2);
    }

  return;
}
