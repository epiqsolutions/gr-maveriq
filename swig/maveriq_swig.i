/* -*- c++ -*- */

#define MAVERIQ_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "maveriq_swig_doc.i"

%{
#include "maveriq/maveriq_source_s.h"
%}

%include "maveriq/maveriq_defs.h"

%include "maveriq/maveriq_source_s.h"
GR_SWIG_BLOCK_MAGIC2(maveriq, maveriq_source_s);
