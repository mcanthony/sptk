/* Copyright (c) 2009-2011 Kyle Gorman
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*
*  SWIPE' pitch estimator
*  Camacho, Arturo. A sawtooth waveform inspired pitch estimator for speech   
*  and music. Doctoral dissertation, University of Florida. 2007.             
*  Implemented in C by Kyle Gorman <kgorman@ling.upenn.edu>
*/
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
/*                1996-2011  Nagoya Institute of Technology          */
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

static char *rcs_id = "$Id: swipe.c,v 1.1 2011/11/04 05:00:00 sawada11 Exp $";

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#if 0
#include <fftw3.h>   // http://www.fftw.org/
#include <sndfile.h> // http://www.mega-nerd.com/libsndfile/
#endif

#include "vector.h"  // comes with release

#define NOK                          0

#define TRUE                         1
#define FALSE                        0

#define DERBS                        .1 
#define POLYV                        .0013028 //  1 / 12 / 64 = 1 / 768
#define DLOG2P                       .0104167 // 1/96

#define ST                           .3  // Feel free to change these
#define DT                           .001
#define MIN                          100.
#define MAX                          600.

#define VNUM                         1.1 // Current version

#ifndef NAN                          
    #define NAN                      sqrt(-1.)
#endif

#ifndef isnan
int isnan(double x) { 
    return(x !=x);
}
#endif

#ifndef log2
double log2(double x) { // A base-2 log function
    return(log(x) / log(2.));
}
#endif

#ifndef round
double round(double x) { // Rounds a double to the nearest integer value
    return(x >= 0. ? floor(x + .5) : floor(x - .5));
}
#endif

double hz2mel(double hz) { // Converts from hertz to Mel frequency
    return(1127.01048 * log(1. + hz / 700.));
}

double hz2erb(double hz) { // Converts from hertz to ERBs
    return(21.4 * log10(1. + hz / 229.));
}

double erb2hz(double erb) { // Converts from ERBs to hertz 
    return((pow(10, erb / 21.4) - 1.) * 229.);
}

double fixnan(double x) { // A silly function that treats NaNs as 0.
    return(isnan(x) ? 0. : x);
}


#if 0
#else
typedef double out_complex[2];
typedef struct _float_list {
    float f;
    struct _float_list *next;
} float_list;
#endif

