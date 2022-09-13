#ifndef WAVELET_OPENCL_PARAM_SEP_H
#define WAVELET_OPENCL_PARAM_SEP_H

#include "Wavelet.h"
#include "WaveletOpenclParam.h"
#include <sstream>

/**
 * @brief Program parameters structure
 */
class WaveletOpenclParamSep : public WaveletOpenclParam
  {
    public:
		  WaveletOpenclParamSep();

      void clear();

      std::string createBuildParam();

      void printDebug();
      proc_dim hor_sizes;
      proc_dim vert_sizes;
      std::string vert_kernel;
      std::string hor_kernel;
  };

#endif
