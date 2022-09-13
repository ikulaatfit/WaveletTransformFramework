#include "WaveletOpenmpParamSep.h"

WaveletOpenmpParamSep::WaveletOpenmpParamSep() : WaveletOpenmpParam(ENGINE_TYPE_OPENMP_SEP)
  {
    this->clear();
  }

void WaveletOpenmpParamSep::clear()
  {
    this->hor_func = OPENMP_SEP_FUNC_OPENMP_COPY_SIMD;
    this->vert_func = OPENMP_SEP_FUNC_OPENMP_COPY_SIMD;
  }

void WaveletOpenmpParamSep::printDebug()
  {
    std::string hor_func_name, vert_func_name;
    switch(this->hor_func)
      {
        case OPENMP_SEP_FUNC_SINGLE:
          hor_func_name = "transformSepSingle";
        break;
        case OPENMP_SEP_FUNC_OPENMP:
          hor_func_name = "transformSepOpenmp";
        break;
        case OPENMP_SEP_FUNC_OPENMP_COPY:
          hor_func_name = "transformSepOpenmpCopy";
        break;
        case OPENMP_SEP_FUNC_OPENMP_SIMD:
          hor_func_name = "transformSepOpenmpSimd";
        break;
        case OPENMP_SEP_FUNC_OPENMP_COPY_SIMD:
          hor_func_name = "transformSepOpenmpCopySimd";
        break;
      }
    switch(this->vert_func)
      {
        case OPENMP_SEP_FUNC_SINGLE:
          vert_func_name = "transformSepSingle";
        break;
        case OPENMP_SEP_FUNC_OPENMP:
          vert_func_name = "transformSepOpenmp";
        break;
        case OPENMP_SEP_FUNC_OPENMP_COPY:
          vert_func_name = "transformSepOpenmpCopy";
        break;
        case OPENMP_SEP_FUNC_OPENMP_SIMD:
          vert_func_name = "transformSepOpenmpSimd";
        break;
        case OPENMP_SEP_FUNC_OPENMP_COPY_SIMD:
          vert_func_name = "transformSepOpenmpCopySimd";
        break;
      }
    fprintf(stderr, " hor kernel name:        %s\n"
                    " vert kernel name:       %s\n", hor_func_name.c_str(), vert_func_name.c_str());
  }