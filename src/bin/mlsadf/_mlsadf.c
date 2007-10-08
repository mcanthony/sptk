/*
  ---------------------------------------------------------------  
            Speech Signal Processing Toolkit (SPTK)

                      SPTK Working Group                           
                                                                   
                  Department of Computer Science                   
                  Nagoya Institute of Technology                   
                               and                                 
   Interdisciplinary Graduate School of Science and Engineering    
                  Tokyo Institute of Technology                    
                                                                   
                     Copyright (c) 1984-2007                       
                       All Rights Reserved.                        
                                                                   
  Permission is hereby granted, free of charge, to use and         
  distribute this software and its documentation without           
  restriction, including without limitation the rights to use,     
  copy, modify, merge, publish, distribute, sublicense, and/or     
  sell copies of this work, and to permit persons to whom this     
  work is furnished to do so, subject to the following conditions: 
                                                                   
    1. The source code must retain the above copyright notice,     
       this list of conditions and the following disclaimer.       
                                                                   
    2. Any modifications to the source code must be clearly        
       marked as such.                                             
                                                                   
    3. Redistributions in binary form must reproduce the above     
       copyright notice, this list of conditions and the           
       following disclaimer in the documentation and/or other      
       materials provided with the distribution.  Otherwise, one   
       must contact the SPTK working group.                        
                                                                   
  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF TECHNOLOGY,   
  SPTK WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM   
  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   
  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSTITUTE OF         
  TECHNOLOGY, SPTK WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE   
  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        
  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   
  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          
  PERFORMANCE OF THIS SOFTWARE.                                    
                                                                   
  ---------------------------------------------------------------  
*/

/****************************************************************

    $Id: _mlsadf.c,v 1.9 2007/10/08 16:49:34 heigazen Exp $

    MLSA Digital Filter

        double mlsadf(x, b, m, a, pd, d)

        double x     : input
        double *c    : MLSA filter coefficients
        int    m     : order of cepstrum
        double a     : all-pass constant
        int    pd    : order of Pade approximation
        double *d    : delay

        return value : filtered data

*****************************************************************/

#include <stdio.h>

#if defined(WIN32)
#include "SPTK.h"
#else
#include <SPTK.h>
#endif

static double pade[] = {1.0,
                        1.0, 0.0,
                        1.0, 0.0,       0.0,
                        1.0, 0.0,       0.0,       0.0,
                        1.0, 0.4999273, 0.1067005, 0.01170221, 0.0005656279,
                        1.0, 0.4999391, 0.1107098, 0.01369984, 0.0009564853, 0.00003041721
                       };

double *ppade;

static double mlsafir (double x, double *b, const int m, const double a, double *d)
{
   double y = 0.0, aa;
   int i;

   aa = 1 - a*a;

   d[0] = x;
   d[1] = aa*d[0] + a*d[1];

   for (i=2; i<=m; i++) {
      d[i] = d[i] + a*(d[i+1]-d[i-1]);
      y += d[i]*b[i];
   }

   for (i=m+1; i>1; i--) d[i] = d[i-1];

   return (y);
}

static double mlsadf1 (double x, double *b, const int m, const double a, const int pd, double *d)
{
   double v, out = 0.0, *pt, aa;
   int i;

   aa = 1 - a*a;
   pt = &d[pd+1];

   for (i=pd; i>=1; i--) {
      d[i] = aa*pt[i-1] + a*d[i];
      pt[i] = d[i] * b[1];
      v = pt[i] * ppade[i];

      x += (1 & i) ? v : -v;
      out += v;
   }

   pt[0] = x;
   out += x;

   return(out);
}

static double mlsadf2 (double x, double *b, const int m, const double a, const int pd, double *d)
{
   double v, out = 0.0, *pt, aa;
   int i;

   aa = 1 - a*a;
   pt = &d[pd * (m+2)];

   for (i=pd; i>=1; i--) {
      pt[i] = mlsafir(pt[i-1], b, m, a, &d[(i-1)*(m+2)]);
      v = pt[i] * ppade[i];

      x  += (1&i) ? v : -v;
      out += v;
   }

   pt[0] = x;
   out  += x;

   return(out);
}

double mlsadf (double x, double *b, const int m, const double a, const int pd, double *d)
{
   ppade = &pade[pd*(pd+1) / 2];

   x = mlsadf1(x, b, m, a, pd, d);
   x = mlsadf2(x, b, m, a, pd, &d[2*(pd+1)]);

   return(x);
}
