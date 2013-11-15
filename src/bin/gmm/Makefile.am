## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.6 2013/11/15 06:23:01 mataki Exp $
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = gmm gmmp
gmm_SOURCES = _gmm.c gmm.c gmm.h
gmm_LDADD = $(top_builddir)/lib/libSPTK.a
gmmp_SOURCES = _gmm.c gmmp.c gmm.h
gmmp_LDADD = $(top_builddir)/lib/libSPTK.a
