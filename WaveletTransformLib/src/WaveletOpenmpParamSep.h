#ifndef WAVELET_OPENMP_PARAM_SEP_H
#define WAVELET_OPENMP_PARAM_SEP_H

#include "WaveletOpenmpParam.h"
#include "Wavelet.h"
#include <sstream>

typedef enum e_openmp_sep_func
{
  OPENMP_SEP_FUNC_SINGLE,
  OPENMP_SEP_FUNC_OPENMP,
  OPENMP_SEP_FUNC_OPENMP_COPY,
  OPENMP_SEP_FUNC_OPENMP_SIMD,
  OPENMP_SEP_FUNC_OPENMP_COPY_SIMD
}openmp_sep_func;

class WaveletOpenmpParamSep : public WaveletOpenmpParam
  {
    public:
		  WaveletOpenmpParamSep();

      void clear();

      void printDebug();
      openmp_sep_func hor_func;
      openmp_sep_func vert_func;
  };

#endif
