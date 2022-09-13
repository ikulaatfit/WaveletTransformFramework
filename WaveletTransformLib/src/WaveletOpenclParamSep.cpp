#include "WaveletOpenclParamSep.h"

WaveletOpenclParamSep::WaveletOpenclParamSep() : WaveletOpenclParam(ENGINE_TYPE_OPENCL_SEP)
  {
    this->clear();
  }
/**
  * Set initial undefined value.
  */
void WaveletOpenclParamSep::clear()
  {
    this->hor_sizes = proc_dim(256,1);
    this->vert_sizes = proc_dim(32,8);
    this->vert_kernel = std::string("wavelet_vert_line");
    this->hor_kernel = std::string("wavelet_hor_basic");
  }
/**
  * Get program build parameters.
	* @param block_size threads in block count
	* @param stat_creating statistics creating type
	* @return parameters
  */
std::string WaveletOpenclParamSep::createBuildParam()
  {
    std::ostringstream str;
    str << this->WaveletOpenclParam::createBuildParam()
        << " -D HOR_BLOCK_SIZE=" << this->hor_sizes.count()
        << " -D HOR_BLOCK_SIZE_Y=" << this->hor_sizes.y
        << " -D VERT_BLOCK_SIZE=" << this->vert_sizes.count() 
        << " -D VERT_BLOCK_SIZE_X=" << this->vert_sizes.x
        << " -D " << this->vert_kernel
        << " -D " << this->hor_kernel;
    #if DEBUG_LEVEL == DEBUG_ALL
      this->printDebug();
    #endif
    return str.str();
  }
/**
  * Print debug information
  */
void WaveletOpenclParamSep::printDebug()
  {
    fprintf(stderr, " hor kernel name:         %s\n"
                    " hor kernel local sizes:  %dx%d\n"
                    " vert kernel name:        %s\n"
                    " vert kernel local sizes: %dx%d\n", this->hor_kernel.c_str(), this->hor_sizes.x, this->hor_sizes.y, this->vert_kernel.c_str(), this->vert_sizes.x, this->vert_sizes.y);
  }