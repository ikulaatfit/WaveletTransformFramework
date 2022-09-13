#ifndef WAVELET_OPENMP_PARAM_H
#define WAVELET_OPENMP_PARAM_H

#include "WaveletParam.h"
#include "Wavelet.h"
#include <sstream>

class WaveletOpenmpParam : public WaveletParam
  {
    public:
		  WaveletOpenmpParam(engine_type in_engine = ENGINE_TYPE_OPENMP_SEP);

      void clear();

      void printDebug();
  };

#endif
