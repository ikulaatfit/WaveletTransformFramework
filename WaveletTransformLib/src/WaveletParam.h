#ifndef WAVELET_PARAM_H
#define WAVELET_PARAM_H
#include <cstdio>
#include <sstream>
#include "Debug.h"
#include "WaveletKernelGeneratorTypes.h"

/**
* @brief Rectangle structure.
*/
typedef struct s_rect
{
  union{
    size_t w; ///< rectangle width
    size_t x;
  };
  union{
    size_t h; ///< rectangle height
    size_t y;
  };
  /**
  * Default rectangle constructor - set rectangle dimensions
  * @param w rectangle width
  * @param h rectangle height
  */
  s_rect(size_t w = 0, size_t h = 0)
  {
    this->w = w;
    this->h = h;
  };
  /**
  * Is dimension valid?
  * @return dimensions validity
  */
  bool isValid()
  {
    return (this->w > 0) && (this->h > 0);
  }
  /**
  * Set rectangle dimensions
  * @param w rectangle width
  * @param h rectangle height
  */
  void set(size_t w = 0, size_t h = 0)
  {
    this->w = w;
    this->h = h;
  };
  /**
  * Set udefined values.
  */
  void clear()
  {
    this->w = 0;
    this->h = 0;
  };

  bool operator==(const s_rect& rhs) const
  {
    return (rhs.h == this->h) && (rhs.w == this->w);
  }

  bool operator!=(const s_rect& rhs) const
  {
    return !(*this == rhs);
  }
  /**
  * Get rectangle size.
  * @return rectangle size
  */
  size_t getSize()
  {
    return this->w * this->h;
  };
}rect;

enum engine_type
{
    ENGINE_TYPE_OPENMP_SEP = 0,
    ENGINE_TYPE_OPENMP_COMB,
    ENGINE_TYPE_OPENCL_SEP,
    ENGINE_TYPE_OPENCL_COMB    
};

enum e_wavelet_type
{  
  WAVELET_TYPE_CDF97,
  WAVELET_TYPE_CDF53,
  WAVELET_TYPE_DD137,
  WAVELET_TYPE_N1715,
  WAVELET_TYPE_N2523,
  WAVELET_TYPE_N2521
};

/**
 * @brief Wavelet interface
 */
class WaveletParam
{
	public:
		unsigned int max_depth;
		bool interlaced;
    e_wavelet_type wavelet;
    rect scaled_image_size;
    s_wavelet_type_info *wavelet_info;
    /**
    * Transcode frame.
    * @param bgra_frame frame in BGRA format to transcode
    * @param out optional frame process timers
    * @param error optional string to write error
    */
    //void printError(){fprintf(stderr, this->error_msg.str().c_str());}
    void setWaveletType(e_wavelet_type wavelet);
	protected:
    WaveletParam(engine_type in_engine);
    ~WaveletParam();
    void printDebug();
    engine_type engine;
    //std::stringstream error_msg;
};

#endif

