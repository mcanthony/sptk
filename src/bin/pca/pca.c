/* ----------------------------------------------------------------- */
/*             The Speech Signal Processing Toolkit (SPTK)           */
/*             developed by SPTK Working Group                       */
/*             http://sp-tk.sourceforge.net/                         */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 1984-2007  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                1996-2008  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the SPTK working group nor the names of its */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/********************************************************************************
*                                                                               *
*    PCA : Principal Component Analysis                                         *
*                                                                               *
*                                           2009.8 A.Tamamori                   *
*                                                                               *
*       usage:                                                                  *
*                pca [ options ] [ infile ] > stdout                            *
*                                                                               *
*       options:                                                                *
*                -l L  : length of vector                           [3]         *
*                -t t  : number of input vectors                    [EOF]       *
*                -n N  : output number of pricipal component        [2]         *
*                -i I  : iteration of jacobi method                 [10000]     *
*                -e e  : threshold of convergence of jacobi method  [0.000001]  *
*                -v    : output eigenvectors and mean vector        [FALSE]     *
*                -V    : output eigenvalues                         [FALSE]     *
*                -p    : output principal component score           [FALSE]     *
*                -c    : output cumulative contribution rate        [FALSE]     *
*                                                                               *
*       infile:                                                                 *
*                training vectors                                               *
*                    X(0), X(1), X(2), ...                                      *
*                    where                                                      *  
*                        X(0)=[x(0), x(1), ..., x(L-1)],                        *
*                        X(1)=[x(L), x(L+1), ..., x(2*L-1)],                    *
*                        X(2)=[x(2*L), x(L+1), ..., x(3*L-1)],                  *
*                          .                                                    *
*                          .                                                    *
*                          .                                                    *
*                                                                               *
*       stdout:                                                                 *
*                eigenvectors and mean vector (if -v option is specified)       *
*                   evec(1), evec(2), ..., evec(N), mean                        *
*                                                                               *
*                eigenvalues  (if -V option is specified)                       *
*                   eval(1), eval(2), ..., eval(N)                              *
*                                                                               *
*                principal component scores (if -p option is specified)         *
*                   z_1(1), ..., z_1(N), z_2(1), ..., z_2(N), ...,              *
*                                                                               *
*                contribution rate (output if -c option is specified)           *
*                   c(1), c(2), ..., c(N)                                       *
*                                                                               *
*       notice:                                                                 *
*                Calculation of PCA is based on jacobi method.                  *
*                Output Mean vector is placed at tail of all other output.      *
*                                                                               *
*********************************************************************************/

static char *rcs_id = "$Id: pca.c,v 1.3 2009/09/30 06:57:18 mataki Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#  ifndef HAVE_STRRCHR
#     define strrchr rindex
#  endif
#endif

#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

/* Defalut Values */
#define LENG           24
#define PRICOMP_ORDER  2
#define EPS            0.000001
#define ITEMAX         10000

typedef enum {FALSE = -1, TRUE = 1} BOOL;

/* Command Name */
char *cmnd;

