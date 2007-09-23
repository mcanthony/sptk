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

/************************************************************************
*                                                                       *
*    Calculation of Root Mean Squared Error                             *
*                                                                       *
*                                     1996.3  K.Koishida                *
*                                                                       *
*       usage:                                                          *
*               rmse [ options ] file1 [ infile ] > stdout              *
*       options:                                                        *
*               -l l     :  frame length        [0]                     *
*       infile:                                                         *
*               data sequence                                           *
*                       , x(0), x(1), ..., x(l-1),                      *
*       stdout:                                                         *
*               rmse                                                    *
*                       , x(0)+x(1)...+x(l-1)/l                         *
*       notice:                                                         *
*               if l>0, calculate rmse frame by frame                   *
*       require:                                                        *
*               rmse()                                                  *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: rmse.c,v 1.14 2007/09/23 15:08:35 heigazen Exp $";


/*  Standard C Libraries  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SPTK.h>


/*  Default Values  */
#define LENG 0

/*  Command Name  */
char *cmnd;


void usage (int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - calculation of root mean squared error\n",cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] file1 [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "      -l l  : frame length       [%d]\n", LENG);
   fprintf(stderr, "      -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "      data sequence (%s)      [stdin]\n", FORMAT);
   fprintf(stderr, "  file1:\n");
   fprintf(stderr, "      data sequence (%s)\n", FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "      root mean squared error (%s)\n", FORMAT);
   fprintf(stderr, "  notice:\n");
   fprintf(stderr, "      if l>0, calculate rmse frame by frame\n");
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n",PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}

int main (int argc, char **argv)
{
   int l=LENG, num=0;
   FILE *fp=stdin, *fp1=NULL;
   double *x, *y, x1, y1, sub, z=0.0;

   if ((cmnd=strrchr(argv[0], '/'))==NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc)
      if (**++argv=='-') {
         switch (*(*argv+1)) {
         case 'l':
            l = atoi(*++argv);
            --argc;
            break;
         case 'h':
            usage (0);
         default:
            fprintf(stderr, "%s : Invalid option '%c' !\n", cmnd, *(*argv+1));
            usage (1);
         }
      }
      else if (fp1==NULL)
         fp1 = getfp(*argv, "r");
      else
         fp = getfp(*argv, "r");

   if (l>0) {
      x = dgetmem(l+l);
      y = x + l;
      while (freadf(x, sizeof(*x), l, fp)==l &&
             freadf(y, sizeof(*y), l, fp1)== l ) {
         z = rmse(x, y, l);
         fwritef(&z, sizeof(z), 1, stdout);
      }
   }
   else {
      while (freadf(&x1, sizeof(x1), 1, fp)==1 &&
             freadf(&y1, sizeof(y1), 1, fp1)== 1) {
         sub = x1 - y1;
         z += sub * sub;

         num++;
      }
      z = sqrt(z /= num);
      fwritef(&z, sizeof(z), 1, stdout);
   }
   
   return(0);
}

