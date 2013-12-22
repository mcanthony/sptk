## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.7 2013/12/22 16:58:55 uratec Exp $
lpcoefdir = $(datarootdir)/SPTK

bin_PROGRAMS = ds
ds_SOURCES = ds.c 
ds_LDADD = $(top_builddir)/lib/libSPTK.a
ds_CPPFLAGS = -DLIB=\"$(lpcoefdir)\" -I$(top_srcdir)/include

dist_lpcoef_DATA = lpfcoef.2to1 \
	lpfcoef.3to2 \
	lpfcoef.4to3 \
	lpfcoef.5to2dn \
	lpfcoef.5to2up \
	emime/lpfcoef.7to4
