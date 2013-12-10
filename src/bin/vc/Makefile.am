## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.2 2013/12/10 10:15:03 mataki Exp $

AM_CPPFLAGS = -I$(srcdir)/hts_engine_API \
              -I$(top_srcdir)/include
bin_PROGRAMS = vc
vc_SOURCES = vc.c _vc.c \
            $(srcdir)/hts_engine_API/HTS_misc.c \
            $(srcdir)/hts_engine_API/HTS_pstream.c \
            $(srcdir)/hts_engine_API/HTS_sstream.c
vc_LDADD = $(top_builddir)/lib/libSPTK.a
