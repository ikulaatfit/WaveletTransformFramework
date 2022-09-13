#ifndef WAVELET_OPENCL_PARAM_H
#define WAVELET_OPENCL_PARAM_H

#include "Wavelet.h"
#include "WaveletParam.h"
#include <sstream>
#include <CL/cl.h>

enum wavelet_proc_type
{
    WAVELET_PROC_TYPE_BLAZ_NORMAL = 0,
    WAVELET_PROC_TYPE_BLAZ_REGISTER,
    WAVELET_PROC_TYPE_LAAN
};

enum opencl_dev_type
{
  OPENCL_DEV_TYPE_GPU = 0,
  OPENCL_DEV_TYPE_CPU
};

enum opencl_mem_type
{
  OPENCL_MEM_TYPE_GLOBAL = 0,
  OPENCL_MEM_TYPE_TEXTURE
};

typedef struct s_proc_dim
  {
    size_t x; ///< first size
    size_t y; ///< second size 
    size_t z; ///< third size
    /**
     * Initialize structure by 0 sizes.
     */
    s_proc_dim()
      {
        this->clear();
      };
    /**
     * Create flat 3 element structure from actual structure
     * @return flat structure
     */
    s_proc_dim flat()
      {
        return s_proc_dim(this->x*this->y*this->z);
      }
    void alignTo(s_proc_dim &align)
      {
        this->x = this->alignTo(x, align.x);
        this->y = this->alignTo(y, align.y);
        this->z = this->alignTo(z, align.z);
      }
    /**
     * Initialize structure by user sizes.
     * @param x first size
     * @param y second size
     * @param z third size
     */
    s_proc_dim(size_t x, size_t y = 1, size_t z = 1)
      {
        this->x = x;
        this->y = y;
        this->z = z;
      };
    /**
     * Clear structure by setting 0 to sizes.
     */
    void clear()
      {
        this->x = 0;
        this->y = 0;
        this->z = 0;
      }
    /**
     * Get total thread/group count.
     * @return total thread/group count
     */
    size_t count()
      {
        return this->x * this->y * this->z;
      };
    s_proc_dim operator *(const s_proc_dim &r_dim)
    {
      return s_proc_dim(x * r_dim.x, y * r_dim.y, z * r_dim.z);
    }
    s_proc_dim operator -(const s_proc_dim &r_dim)
    {
      return s_proc_dim(x - r_dim.x, y - r_dim.y, z - r_dim.z);
    }
    private:
      size_t alignTo(size_t num, size_t align)
        {
          return ((num + align - 1)/align)*align;
        }
  }proc_dim;

/**
 * @brief Program parameters structure
 */
class WaveletOpenclParam : public WaveletParam
  {
    public:

		  WaveletOpenclParam(engine_type in_engine = ENGINE_TYPE_OPENCL_SEP);

      void clear();

      virtual std::string createBuildParam();

      void printDebug();
      proc_dim pairs_per_thread;
      cl_device_type dev_type;
      cl_uint warp_size;
      wavelet_proc_type vert_proc;
      wavelet_proc_type hor_proc;
      cl_uint dev_id;
      cl_uint subdevice_size;
      opencl_mem_type image_mem_type;
      bool double_buffering;
      bool memless_exec;
      bool benchmark_proc;

      wavelet_optim_warp optim_warp;

      cl_uint repeat_count;
      bool gen_filter_body;
  };

#endif