#if 0
// A helper function for loudness() for individual fft slices
void La(matrix L, vector f, vector fERBs, fftw_plan plan, 
	fftw_complex* fo, int w2, int hi, int i) {
    int j;


    fftw_execute(plan);

#else
void La(matrix L, vector f, vector fERBs, 
	out_complex* fo, int w2, int hi, int i, double* fi) {
    int j;

    double* fi_tmp = malloc(sizeof(double) * w2*2);
    double* fo_tmp = malloc(sizeof(double) * w2*2); 

    for(j=0;j<w2 * 2;j++){
      fi_tmp[j] = fi[j];
    }

    fftr(fi_tmp,fo_tmp,w2*2);

    for(j=0;j<w2*2;j++){
      fo[j][0] = fi_tmp[j];
      fo[j][1] = fo_tmp[j];
    }

#endif

    vector a = makev(w2);
    for (j = 0; j < w2; j++) { // This iterates over only the first half
        a.v[j] = sqrt(fo[j][0] * fo[j][0] + fo[j][1] * fo[j][1]);
    }
    vector a2 = spline(f, a); // a2 is now the result of the cubic spline
    L.m[i][0] = fixnan(sqrt(splinv(f, a, a2, fERBs.v[0], hi)));
    for (j = 1; j < L.y; j++) { // Perform a bisection query at ERB intervals
        hi = bilookv(f, fERBs.v[j], hi); 
        L.m[i][j] = fixnan(sqrt(splinv(f, a, a2, fERBs.v[j], hi)));
    }
    freev(a); 
    freev(a2);
}

//  A function for populating the loudness matrix with a signal x
matrix loudness(vector x, vector fERBs, double nyquist, int w, int w2) { // L
    int i;
    int j; 
    int hi;
    int offset = 0;
    double td = nyquist / w2; // This is equivalent to fstep
    // Testing showed this configuration of fftw to be fastest for speech-range 
#if 0
    double* fi = fftw_malloc(sizeof(double) * w); 
    fftw_complex* fo = fftw_malloc(sizeof(fftw_complex) * w);
    fftw_plan plan = fftw_plan_dft_r2c_1d(w, fi, fo, FFTW_ESTIMATE); 
#else
    double* fi = malloc(sizeof(double) * w); 
    out_complex* fo = malloc(sizeof(out_complex) * w);
#endif
    vector hann = makev(w); // This defines the Hann[ing] window
    for (i = 0; i < w; i++) {
        hann.v[i] = .5 - (.5 * cos(2. * M_PI * ((double) i / w)));
    }
    vector f = makev(w2);
    for (i = 0; i < w2; i++) { 
        f.v[i] = i * td;
    }
    hi = bisectv(f, fERBs.v[0]); // All calls to La() will begin here
    matrix L = makem(ceil((double) x.x / w2) + 1, fERBs.x); 
    for (j = 0; j < w2; j++) { // Left boundary case
        fi[j] = 0.; // More explicitly, 0. * hann.v[j]
    }
    for (/* j = w2 */; j < w; j++) {
        fi[j] = x.v[j - w2] * hann.v[j];
    }
#if 0
       La(L, f, fERBs, plan, fo, w2, hi, 0); 
#else 
       La(L, f, fERBs, fo, w2, hi, 0, fi); 
#endif
    for (i = 1; i < L.x - 2; i++) { // Middle case 
        for (j = 0; j < w; j++) {
            fi[j] = x.v[j + offset] * hann.v[j];
        }
#if 0
       La(L, f, fERBs, plan, fo, w2, hi, i); 
#else 
       La(L, f, fERBs, fo, w2, hi, i, fi); 
#endif
        offset += w2;
    }
    for (/* i = L.x - 2; */; i < L.x; i++) { // Right two boundary cases
        for (j = 0; j < x.x - offset; j++) { // This dies at x.x + w2
            fi[j] = x.v[j + offset] * hann.v[j];
        }
        for (/* j = x.x - offset */; j < w; j++) {
            fi[j] = 0.; // Once again, 0. * hann.v[j] 
        }
#if 0
       La(L, f, fERBs, plan, fo, w2, hi, i); 
#else 
       La(L, f, fERBs, fo, w2, hi, i, fi); 
#endif
        offset += w2;
    } // Now L is fully valued
    freev(hann);
    freev(f);
    // L must now be normalized
    for (i = 0; i < L.x; i++) { 
        td = 0.; // td is the value of the normalization factor
        for (j = 0; j < L.y; j++) { 
            td += L.m[i][j] * L.m[i][j];
        }
        if (td != 0.) { // Catches zero-division
            td = sqrt(td);
            for (j = 0; j < L.y; j++) {
                L.m[i][j] /= td;
            }
        } // Otherwise, it is already 0.
    } 
#if 0
    fftw_destroy_plan(plan); 
    fftw_free(fi); 
    fftw_free(fo); 
#else
    //    fftw_destroy_plan(plan); 
    free(fi);
    free(fo);
#endif
    return(L);
}

// Populates the strength matrix using the loudness matrix
Sadd(matrix S, matrix L, vector fERBs, vector pci, vector mu, intvector ps,
                                            double dt, double nyquist2, 
                                              int lo, int hi, int psz, int w2) {
    int i;
    int j;
    int k; 
    int plim;
    double t = 0.;
    double tp = 0.;
    double td; 
    double dtp = w2 / nyquist2;
    matrix Slocal = zerom(psz, L.x);
    for (i = 0; i < Slocal.x; i++) {
        vector q = makev(fERBs.x);
        for (j = 0; j < q.x; j++) {
            q.v[j] = fERBs.v[j] / pci.v[i];
        }
        plim = floor((fERBs.v[fERBs.x - 1] / pci.v[i]) - .75);
        vector kernel = zerov(fERBs.x); // A zero-filled kernel vector
        for (j = 0; j < ps.x; j++) { 
            if PRIME(ps.v[j]) {
                for (k = 0; k < kernel.x; k++) {
                    td = fabs(q.v[k] - j - 1.); // Avoids calculating this twice
                    if (td < .25) { // Peaks
                        kernel.v[k] = cos(2. * M_PI * q.v[k]);
                    }
                    else if (td < .75) { // Valleys
                        kernel.v[k] += cos(2. * M_PI * q.v[k]) / 2.;
                    }
                }
            }
        }
        freev(q);
        td = 0.; 
        for (j = 0; j < kernel.x; j++) {
            kernel.v[j] *= sqrt(1. / fERBs.v[j]); // Applying the envelope
            if (kernel.v[j] > 0.) { 
                td += kernel.v[j] * kernel.v[j];
            }
        }
        td = sqrt(td); // Now, td is the p=2 norm factor
        for (j = 0; j < kernel.x; j++) { // Normalize the kernel
            kernel.v[j] /= td;
        }
        for (j = 0; j < L.x; j++) { 
            for (k = 0; k < L.y; k++) {
                Slocal.m[i][j] += kernel.v[k] * L.m[j][k]; // i.e, kernel' * L
            } 
        }
        freev(kernel);
    } // Slocal is filled out; time to interpolate
    k = 0; 
    for (j = 0; j < S.y; j++) { // Determine the interpolation params 
        td = t - tp; 
        while (td >= 0.) {
            k++;
            tp += dtp;
            td -= dtp;
        } // td now equals the time difference
        for (i = 0; i < psz; i++) {
            S.m[lo + i][j] += (Slocal.m[i][k] + (td * (Slocal.m[i][k] -
                                    Slocal.m[i][k - 1])) / dtp) * mu.v[i];
        }
        t += dt;
    }
    freem(Slocal);
}

// Helper function for populating the strength matrix for the left boundary case
Sfirst(matrix S, vector x, vector pc, vector fERBs, vector d, 
                                  intvector ws, intvector ps, double nyquist, 
                                            double nyquist2, double dt, int n) {
    int i; 
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = 0; // The start of Sfirst-specific code
    int hi = bisectv(d, 2.);
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    for (i = 0; i < hi; i++) { 
        pci.v[i] = pc.v[i];
        mu.v[i] = 1. - fabs(d.v[i] - 1.);
    } // End of Sfirst-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, hi, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// Generic helper function for populating the strength matrix
Snth(matrix S, vector x, vector pc, vector fERBs, vector d, intvector ws,
                                intvector ps, double nyquist, double nyquist2, 
                                                             double dt, int n) {
    int i;
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = bisectv(d, n); // Start of Snth-specific code
    int hi = bisectv(d, n + 2);
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    int ti = 0;
    for (i = lo; i < hi; i++) {
        pci.v[ti] = pc.v[i];
        mu.v[ti] = 1. - fabs(d.v[i] - (n + 1));
        ti++;
    } // End of Snth-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, hi, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// Helper function for populating the strength matrix from the right boundary
Slast(matrix S, vector x, vector pc, vector fERBs, vector d, 
                                 intvector ws, intvector ps, double nyquist, 
                                            double nyquist2, double dt, int n) {
    int i;
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = bisectv(d, n); // Start of Slast-specific code
    int hi = d.x;
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    int ti = 0;
    for (i = lo; i < hi; i++) {
        pci.v[ti] = pc.v[i];
        mu.v[ti] = 1. - fabs(d.v[i] - (n + 1));
        ti++;
    } // End of Slast-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, hi, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// Peforms polynomial tuning on the strength matrix to determine the pitch
vector pitch(matrix S, vector pc, double st) {
    int i;
    int j;
    int maxi; 
    int search = (int) round((log2(pc.v[2]) - log2(pc.v[0])) / POLYV + 1.);
    double nftc; 
    double maxv;
    double log2pc;
    double tc2 = 1. / pc.v[1];
    vector coefs;
    vector s = makev(3);
    vector ntc = makev(3);
    ntc.v[0] = ((1. / pc.v[0]) / tc2 - 1.) * 2. * M_PI; 
    ntc.v[1] = (tc2 / tc2 - 1.) * 2. * M_PI; 
    ntc.v[2] = ((1. / pc.v[2]) / tc2 - 1.) * 2. * M_PI;
    vector p = makev(S.y);  
    for (j = 0; j < S.y; j++) {
        maxv = SHRT_MIN;  
        for (i = 0; i < S.x; i++) { 
            if (S.m[i][j] > maxv) {
                maxv = S.m[i][j];
                maxi = i;
            }
        }
        if (maxv > st) { // Make sure it's big enough
            if (maxi == 0 || maxi == S.x - 1) { // First or last? 
                p.v[j] = pc.v[0];
            }
            else { // Generic case
                tc2 = 1. / pc.v[maxi];
                log2pc = log2(pc.v[maxi - 1]); 
                s.v[0] = S.m[maxi - 1][j];
                s.v[1] = S.m[maxi][j];
                s.v[2] = S.m[maxi + 1][j]; 
                coefs = polyfit(ntc, s, 2); 
                maxv = SHRT_MIN; 
                for (i = 0; i < search; i++) { // Check the nftc space
                    nftc = polyval(coefs, ((1. / pow(2, i * POLYV + log2pc)) / 
                                                          tc2 - 1) * 2 * M_PI);
                    if (nftc > maxv) {
                        maxv = nftc;
                        maxi = i;
                    }
                } // Now we've got the pitch numbers we need
                freev(coefs);
                p.v[j] = pow(2, log2pc + (maxi * POLYV));
            }
        }
        else { // If not voiced during that interval, then...
#if 0
            p.v[j] = NAN;
#else
            p.v[j] = 0.0;
#endif
        } 
    }
    freev(ntc);
    freev(s);
    return(p);
}

// Primary utility function for each pitch extraction
#if 0
void swipe(char wav[], double min, double max, double st, double dt) {
#else
  void swipe(float_list *input, int length, double min, double max, double st, double dt, double samplerate, int otype) {
#endif
    int i; 
    double td;
#if 0
    FILE* wavf;
    SF_INFO info;
    SNDFILE* source;
    if (strcmp(wav, "<STDIN>") == 0) { // i.e., is coming from STDIN
        wavf = stdin;
    }
    else { // is specified
        wavf = fopen(wav, "r");
    }
    source = sf_open_fd(fileno(wavf), SFM_READ, &info, TRUE);
#else
    double tmp;
    float_list *tmpf;
    vector x = makev(length);
    for (i = 0, tmpf = input; tmpf != NULL; i++, tmpf = tmpf->next) 
      x.v[i] = tmpf->f / 32768.0; // normalized by max_short
#endif

#if 0
    // Perform checks on the wav header
    if (info.sections < 1) {
        fprintf(stderr, "File or stream %s not read as audio ... \n", wav);
        return(makev(0)); // This will be detected as an error
    }
    double nyquist = info.samplerate / 2.; 
    double nyquist2 = info.samplerate; // Used so g.d. often here...
    double nyquist16 = info.samplerate * 8.; 
#else
    double nyquist = samplerate / 2.; 
    double nyquist2 = samplerate; // Used so g.d. often here...
    double nyquist16 = samplerate * 8.; 
#endif

    if (max > nyquist) { 
        max = nyquist;
        fprintf(stderr, "Max pitch > Nyquist ... max set to %.2f Hz.\n", max);
    }
    if (dt > nyquist2) {
        dt = nyquist2;
        fprintf(stderr, "Timestep > SR ... timestep set to %f.\n", nyquist2);
    }
    intvector ws = makeiv(round(log2((nyquist16) / min) -  
                                log2((nyquist16) / max)) + 1); 
    for (i = 0; i < ws.x; i++) {
        ws.v[i] = pow(2, round(log2(nyquist16 / min))) / pow(2, i);
    }
    vector pc = makev(ceil((log2(max) - log2(min)) / DLOG2P));
    vector d = makev(pc.x);
    for (i = pc.x - 1; i >= 0; i--) { // Doing this bckwards saves a computation
        td = log2(min) + (i * DLOG2P);
        pc.v[i] = pow(2, td);
        d.v[i] = 1. + td - log2(nyquist16 / ws.v[0]); 
    } // td now equals log2(min)

#if 0
    vector x = makev(info.frames); // This reads the signal in
    sf_read_double(source, x.v, x.x);
    sf_close(source); // Takes FILE* wavf with it, too

#else

#endif

    vector fERBs = makev(ceil((hz2erb(nyquist) - 
                               hz2erb(pow(2, td) / 4)) / DERBS));
    td = hz2erb(min / 4.);
    for (i = 0; i < fERBs.x; i++) { 
        fERBs.v[i] = erb2hz(td + (i * DERBS));
    }
    intvector ps = onesiv(floor(fERBs.v[fERBs.x - 1] / pc.v[0] - .75));
    sieve(ps);
    ps.v[0] = P; // Hack to make 1 "act" prime...don't ask 
    matrix S = zerom(pc.x, ceil(((double) x.x / nyquist2) / dt)); // Strength
    Sfirst(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, 0); 
    for (i = 1; i < ws.x - 1; i++) { // S is updated inline here
        Snth(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, i);
    } // i is now (ws.x - 1)
    Slast(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, i);
    freev(fERBs); 
    freeiv(ws);
    freeiv(ps);
    freev(d);
    freev(x);

    vector p = pitch(S, pc, st); // Find pitch using strength matrix
    freev(pc);
    freem(S);
#if 0
    return p;
#else 
    for (i = 0; i < p.x; i++) {
       switch(otype) {
          case 1:      /* f0 */
             fwritef(&p.v[i], sizeof(p.v[i]), 1, stdout);
             break;
          case 2:      /* log(f0) */
             if (p.v[i] != 0.0)
                p.v[i] = log(p.v[i]);
             else
                p.v[i] = -1.0E10;
             fwritef(&p.v[i], sizeof(p.v[i]), 1, stdout);
             break;
          default:     /* pitch */
             if (p.v[i] != 0.0)
                p.v[i] = samplerate / p.v[i];
             fwritef(&p.v[i], sizeof(p.v[i]), 1, stdout);
             break;
       }
    }
    freev(p);
#endif
}

// Function for printing the pitch vector returned by swipe()
void printp(vector p, char out[], double dt, int mel, int vlo) {
    int i;
    double t = 0.; 
    FILE* sink; // Handle for printing to file/STDOUT
    if (strcmp(out, "<STDOUT>") == 0) {
        sink = stdout;
    }
    else {
#if 0
        sink = fopen(out, "w");
#else
        sink = fopen(out, "wb");
#endif
        if (sink == NULL) {
            fprintf(stderr, "File or stream %s not writable, aborting.\n", out);
            exit(EXIT_FAILURE);
        }
    }
    if (mel) {
        if (vlo) {
            for (i = 0; i < p.x; i++) {
                fprintf(sink, "%4.4f %5.4f\n", t, hz2mel(p.v[i])); 
                t += dt;
            }
        }
        else { // Default case
            for (i = 0; i < p.x; i++) {
                if (!isnan(p.v[i])) {
                    fprintf(sink, "%4.4f %5.4f\n", t, hz2mel(p.v[i])); 
                }
                t += dt;
            }
        }
    }
    else {
        if (vlo) {
#if 0
#else
          float tmp_float;
          float tmp2_float;
#endif
            for (i = 0; i < p.x; i++) {
#if 0
                fprintf(sink, "%4.4f %5.4f\n", t, p.v[i]); 
#else
	      tmp_float = (float)p.v[i];
	      tmp2_float = (float)t;
	      fwrite(&tmp_float,sizeof(float),1,sink);
#endif
                t += dt;
            }
        }
        else { 
            for (i = 0; i < p.x; i++) {
                if (!isnan(p.v[i])) {
                    fprintf(sink, "%4.4f %5.4f\n", t, p.v[i]); 
                }
                t += dt;
            }
        }
    } 
    fclose(sink); 
}