void usage (int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - Pricipal Component Analysis\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -l L  : length of vector(for input and output)                   [%d]\n", LENG);
   fprintf(stderr, "       -t t  : number of input vectors                                  [EOF]\n");
   fprintf(stderr, "       -n N  : output order of principal component                      [%d]\n", PRICOMP_ORDER);
   fprintf(stderr, "       -i I  : iteration of jacobi method                               [%d]\n", ITEMAX);
   fprintf(stderr, "       -e e  : threshold of convergence of jacobi method                [%f]\n", EPS);
   fprintf(stderr, "       -v    : output eigen vectors and mean vector                     [FALSE]\n");
   fprintf(stderr, "       -V    : output eigen values                                      [FALSE]\n");
   fprintf(stderr, "       -c    : output contribution rates                                [FALSE]\n");
   fprintf(stderr, "       -p    : output principal component scores of each training data  [FALSE]\n");
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       training data set       [stdin]\n");
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       eigen vectors, eigen values, principal component scores, contribution rates\n");
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n", PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}

double ** malloc_matrix(int n)
{
  double **m;
  double *mtmp;
  int i, j;

  if ((mtmp = (double *)malloc(sizeof(double) * n * n)) == NULL) {
    fprintf(stderr, "Can't malloc in %s\n", cmnd);
    exit(EXIT_FAILURE);
  }
  if ((m = (double **)malloc(sizeof(double *) * n)) == NULL) {
    fprintf(stderr, "Can't malloc in %s\n", cmnd);
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < n; i++) {
    m[i] = &(mtmp[i * n]);
  }
  return m;
}
  
int jacobi(double **m, int n, double eps, double *e_val, double **e_vec, int itemax)
{
  int i, j, k;
  int count;
  int ret;
  double **a;
  double max_e;
  int r, c;
  double a1, a2, a3;
  double co, si;
  double w1, w2;
  double t1, ta;
  double tmp;
  
  for(i=0;i<n;i++) {
    for(j=i+1;j<n;j++) {
      if (m[i][j] != m[j][i]) {
	return -1;
      }
    }
  }
  
  if((a = malloc_matrix(n)) == NULL){
    fprintf(stderr, "Error : Can't malloc at jacobi in %s\n", cmnd);
    exit(EXIT_FAILURE);
  }
  for(i=0;i<n;i++) {
    for(j=0;j<n;j++) {
      a[i][j] = m[i][j];
    }
  }

  for(i=0;i<n;i++) {
    for(j=0;j<n;j++) {
      e_vec[i][j] = (i == j) ? 1.0 : 0.0;
    }
  }

  count = 0;
  
  while(1) {

    max_e = 0.0;
    for(i=0;i<n;i++) {
      for(j=i+1;j<n;j++) {
	if (max_e < fabs(a[i][j])) {
	  max_e = fabs(a[i][j]);
	  r = i;
	  c = j;
	}
      }
    }
    if (max_e <= eps) {
      ret = count;
      break;
    }
    if (count >= itemax) {
      ret = -2;
      break;
    }

    a1 = a[r][r];
    a2 = a[c][c];
    a3 = a[r][c];
    
    t1 = fabs(a1 - a2);
    ta = 2.0 * a3 / (t1 + sqrt(t1 * t1 + 4.0 * a3 * a3));
    co = sqrt(1.0 / (ta * ta + 1.0));
    si = ta * co;
    if (a1 < a2) si = -si;
    
    for(i=0;i<n;i++) {
      w1 = e_vec[i][r];
      w2 = e_vec[i][c];
      e_vec[i][r] = w1 * co + w2 * si;
      e_vec[i][c] = -w1 * si + w2 * co;
      if (i == r || i == c) continue;
      w1 = a[i][r];
      w2 = a[i][c];
      a[i][r] = w1 * co + w2 * si;
      a[i][c] = -w1 * si + w2 * co;
      a[r][i] = a[i][r];
      a[c][i] = a[i][c];
    }
    a[r][r] = a1 * co * co + a2 * si * si + 2.0 * a3 * co * si;
    a[c][c] = a1 + a2 - a[r][r];
    a[r][c] = 0.0;
    a[c][r] = 0.0;
    
    count++;
  }
  
  for(i=0;i<n;i++) {
    e_val[i] = a[i][i];
  }
  
  for(i=0;i<n;i++) {
    for(j=i+1;j<n;j++) {
      tmp = e_vec[i][j];
      e_vec[i][j] = e_vec[j][i];
      e_vec[j][i] = tmp;
    }
  }
  
  for(i=0;i<n;i++) {
    for(j=n-2;j>=i;j--) {
      if (e_val[j] < e_val[j+1]) {
	tmp = e_val[j];
	e_val[j] = e_val[j+1];
	e_val[j+1] = tmp;
	for(k=0;k<n;k++) {
	  tmp = e_vec[j][k];
	  e_vec[j][k] = e_vec[j+1][k];
	  e_vec[j+1][k] = tmp;
	}
      }
    }
  }

  free(a[0]); free(a);

  return (ret);
}

int main (int argc,char *argv[])
{
   FILE *fp = stdin;
   int i, j, k, n = PRICOMP_ORDER, leng = LENG, total = -1, ispipe;
   BOOL out_evecFlg = FALSE;
   BOOL out_pri_compFlg = FALSE, out_evalFlg = FALSE, out_cont_rateFlg = FALSE;
   double sum;
   double *buf = NULL;
   double *mean = NULL, **var = NULL;
   double eps = EPS;
   int itemax = ITEMAX;
   double **e_vec = NULL, *e_val = NULL; /* eigenvector and eigenvalue */
   double *cont_rate = NULL; /* contribution rate */
   double jacobi_conv;
   double *z = NULL; /* output principal component score */
   double *y = NULL;
   
   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   
   while (--argc) 
      if ((**++argv) == '-'){
         switch (*(*argv + 1)) {
         case 'l':
            leng = atoi(*++argv);
	    --argc;
            break;
	 case 't':
	   total = atoi(*++argv);
	   --argc;
	   break;
	 case 'n':
	   n = atoi(*++argv);
	   --argc;
	   break;
	 case 'e':
	   eps = atof(*++argv);
	   --argc;
	   break;
	 case 'i':
	   itemax = atoi(*++argv);
	   --argc;
	   break;
	 case 'v':
	   out_evecFlg = TRUE;
	   break;	   
	 case 'V':
	   out_evalFlg = TRUE;
	   break;
	 case 'c':
	   out_cont_rateFlg = TRUE;
	   break;
	 case 'p':
	   out_pri_compFlg = TRUE;
	   break;	   
	 case 'h':
            usage(EXIT_SUCCESS);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(argv + 1));
            usage(EXIT_FAILURE);
         }
      }
      else
	fp = getfp(*argv, "rb");

   /* -- Count number of input vectors -- */   
   if (total == -1) {
     ispipe = fseek(fp, 0L, SEEK_END); /* set current position to EOF */
     total = (int)(ftell(fp) / (double)leng / (double)sizeof(float));
     rewind(fp); /* return current position to the front */

     if (ispipe != 0) {   /* input vectors is from standard input via pipe */
       fprintf(stderr, "\n %s (Error) -t option must be specified for the standard input via pipe.\n", cmnd);
       usage(EXIT_FAILURE);
     }
   }

   if(n > leng){
     fprintf(stderr, "\n %s (Error) output number of pricipal component"
	     " must be less than length of vector.\n", cmnd);
     usage(EXIT_FAILURE);
   }

/* PCA */
   /* allocate memory for input vectors */ 
   buf = dgetmem(leng * total);
   
   /* read input vectors */
   freadf(buf, sizeof(*buf), total * leng, fp);

   /* allocate memory for mean vectors and covariance matrix */
   mean = dgetmem(leng);
   var = malloc_matrix(leng);
   
   /* calculate mean vector */
   for(i = 0; i < leng; i++){
     for(j = 0, sum = 0.0; j < total; j++)
       sum += buf[i + j * leng];
     mean[i] = sum / total;
   }
   /* calculate cov. mat. */
   for(i = 0; i < leng; i++){
     for(j = 0; j < leng; j++){
       sum = 0.0;
       for(k = 0; k < total; k++)
	 sum += (buf[i + k * leng] - mean[i]) * (buf[j + k * leng] - mean[j]);
       var[i][j] = sum / total;
     }
   }
   
   /* allocate memory for eigenvector and eigenvalue */
   e_vec = malloc_matrix(leng);
   e_val = dgetmem(leng);
   
   /* calculate eig.vec. and eig.val. with jacobi method */
   if((jacobi_conv = jacobi(var, leng, eps, e_val, e_vec, itemax)) == -1){
     fprintf(stderr, "Error : matrix is not symmetric.\n");
     exit(EXIT_FAILURE);
   }
   else if(jacobi_conv == -2){
     fprintf(stderr, "Error : loop in jacobi method reached %d times.\n", itemax);
     exit(EXIT_FAILURE);
   }
   
   /* allocate memory for contribution rate of each eigenvalue */
   cont_rate = dgetmem(leng);

   /* calculate contribution rate of each eigenvalue */
   for(j = 0; j < leng; j++){
     sum = 0.0;
     for(i = 0; i < leng; i++)
       sum += e_val[i];
     cont_rate[j] = e_val[j] / sum;
   }

   /* allocate memory for pricipal component score */
   z = dgetmem(n * total);
   y = dgetmem(total);
   fillz(z, n * total, sizeof(double));
   fillz(y, total, sizeof(double));

   /* calculate pricipal component score */
   for(k = 0; k < total; k++)
     for(i = 0; i < n; i++)
       for(j = 0; j < leng; j++)
	 z[k * leng + i] += e_vec[i][j] * (buf[k * leng + j] - mean[j]);

/* end of PCA */
   
   for(i = 0; i < n; i++){
     if(out_evecFlg == TRUE){ /* -v option */
       fwritef(e_vec[i], sizeof(double), leng, stdout);
     }
     if(out_evalFlg == TRUE){ /* -V option */
       fwritef(e_val + i, sizeof(double), 1, stdout);
     }
     else if(out_cont_rateFlg == TRUE){ /* -c option */
       fwritef(cont_rate + i, sizeof(double), 1, stdout);
     }
     else if(out_pri_compFlg == TRUE){ /* -p option */
       for(j = 0; j < total; j++)
	 y[j] = z[j * leng + i];
       fwritef(y, sizeof(*y), total, stdout);       
     }
   }
   if(out_evecFlg == TRUE) /* mean vector */
     fwritef(mean, sizeof(double), leng, stdout);
   
   return 0;
}