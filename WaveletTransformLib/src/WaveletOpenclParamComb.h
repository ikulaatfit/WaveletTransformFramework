#ifndef WAVELET_OPENCL_PARAM_COMB_H
#define WAVELET_OPENCL_PARAM_COMB_H

#include "Wavelet.h"
#include "WaveletOpenclParam.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "WaveletKernelGeneratorGpu.h"
#include "WaveletKernelGeneratorCpu.h"

/**
 * @brief Program parameters structure
 */
class WaveletOpenclParamComb : public WaveletOpenclParam
  {
    public:
		  /**
       * Default constructor for set initial undefined value.
       */
		  WaveletOpenclParamComb();
		  /**
       * Get program build parameters.
		   * @param block_size threads in block count
		   * @param stat_creating statistics creating type
		   * @return parameters
       */
      std::string createBuildParam();
      /**
       * Print debug information
       */
      void printDebug();
      void clear();
      bool setKernel(std::string &kernel_name);
      
      proc_dim comb_sizes;
      std::string comb_kernel;
      cl_uint comb_hor_corners_proc;
      e_kernel_type kernel_type;
      bool optim_thread;
      std::string gen_data;
	  e_intrinsics instrinsics;
	  //WaveletKernelGeneratorCpu<INTRINSICS_NONE, false> *kernel_generator;
	  WaveletKernelGeneratorGpu *kernel_generator;
   private:
      std::string getFirDef(std::vector<float> &fir, std::string &filter_name);
      std::string getFirDefs();
      
      bool isHorOpAtomic();
  };

#endif
