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

    $Id: _msvq.c,v 1.8 2007/10/08 16:49:29 heigazen Exp $

    Multi Stage Vector Quantization

       void msvq(x, cb, l, cbsize, stage, index)

       double *x      : input vector
       double *cb     : codebook vector
       int     l      : order of vector
       int    *cbsize : size of codebook
       int     stage  : number of stage
       int    *index  : index of codebook

*****************************************************************/

#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)
#include "SPTK.h"
#else
#include <SPTK.h>
#endif

void msvq (double *x, double *cb, const int l, int *cbsize, const int stage, int *index)
{
   int i, j;
   double *p;
   static double *xx=NULL;
   static int size;

   if (xx==NULL) {
      xx = dgetmem(l);
      size = l;
   }
   if (size>l) {
      free(xx);
      xx = dgetmem(l);
      size = l;
   }

   movem(x, xx, sizeof(*x), l);

   for (i=0; i<stage; i++) {
      index[i] = vq(xx, cb, l, cbsize[i]);

      p = cb + index[i] * l;
      for (j=0; j<l; j++)
         xx[j] -= p[j];

      cb += cbsize[i] * l;
   }
}

