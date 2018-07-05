NAFILTH = test-nafil.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -DPKGDATADIR='"$(pkgdatadir)"'

bin_PROGRAMS = test-nafil

test_nafil_SOURCES = test-nafil.cc ${KYTH}
test_nafil_LDADD = ../lib/libnafil.la $(BOOST_REGEX_LIB)
