#include "WaveletParam.h"

float wavelet_type_cdf53_coef[] = { -0.5f, 0.25f };
s_wavelet_type_info wavelet_type_cdf53 = { wavelet_type_cdf53_coef, 1.4142135623730f, 1, 1, std::string("WAVELET_TYPE_CDF53") };

float wavelet_type_cdf97_coef[] = { -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f };
s_wavelet_type_info wavelet_type_cdf97 = { wavelet_type_cdf97_coef, 1.1496043988602f, 2, 1, std::string("WAVELET_TYPE_CDF97") };

float wavelet_type_dd137_coef[] = { -0.5625f, 0.0625f, -0.28125f, 0.03125f };
s_wavelet_type_info wavelet_type_dd137 = { wavelet_type_dd137_coef, 1.0f, 1, 2, std::string("WAVELET_TYPE_DD137") };


float wavelet_type_n1715_coef[] = { -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f,  -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f };
s_wavelet_type_info wavelet_type_n1715 = { wavelet_type_n1715_coef, 1.1496043988602f, 4, 1, std::string("WAVELET_TYPE_N1715") };

float wavelet_type_n2523_coef[] = { -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f,  -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f, -1.58613434342059f, -0.0529801185729f, 0.8829110755309f, 0.4435068520439f };
s_wavelet_type_info wavelet_type_n2523 = { wavelet_type_n2523_coef, 1.1496043988602f, 6, 1, std::string("WAVELET_TYPE_N2523") };

float wavelet_type_n2521_coef[] = { -0.5625f, 0.0625f, -0.28125f, 0.03125f, -0.5625f, 0.0625f, -0.28125f, 0.03125f };
s_wavelet_type_info wavelet_type_n2521 = { wavelet_type_n2521_coef, 1.0f, 2, 2, std::string("WAVELET_TYPE_DD2521") };



void WaveletParam::setWaveletType(e_wavelet_type wavelet)
{
  switch(wavelet)
  {
    case WAVELET_TYPE_CDF53:
      this->wavelet_info = &wavelet_type_cdf53;
    break;
    case WAVELET_TYPE_CDF97:
      this->wavelet_info = &wavelet_type_cdf97;
    break;
    case WAVELET_TYPE_DD137:
      this->wavelet_info = &wavelet_type_dd137;
    break;

	case WAVELET_TYPE_N1715:
		this->wavelet_info = &wavelet_type_n1715;
		break;

	case WAVELET_TYPE_N2523:
		this->wavelet_info = &wavelet_type_n2523;
		break;
	case WAVELET_TYPE_N2521:
		this->wavelet_info = &wavelet_type_n2521;
		break;
    default:
      return;  
  }
  this->wavelet = wavelet;
}

WaveletParam::WaveletParam(engine_type in_engine) : engine(in_engine)
{
  wavelet = WAVELET_TYPE_CDF53;
  max_depth = 1;
  interlaced = false;
  this->setWaveletType(this->wavelet);
}

WaveletParam::~WaveletParam()
{
}
/**
  * Print debug information
  */
void WaveletParam::printDebug()
{
  std::string engine_type;
  switch(this->engine)
  {
    case ENGINE_TYPE_OPENCL_SEP:
      engine_type = "opencl";
      break;
    case ENGINE_TYPE_OPENCL_COMB:
      engine_type = "openclcomb";
      break;
    case ENGINE_TYPE_OPENMP_SEP:
    case ENGINE_TYPE_OPENMP_COMB:
      engine_type = "openmp";
      break;
  }
  std::string wavelet_type;
  switch(this->wavelet)
  {
    case WAVELET_TYPE_CDF53:
      wavelet_type = "CDF5/3";
      break;
    case WAVELET_TYPE_CDF97:
      wavelet_type = "CDF9/7";
      break;
    case WAVELET_TYPE_DD137:
      wavelet_type = "DD13/7";
      break;
  }
  fprintf(stderr, "\nProgram configuration:\n"
    " engine type:             %s\n"
    " wavelet type:            %s\n"
    " scaled image size:       %dx%d\n", engine_type.c_str(), wavelet_type.c_str(), (int)scaled_image_size.x, (int)scaled_image_size.y);
}