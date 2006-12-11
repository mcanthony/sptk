/*
  ----------------------------------------------------------------
	Speech Signal Processing Toolkit (SPTK): version 3.0
			 SPTK Working Group

		   Department of Computer Science
		   Nagoya Institute of Technology
				and
    Interdisciplinary Graduate School of Science and Engineering
		   Tokyo Institute of Technology
		      Copyright (c) 1984-2000
			All Rights Reserved.

  Permission is hereby granted, free of charge, to use and
  distribute this software and its documentation without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of this work, and to permit persons to whom this
  work is furnished to do so, subject to the following conditions:

    1. The code must retain the above copyright notice, this list
       of conditions and the following disclaimer.

    2. Any modifications must be clearly marked as such.

  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF TECHNOLOGY,
  SPTK WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM
  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF
  TECHNOLOGY, SPTK WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE
  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  PERFORMANCE OF THIS SOFTWARE.
 ----------------------------------------------------------------
*/

/************************************************************************
*									*
*    Interpolation							*
*									*
*					1996.4  K.Koishida		*
*									*
*	usage:								*
*		interpolate [ options ] [ infile ] > stdout		*
*	options:							*
*		-p p     :  interpolation period	[10]		*
*		-s s     :  start sample		[0]		*
*	infile:								*
*		data sequence						*
*		    , x(0), x(1), ...					*
*	stdout:								*
*		decimated data						*
*		    , 0, ..., 0, x(0), 0, ..., x(1), 0, ...,		*
*		      ---------	 ------------- -------------		*
*		         s-1           p            p			*
************************************************************************/

static char *rcs_id = "$Id: interpolate.c,v 1.3 2006/12/11 07:16:38 mr_alex Exp $";


/*  Standard C Libraries  */
#include <stdio.h>
#include <string.h>
#include <SPTK.h>


/*  Default Values  */
#define PERIOD		10
#define START		0


/*  Command Name  */
char	*cmnd;


void usage(int status)
{
    fprintf(stderr, "\n");
    fprintf(stderr, " %s - interpolation \n",cmnd);
    fprintf(stderr, "\n");
    fprintf(stderr, "  usage:\n");
    fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
    fprintf(stderr, "  options:\n");
    fprintf(stderr, "       -p p  : interpolation period [%d]\n", PERIOD);
    fprintf(stderr, "       -s s  : start sample         [%d]\n", START);
    fprintf(stderr, "       -h    : print this message\n");
    fprintf(stderr, "  infile:\n");
    fprintf(stderr, "       data sequence (float)        [stdin]\n");
    fprintf(stderr, "  stdout:\n");
    fprintf(stderr, "       interpolated data sequence (float)\n");
    fprintf(stderr, "\n");
    exit(status);
}


int main(int argc, char **argv)
{
    int		period = PERIOD, start = START, i;
    FILE	*fp = stdin;
    double	*x;
    
    
    if ((cmnd = strrchr(argv[0], '/')) == NULL)
	cmnd = argv[0];
    else
	cmnd++;
    while (--argc)
	if (**++argv == '-') {
	    switch (*(*argv+1)) {
		case 'p':
		    period = atoi(*++argv);
		    --argc;
		    break;
		case 's':
		    start = atoi(*++argv);
		    --argc;
		    break;
		case 'h':
		    usage(0);
		default:
		    fprintf(stderr, "%s : Invalid option '%c' !\n", cmnd, *(*argv+1));
		    usage(1);
		}
	}
	else 
	    fp = getfp(*argv, "r");

    x = dgetmem(period);
    
    fwritef(x, sizeof(*x), start, stdout);

    while(freadf(x, sizeof(*x), 1, fp) == 1)
	fwritef(x, sizeof(*x), period, stdout);

    exit(0);
}

