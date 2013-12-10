## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.1 2013/12/10 08:36:54 mataki Exp $

AM_CPPFLAGS = -I$(srcdir)/hts_engine_API \
              -I$(top_srcdir)/include
bin_PROGRAMS = vc
vc_SOURCES = vc.c _vc.c \
            $(srcdir)/hts_engine_API/HTS_misc.c \
            $(srcdir)/hts_engine_API/HTS_pstream.c \
            $(srcdir)/hts_engine_API/HTS_sstream.c \
            $(srcdir)/hts_engine_API/HTS_label.c \
            $(srcdir)/hts_engine_API/HTS_model.c
vc_LDADD = $(top_builddir)/lib/libSPTK.a
