## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.9 2014/03/26 13:35:08 uratec Exp $
AM_CPPFLAGS = -I$(top_srcdir)/include -DLIB=\"$(pkgdatadir)\"
bin_PROGRAMS = ds
ds_SOURCES = ds.c 
ds_LDADD = $(top_builddir)/lib/libSPTK.a

dist_pkgdata_DATA = lpfcoef.2to1 \
                    lpfcoef.3to2 \
                    lpfcoef.4to3 \
                    lpfcoef.5to2dn \
                    lpfcoef.5to2up \
                    titech/lpfcoef.7to4
