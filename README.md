ScuttleButt
===========

ScuttleButt library (See http://www.cs.cornell.edu/home/rvr/papers/flowgossip.pdf)

( This project was originally conceived by reading the source code of https://github.com/dominictarr/scuttlebutt
  and the testing that I do is done against that implementation, meaning it should be compatible with apps
  built using his module.
  
  I have no shame. )

This project depends on jansson for JSON processing: https://github.com/akheron/jansson


If you want to just build the library, after using configure go into the
`src/` directory and `make libscuttle.a`

Otherwise it wants to build the test program `scuttlebutt`, whose source
code is in `main.cpp`.

What's it Do?
=============
The idea of the protocol is to reconcile data across systems. Giving you data that can be
shared in, for instance, a distributed environment.

It uses JSON to encode the data. The rationale for this is pretty simple: JSON is unicode
text, is compact (esp. compared to, i.e. XML), and is supported across almost all known
programming languages with parsers/encoders.

Messages are structured as [payload, version, source_id], such that the version is a timestamp
of seconds since epoch including fractional seconds, and the source_id is a unique id number.

The source id in the library is generated randomly using a method found in dominictarr's
version, and it's "good enough". If you have an idea for a better one let me know.


Compiling Notes
===============
Haven't done c++ in a while, so...  The Makefile.am specifies `-lrt` while that is
entirely unnecessary on my OSX Mavericks box. So, feel free to screw around the autotools
files to try and get it to "do the right thing".  I'm pretty horrible at autotools.

Oh, and you need both Jansson and libuv if you are wanting to compile the test program
that resides in `main.cpp`

I don't actually want this library to "stream the things", just return the things what
need to be streamed. In that vein, the eventual library built on top of this one that
implements the store will also use libuv to handle all the streaming...

( That libuv is pretty genius, btb. You should check it out at http://github.com/joyent/libuv )
