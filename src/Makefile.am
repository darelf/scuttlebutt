lib_LIBRARIES = libscuttle.a
libscuttle_a_SOURCES = scuttle.hpp scuttle.cpp
libscuttle_a_CPPFLAGS = @JANSSON_CFLAGS@

bin_PROGRAMS = scuttlebutt
scuttlebutt_SOURCES = main.cpp
scuttlebutt_CPPFLAGS = @JANSSON_CFLAGS@
scuttlebutt_LDADD = -luv libscuttle.a @JANSSON_LIBS@
