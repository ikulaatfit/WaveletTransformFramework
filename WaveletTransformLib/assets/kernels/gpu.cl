#define WAVELET_OPTIM_WARP_NONE 0
#define WAVELET_OPTIM_WARP_LOCAL 1
#define WAVELET_OPTIM_WARP_SHUFFLE 2

#define DECLARE_VAR(type,name) type name;

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define MEM_TYPE_GLOBAL 0
#define MEM_TYPE_TEXTURE 1

#ifndef IMAGE_MEM_TYPE
  #define IMAGE_MEM_TYPE MEM_TYPE_GLOBAL
#endif

#if IMAGE_MEM_TYPE == MEM_TYPE_GLOBAL
  #define IN_IMAGE_MEM_TYPE(MEM_TYPE) __global MEM_TYPE *
  #define OUT_IMAGE_MEM_TYPE(MEM_TYPE) __global MEM_TYPE *
#else
  #define IN_IMAGE_MEM_TYPE(MEM_TYPE) __read_only image2d_t
  #define OUT_IMAGE_MEM_TYPE(MEM_TYPE) __write_only image2d_t
  const sampler_t image_samp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
#endif

// program constants
#if defined(WAVELET_TYPE_CDF53)
 #define FILTER_LENGTH 1
 #define BORDER_SIZE 1
 #define LIFTING_STEPS_COUNT (1)
 #define ALPHA (-0.5f)
 #define BETA (0.25f)
 #define ZETA1 (1.4142135623730f)
 #define ZETA2 (0.7071067811865f)

 #define PREDICT_COEF {-0.5f}
 #define UPDATE_COEF {0.25f}

// begin smazat zavislosti
 #define GAMMA (0.0f)
 #define DELTA (0.0f)
// end smazat zavislosti
#elif defined(WAVELET_TYPE_CDF97) 
 #define FILTER_LENGTH 1
 #define BORDER_SIZE 2
 #define LIFTING_STEPS_COUNT (2)
 #define ALPHA (-1.58613434342059f)
 #define BETA  (-0.0529801185729f)
 #define GAMMA (0.8829110755309f)
 #define DELTA (0.4435068520439f)
 #define ZETA1 (1.1496043988602f)
 #define ZETA2 (0.8698644516248f)
 #define PREDICT_COEF {-1.58613434342059f, 0.8829110755309f}
 #define UPDATE_COEF {-0.0529801185729f, 0.4435068520439f}
#elif defined(WAVELET_TYPE_DD137) 
 #define FILTER_LENGTH 2
 #define LIFTING_STEPS_COUNT (1)
 #define ALPHA1 (-0.5625f)
 #define ALPHA2 (0.0625f)
 #define BETA1 (-0.28125f)
 #define BETA2 (0.03125f)
 #define ZETA1 (1.0f)
 #define ZETA2 (1.0f)
 #define PREDICT_COEF {-0.5625f, 0.0625f}
 #define UPDATE_COEF {-0.28125f, 0.03125f}

// begin smazat zavislosti
 #define ALPHA (0.0f)
 #define BETA (0.0f)
 #define GAMMA (0.0f)
 #define DELTA (0.0f)
// end smazat zavislosti
#endif

// code selection enums
#define PROC_TYPE_BLAZ_NORMAL 0
#define PROC_TYPE_BLAZ_REGISTER 1
#define PROC_TYPE_LAAN 2

#define DEVICE_TYPE_GPU 0
#define DEVICE_TYPE_CPU 1

#ifndef OPTIM_THREAD
  #define OPTIM_THREAD 0
#endif

#ifndef OPTIM_WARP
 #define OPTIM_WARP WAVELET_OPTIM_WARP_NONE
#endif

#if OPTIM_WARP == WAVELET_OPTIM_WARP_NONE
 #define WARP_SIZE 1
#endif

#ifndef WARP_SIZE
  #if DEVICE_TYPE == DEVICE_TYPE_GPU
    #define WARP_SIZE 32
  #else
    #define WAPR_SIZE 1
  #endif
#endif

#ifndef DEVICE_TYPE
#define DEVICE_TYPE DEVICE_TYPE_GPU
#endif

#ifndef LOCAL_MEM_BANK_COUNT
  #define LOCAL_MEM_BANK_COUNT 32
#endif


#ifndef VERT_BLOCK_SIZE
  #define VERT_BLOCK_SIZE 256
#endif

#ifndef VERT_PAIRS_PER_THREAD
  #define VERT_PAIRS_PER_THREAD 12
#endif

#ifndef VERT_BLOCK_SIZE_X
  #define VERT_BLOCK_SIZE_X 32
#endif

#define VERT_PIXELS_PER_THREAD (VERT_PAIRS_PER_THREAD * 2)

#define VERT_BLOCK_SIZE_Y (VERT_BLOCK_SIZE / VERT_BLOCK_SIZE_X)

#define VERT_PAIRS_PER_GROUP (VERT_BLOCK_SIZE_Y * VERT_PAIRS_PER_THREAD)

#define VERT_PIXELS_PER_GROUP (VERT_PAIRS_PER_GROUP * 2)

#define VERT_LOCAL_SIZE_Y (VERT_PIXELS_PER_GROUP + 7)

#define VERT_LOCAL_SIZE (VERT_BLOCK_SIZE_X * VERT_LOCAL_SIZE_Y)

#ifndef VERT_PROC_TYPE
  #define VERT_PROC_TYPE PROC_TYPE_BLAZ_NORMAL
#endif


#ifndef HOR_BLOCK_SIZE
  #define HOR_BLOCK_SIZE 256
#endif

#ifndef HOR_PAIRS_PER_THREAD
  #define HOR_PAIRS_PER_THREAD 1
#endif

#ifndef HOR_BLOCK_SIZE_Y
  #define HOR_BLOCK_SIZE_Y 1
#endif

#define HOR_PIXELS_PER_THREAD (HOR_PAIRS_PER_THREAD * 2)

#define HOR_BLOCK_SIZE_X (HOR_BLOCK_SIZE / HOR_BLOCK_SIZE_Y)

#define HOR_PAIRS_PER_GROUP (HOR_PAIRS_PER_THREAD * HOR_BLOCK_SIZE_X)

#define HOR_PIXELS_PER_GROUP (HOR_PAIRS_PER_GROUP * 2)

#define HOR_LOCAL_SIZE_X (HOR_PIXELS_PER_GROUP + 7)

#define HOR_LOCAL_SIZE (HOR_BLOCK_SIZE_Y * HOR_LOCAL_SIZE_X)

#ifndef HOR_PROC_TYPE
  #define HOR_PROC_TYPE PROC_TYPE_BLAZ_NORMAL
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if OPTIM_WARP == WAVELET_OPTIM_WARP_SHUFFLE
#if defined(cl_nv_pragma_unroll)
inline int __shfl_i(int var, int srcLane, int mask)
  {
  int out_value;
  asm("shfl.sync.idx.b32 %0, %1, %2, %3, 0xffffffff;" : "=r"(out_value) : "r"(var), "r"(srcLane), "r"(mask));
  return out_value;
  }

inline int __shfl_up_i(int var, unsigned int delta, int mask)
  {
  int out_value;
  asm("shfl.sync.up.b32 %0, %1, %2, %3, 0xffffffff;" : "=r"(out_value) : "r"(var), "r"(delta), "r"(mask));
  return out_value;
  }

inline int __shfl_down_i(int var, unsigned int delta, int mask)
  {
  int out_value;
  asm("shfl.sync.down.b32 %0, %1, %2, %3, 0xffffffff;" : "=r"(out_value) : "r"(var), "r"(delta), "r"(mask));
  return out_value;
  }

inline int __shfl_xor_i(int var, int laneMask, int mask)
  {
  int out_value;
  asm("shfl.sync.bfly.b32 %0, %1, %2, %3, 0xffffffff;" : "=r"(out_value) : "r"(var), "r"(laneMask), "r"(mask));
  return out_value;
  }

inline float __shfl_f(float var, int srcLane, int mask)
  {
  float out_value;
  asm("shfl.sync.idx.b32 %0, %1, %2, %3, 0xffffffff;" : "=f"(out_value) : "f"(var), "r"(srcLane), "r"(mask));
  return out_value;
  }

inline float __shfl_up_f(float var, unsigned int delta, int mask)
  {
  float out_value;
  asm("shfl.sync.up.b32 %0, %1, %2, %3, 0xffffffff;" : "=f"(out_value) : "f"(var), "r"(delta), "r"(mask));
  return out_value;
  }

inline float __shfl_down_f(float var, unsigned int delta, int mask)
  {
  float out_value;
  asm("shfl.sync.down.b32 %0, %1, %2, %3, 0xffffffff;" : "=f"(out_value) : "f"(var), "r"(delta), "r"(mask));
  return out_value;
  }

inline float __shfl_xor_f(float var, int laneMask, int mask)
  {
  float out_value;
  asm("shfl.sync.bfly.b32 %0, %1, %2, %3, 0xffffffff;" : "=f"(out_value) : "f"(var), "r"(laneMask), "r"(mask));
  return out_value;
  }
#elif defined(cl_intel_subgroups)
inline int __shfl_up_i(unsigned int var, unsigned int delta, int mask)
{
	return intel_sub_group_shuffle_up(var, var, delta);
}
inline int __shfl_down_i(unsigned int var, unsigned int delta, int mask)
{
	return intel_sub_group_shuffle_down(var, var, delta);
}

inline int __shfl_up_f(float var, unsigned int delta, int mask)
{
	return intel_sub_group_shuffle_up(var, var, delta);
}
inline int __shfl_down_f(float var, unsigned int delta, int mask)
{
	return intel_sub_group_shuffle_down(var, var, delta);
}
#elif __has_builtin(__builtin_amdgcn_mov_dpp)
inline int __shfl_up_1(unsigned int var, unsigned int delta, int mask)
{
	return __builtin_amdgcn_mov_dpp(var, 0x138, 0xF, 0xF, 1);
}
inline int __shfl_down_1(unsigned int var, unsigned int delta, int mask)
{
	return __builtin_amdgcn_mov_dpp(var, 0x130, 0xF, 0xF, 1);
}

/*inline int __shfl_up_i(unsigned int var, unsigned int delta, int mask)
{
	unsigned int flag = 0x110 + delta;
	return __builtin_amdgcn_mov_dpp(var, flag, 0xF, 0xF, 1);
}
inline int __shfl_down_i(unsigned int var, unsigned int delta, int mask)
{
	unsigned int flag = 0x100 + delta;
	return __builtin_amdgcn_mov_dpp(var, flag, 0xF, 0xF, 1);
}

inline int __shfl_up_f(float var, unsigned int delta, int mask)
{
	unsigned int flag = 0x110 + delta;
	return __builtin_amdgcn_mov_dpp(var, flag, 0xF, 0xF, 1);
}
inline int __shfl_down_f(float var, unsigned int delta, int mask)
{
	unsigned int flag = 0x100 + delta;
	return __builtin_amdgcn_mov_dpp(var, flag, 0xF, 0xF, 1);
}*/
#define __shfl_up_i(var, delta, mask) __builtin_amdgcn_mov_dpp(var, 0x110 + delta, 0xF, 0xF, 1)
#define __shfl_down_i(var, delta, mask) __builtin_amdgcn_mov_dpp(var, 0x100 + delta, 0xF, 0xF, 1)
#define __shfl_up_f(var, delta, mask) __builtin_amdgcn_mov_dpp(var, 0x110 + delta, 0xF, 0xF, 1)
#define __shfl_down_f(var, delta, mask) __builtin_amdgcn_mov_dpp(var, 0x100 + delta, 0xF, 0xF, 1)
#endif
#endif



#if DEVICE_TYPE == DEVICE_TYPE_GPU
  #define HOR_LOCAL_SIZE_ALIGN (HOR_LOCAL_SIZE + (HOR_LOCAL_SIZE / LOCAL_MEM_BANK_COUNT))
  size_t change_offset(size_t offset)
    {
      return offset + (offset >> 5);
    }
#else
  #define HOR_LOCAL_SIZE_ALIGN HOR_LOCAL_SIZE
  inline size_t change_offset(size_t offset)
    {
      return offset; 
    }
#endif

#if (WARP_SIZE >= HOR_BLOCK_SIZE_X) && ((WARP_SIZE % HOR_BLOCK_SIZE_X) == 0)
#define HOR_ATOM 1
#else
#define HOR_ATOM 0
#endif

void load_vert_t(__global float *in_image, __local float *temp_image, size_t ly, uint stride)
{
  float act_val = in_image[0];
  temp_image[4 * VERT_BLOCK_SIZE_X] = act_val;
  if((ly < 5) && (ly != 0))
    {
      temp_image[(4 - 2 * ly) * VERT_BLOCK_SIZE_X] = act_val;
    }
  for(int i = 1; i < VERT_PIXELS_PER_THREAD; i++)
    {
      temp_image[(4 + i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
  if(ly < 3)
    {
      temp_image[(4 + VERT_PIXELS_PER_GROUP) * VERT_BLOCK_SIZE_X] = in_image[VERT_PIXELS_PER_GROUP * stride];
    }
}

void load_vert_m(__global float *in_image, __local float *temp_image, size_t ly, uint stride)
{
  // copy previous data
  if(ly < 7)
    {
      temp_image[0] = temp_image[(VERT_LOCAL_SIZE_Y - 7) * VERT_BLOCK_SIZE_X];
    }
  // get next data
  for(int i = 0; i < VERT_PIXELS_PER_THREAD; i++)
    {
      temp_image[(7 + i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
}

void load_vert_b(__global float *in_image, __local float *temp_image, size_t ly, uint stride)
{
  float act_val;
  // copy previous data
  if(ly < 7)
    {
      temp_image[0] = temp_image[(VERT_LOCAL_SIZE_Y - 7) * VERT_BLOCK_SIZE_X];
    }
  // get next data
  for(int i = 0; i < VERT_PIXELS_PER_THREAD - 1; i++)
    {
      temp_image[(7 + i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
  if(ly < VERT_BLOCK_SIZE_Y - 3)
    {
      act_val = in_image[(VERT_PIXELS_PER_GROUP - VERT_BLOCK_SIZE_Y) * stride];
      temp_image[(7 + VERT_PIXELS_PER_GROUP - VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = act_val;
    }

  if((ly < VERT_BLOCK_SIZE_Y - 4) && (ly >= VERT_BLOCK_SIZE_Y - 7))
    {
      temp_image[(4 + VERT_PIXELS_PER_GROUP + VERT_BLOCK_SIZE_Y - 5 - 2 * ly) * VERT_BLOCK_SIZE_X] = act_val;
    }
}

void load_vert_m_split(__global float *in_image, __local float *temp_image, size_t ly, uint stride)
{
  // get next data
  for(int i = 0; i < VERT_PIXELS_PER_THREAD; i++)
    {
      temp_image[(i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
  // copy previous data
  if(ly < 7)
    {
      temp_image[VERT_PIXELS_PER_GROUP * VERT_BLOCK_SIZE_X] = in_image[VERT_PIXELS_PER_GROUP * stride];
    }
}

void load_vert_b_split(__global float *in_image, __local float *temp_image, size_t ly, uint stride)
{
  float act_val;
  for(int i = 0; i < VERT_PIXELS_PER_THREAD; i++)
    {
	  temp_image[(i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
	}
  if(ly < 4)
	{
	  act_val = in_image[VERT_PIXELS_PER_GROUP * stride];
	  temp_image[VERT_PIXELS_PER_GROUP * VERT_BLOCK_SIZE_X] = act_val;
	}
  if(ly < 3)
	{
	  temp_image[(6 + VERT_PIXELS_PER_GROUP - ly * 2) * VERT_BLOCK_SIZE_X] = act_val;
	}
}

void load_vert_b_doublebuffer(__global float *in_image, __local float *temp_image, __local float *temp_image_old, size_t ly, uint stride)
{
  float act_val;
  // copy previous data
  if(ly < 7)
    {
      temp_image[0] = temp_image_old[(VERT_LOCAL_SIZE_Y - 7) * VERT_BLOCK_SIZE_X];
    }
  // get next data
  for(int i = 0; i < VERT_PIXELS_PER_THREAD - 1; i++)
    {
      temp_image[(7 + i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
  if(ly < VERT_BLOCK_SIZE_Y - 3)
    {
      act_val = in_image[(VERT_PIXELS_PER_GROUP - VERT_BLOCK_SIZE_Y) * stride];
      temp_image[(7 + VERT_PIXELS_PER_GROUP - VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = act_val;
    }

  if((ly < VERT_BLOCK_SIZE_Y - 4) && (ly >= VERT_BLOCK_SIZE_Y - 7))
    {
      temp_image[(4 + VERT_PIXELS_PER_GROUP + VERT_BLOCK_SIZE_Y - 5 - 2 * ly) * VERT_BLOCK_SIZE_X] = act_val;
    }
}

void load_vert_m_doublebuffer(__global float *in_image, __local float *temp_image, __local float *temp_image_old, size_t ly, uint stride)
{
  // copy previous data
  if(ly < 7)
    {
      temp_image[0] = temp_image_old[(VERT_LOCAL_SIZE_Y - 7) * VERT_BLOCK_SIZE_X];
    }
  // get next data
  for(int i = 0; i < VERT_PIXELS_PER_THREAD; i++)
    {
      temp_image[(7 + i * VERT_BLOCK_SIZE_Y) * VERT_BLOCK_SIZE_X] = in_image[(i * VERT_BLOCK_SIZE_Y) * stride];
    }
}


#if VERT_PROC_TYPE == PROC_TYPE_BLAZ_NORMAL
void proc_vert_blazewicz(__local float *local_image, __global float *out_image_l, __global float *out_image_h, uint stride)
{
  // create private memory
  __private float private_image[VERT_PIXELS_PER_THREAD + 5];
  for (int i = 0; i < VERT_PIXELS_PER_THREAD + 5; i++)
    {
      private_image[i] = local_image[(i + 1) * VERT_BLOCK_SIZE_X];
    }
  // compute wavelet transform
  private_image[0] += ALPHA * (private_image[1] + local_image[0]);
  private_image[VERT_PIXELS_PER_THREAD + 4] += ALPHA * (private_image[VERT_PIXELS_PER_THREAD + 3] + local_image[(VERT_PIXELS_PER_THREAD + 6) * VERT_BLOCK_SIZE_X]);
  for(int i = 1; i < VERT_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2] += ALPHA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 0; i < VERT_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2 + 1] += BETA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  for(int i = 1; i < VERT_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2] += GAMMA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 1; i < VERT_PAIRS_PER_THREAD + 1; i++)
    {
      private_image[i * 2 + 1] += DELTA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  // save output coefficients
  for(int i = 0; i < VERT_PAIRS_PER_THREAD; i++)
    {
      out_image_l[i * stride] = ZETA1 * private_image[i * 2 + 3];
      out_image_h[i * stride] = ZETA2 * private_image[i * 2 + 4];
    }
}
#else
void proc_vert_blazewicz(__local float *local_image, __global float *out_image_l, __global float *out_image_h, uint stride)
{
  // create private memory
  __private float private_image[5];
  float tmp_image_0, tmp_image_1, tmp_image_2;
  
  private_image[0] = local_image[0];

  tmp_image_0 = private_image[0];
  private_image[0] = local_image[2 * VERT_BLOCK_SIZE_X];
  private_image[1] = local_image[1 * VERT_BLOCK_SIZE_X] + ALPHA * (tmp_image_0 + private_image[0]);

  tmp_image_0 = private_image[0];
  tmp_image_1 = private_image[1];
  private_image[0] = local_image[4 * VERT_BLOCK_SIZE_X];
  private_image[1] = local_image[3 * VERT_BLOCK_SIZE_X] + ALPHA * (tmp_image_0 + private_image[0]);
  private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);
  
  tmp_image_0 = private_image[0];
  tmp_image_1 = private_image[1];
  tmp_image_2 = private_image[2];
  private_image[0] = local_image[6 * VERT_BLOCK_SIZE_X];
  private_image[1] = local_image[5 * VERT_BLOCK_SIZE_X] + ALPHA * (tmp_image_0 + private_image[0]);
  private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);
  private_image[3] = tmp_image_1 + GAMMA * (tmp_image_2 + private_image[2]);

  for(int i = 0; i < VERT_PAIRS_PER_THREAD; i++)
    {
      tmp_image_0 = private_image[0];
      tmp_image_1 = private_image[1];
      tmp_image_2 = private_image[2];
      private_image[0] = local_image[(8 + i * 2) * VERT_BLOCK_SIZE_X];
      private_image[1] = local_image[(7 + i * 2) * VERT_BLOCK_SIZE_X] + ALPHA * (tmp_image_0 + private_image[0]);
      private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);
      tmp_image_0 = private_image[3];
      private_image[3] = tmp_image_1 + GAMMA * (tmp_image_2 + private_image[2]);
      private_image[4] = tmp_image_2 + DELTA * (tmp_image_0 + private_image[3]);
      out_image_l[i * stride] = ZETA1 * private_image[4];
      out_image_h[i * stride] = ZETA2 * private_image[3];
    }
}
#endif
// requirements:
//  width align to VERT_BLOCK_SIZE_X
//  height align to VERT_BLOCK_SIZE_Y * PAIR_PER_THREAD * 2
//  minimum height 2 * VERT_BLOCK_SIZE_Y * PAIR_PER_THREAD * 2
#ifdef wavelet_vert_basic
__kernel void wavelet_vert(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);

  #ifndef WAVELET_OUTPUT_INTERLACED
	  size_t gx = get_global_id(0);
	  size_t gy = get_global_id(1);
    int gy_l = gy * VERT_PAIRS_PER_THREAD;
    int gy_l_to_h_stride = height>>(1+depth);
    int gy_out_stride = 1;
    int gy_block_stride = VERT_PAIRS_PER_GROUP;
	  int gy_in_stride = 1;
  #else
	  size_t gx = get_global_id(0) << depth;
	  size_t gy = get_global_id(1) << depth;
    int gy_l = gy * VERT_PIXELS_PER_THREAD;
    int gy_l_to_h_stride = 1 << depth;
    int gy_out_stride = 1 << (1 + depth);
    int gy_block_stride = VERT_PIXELS_PER_GROUP << depth;
	  int gy_in_stride = 1 << depth;
  #endif

  int out_stride = gy_out_stride * width;
  int l_to_h_stride = gy_l_to_h_stride * width;
  int block_stride = gy_block_stride * width;
  int in_stride = gy_in_stride * width;

  __local float temp_image[VERT_LOCAL_SIZE];

  __global float *g_in_image = in_image + gx + gy * width;
  __global float *g_out_image_l = out_image + gx + gy_l * width;
  __global float *g_out_image_h = g_out_image_l + l_to_h_stride;
  __local float *l_load_image_pos = temp_image + lx + ly * VERT_BLOCK_SIZE_X;
  __local float *l_proc_image_pos = temp_image + lx + ly * VERT_PIXELS_PER_THREAD * VERT_BLOCK_SIZE_X;
  
  load_vert_t(g_in_image, l_load_image_pos, ly, in_stride);
  barrier(CLK_LOCAL_MEM_FENCE);
  proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
  g_in_image += (VERT_PIXELS_PER_GROUP + 3) * in_stride;
  g_out_image_l += block_stride;
  g_out_image_h += block_stride;
  for(int i = 1; i < ((height>>depth) / VERT_PIXELS_PER_GROUP) - 1; i++)
    {
      barrier(CLK_LOCAL_MEM_FENCE);
      load_vert_m(g_in_image, l_load_image_pos, ly, in_stride);
      barrier(CLK_LOCAL_MEM_FENCE);
      proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
      g_in_image += VERT_PIXELS_PER_GROUP * in_stride;
      g_out_image_l += block_stride;
	  g_out_image_h += block_stride;
    }
  barrier(CLK_LOCAL_MEM_FENCE);
  load_vert_b(g_in_image, l_load_image_pos, ly, in_stride);
  barrier(CLK_LOCAL_MEM_FENCE);
  proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
}
#endif


#ifdef wavelet_vert_split
__kernel void wavelet_vert(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);
  size_t gry = get_group_id(1);
	
  #ifndef WAVELET_OUTPUT_INTERLACED
	  size_t gx = get_global_id(0);
	  size_t gy = get_global_id(1);
    int gy_l = gy * VERT_PAIRS_PER_THREAD;
    int gy_l_to_h_stride = height>>(1+depth);
    int gy_out_stride = 1;
	  int gy_in_stride = 1;
  #else
	  size_t gx = get_global_id(0) << depth;
	  size_t gy = get_global_id(1) << depth;
    int gy_l = gy * VERT_PIXELS_PER_THREAD;
    int gy_l_to_h_stride = 1 << depth;
    int gy_out_stride = 1 << (1 + depth);
	int gy_in_stride = 1 << depth;
  #endif

  int out_stride = gy_out_stride * width;
  int l_to_h_stride = gy_l_to_h_stride * width;
  int in_stride = gy_in_stride * width;

  __local float temp_image[VERT_LOCAL_SIZE];

  __global float *g_in_image = in_image + gx + ly * in_stride;
  __global float *g_out_image_l = out_image + gx + gy_l * width;
  __global float *g_out_image_h = g_out_image_l + l_to_h_stride;
  __local float *l_load_image_pos = temp_image + lx + ly * VERT_BLOCK_SIZE_X;
  __local float *l_proc_image_pos = temp_image + lx + ly * VERT_PIXELS_PER_THREAD * VERT_BLOCK_SIZE_X;
  if(gry == 0)
    {
      load_vert_t(g_in_image, l_load_image_pos, ly, in_stride);
	    barrier(CLK_LOCAL_MEM_FENCE);
	    proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
    }
  else if(gry != get_num_groups(1) - 1)
    {
      g_in_image += (VERT_PIXELS_PER_GROUP * gry - 4) * in_stride;
      load_vert_m_split(g_in_image, l_load_image_pos, ly, in_stride);
      barrier(CLK_LOCAL_MEM_FENCE);
      proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
    }
  else
    {
      g_in_image += (VERT_PIXELS_PER_GROUP * gry - 4) * in_stride;
      load_vert_b_split(g_in_image, l_load_image_pos, ly, in_stride);
      barrier(CLK_LOCAL_MEM_FENCE);
      proc_vert_blazewicz(l_proc_image_pos, g_out_image_l, g_out_image_h, out_stride);
    }
}
#endif


#ifdef wavelet_vert_line
__kernel void wavelet_vert(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{ 
  #ifndef WAVELET_OUTPUT_INTERLACED
	  size_t gx = get_global_id(0);
	  size_t gy = get_global_id(1);
    size_t gy_l = gy * VERT_PAIRS_PER_THREAD;
    int gy_l_to_h = height>>(1+depth);
    int gy_out_stride = 1;
	  int gy_in_stride = 1;
  #else
	  size_t gx = get_global_id(0) << depth;
	  size_t gy = get_global_id(1) << depth;
    size_t gy_l = gy * VERT_PIXELS_PER_THREAD;
    int gy_l_to_h = 1 << depth;
    int gy_out_stride = 1 << (1 + depth);
	  int gy_in_stride = 1 << depth;
  #endif

  size_t gy_in = gy * VERT_PIXELS_PER_THREAD;

  int out_stride = gy_out_stride * width;
  int in_stride = gy_in_stride * width;

  __global float *g_in_image = in_image + gx + gy_in * width;
  __global float *g_out_image_l = out_image + gx + gy_l * width;
  __global float *g_out_image_h = g_out_image_l + gy_l_to_h * width;

  float a,b,c,d,e,tmp1;
  if(gy == 0)
    {
      e = *g_in_image;
      g_in_image += in_stride;
      d = *g_in_image;
      g_in_image += in_stride;
      c = *g_in_image;
      g_in_image += in_stride;

      d += ALPHA * (e + c);
      e += BETA * 2 * d;

      b = *g_in_image;
      g_in_image += in_stride;
      a = *g_in_image;
      g_in_image += in_stride;

      
      b += ALPHA * (c + a);
      c += BETA * (d + b);
      d += GAMMA * (e + c);
      e += DELTA * 2 * d;
    }
  else
    {
      g_in_image -= in_stride * 4;
      
      e = *g_in_image;
      g_in_image += in_stride;
      tmp1 = *g_in_image;
      g_in_image += in_stride;
      d = *g_in_image;
      g_in_image += in_stride;

      e = tmp1 + ALPHA * (e + d);
      
      tmp1 = *g_in_image;
      g_in_image += in_stride;
      c = *g_in_image;
      g_in_image += in_stride;
      
      e = d + BETA * e;
      d = tmp1 + ALPHA * (d + c);
      e += BETA * d;

      tmp1 = *g_in_image;
      g_in_image += in_stride;
      b = *g_in_image;
      g_in_image += in_stride;

      e = d + GAMMA * e;
      d = c + BETA * d;
      c = tmp1 + ALPHA * (c + b);
      d += BETA * c;
      e += GAMMA * d;

      tmp1 = *g_in_image;
      g_in_image += in_stride;
      a = *g_in_image;
      g_in_image += in_stride;

      e = d + DELTA * e;
      d = c + GAMMA * d;
      c = b + BETA * c;
      b = tmp1 + ALPHA * (b + a);
      c += BETA * b;
      d += GAMMA * c;
      e += DELTA * d;
    }
  *g_out_image_l = ZETA1 * e;
  *g_out_image_h = ZETA2 * d;
  g_out_image_l += out_stride;
  g_out_image_h += out_stride;

  for(int i = 1; i < VERT_PAIRS_PER_THREAD - 2; i++)
    {
      tmp1 = *g_in_image;
      g_in_image += in_stride;

      e = DELTA * d + c;
      d = GAMMA * c + b;
      c = BETA * b + a;
      b = ALPHA * a + tmp1;

      a = *g_in_image;
      g_in_image += in_stride;

      b += ALPHA * a;
      c += BETA * b;
      d += GAMMA * c;
      e += DELTA * d;

      *g_out_image_l = ZETA1 * e;
      *g_out_image_h = ZETA2 * d;
      g_out_image_l += out_stride;
      g_out_image_h += out_stride;
    }
  
  if(gy == get_global_size(1) - 1)
    {
      tmp1 = *g_in_image;
      g_in_image += in_stride;

      e = DELTA * d + c;
      d = GAMMA * c + b;
      c = BETA * b + a;
      b = ALPHA * 2 * a + tmp1;
      c += BETA * b;
      d += GAMMA * c;
      e += DELTA * d;

      *g_out_image_l = ZETA1 * e;
      *g_out_image_h = ZETA2 * d;
      g_out_image_l += out_stride;
      g_out_image_h += out_stride;

      e = DELTA * d + c;
      d = GAMMA * 2 * c + b;
      e += DELTA * d;

      *g_out_image_l = ZETA1 * e;
      *g_out_image_h = ZETA2 * d;
    }
  else
    {
      tmp1 = *g_in_image;
      g_in_image += in_stride;

      e = DELTA * d + c;
      d = GAMMA * c + b;
      c = BETA * b + a;
      b = ALPHA * a + tmp1;

      a = *g_in_image;
      g_in_image += in_stride;

      b += ALPHA * a;
      c += BETA * b;
      d += GAMMA * c;
      e += DELTA * d;

      *g_out_image_l = ZETA1 * e;
      *g_out_image_h = ZETA2 * d;
      g_out_image_l += out_stride;
      g_out_image_h += out_stride;

      tmp1 = *g_in_image;
      g_in_image += in_stride;

      e = DELTA * d + c;
      d = GAMMA * c + b;
      c = BETA * b + a;
      b = ALPHA * a + tmp1;

      a = *g_in_image;
      g_in_image += in_stride;

      b += ALPHA * a;
      c += BETA * b;
      d += GAMMA * c;
      e += DELTA * d;

      *g_out_image_l = ZETA1 * e;
      *g_out_image_h = ZETA2 * d;
    }
}
#endif

void load_hor_l(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  float act_val = in_image[0];
  if((lx < 5) && (lx != 0))
    {
      temp_image[change_offset(l_in_image_pos + 4 - 2 * lx)] = act_val;
    }
  temp_image[change_offset(l_in_image_pos + 4)] = act_val;
  for(int i = 1; i < HOR_PIXELS_PER_THREAD; i++)
    {
      temp_image[change_offset(l_in_image_pos + 4 + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
  if(lx < 3)
    {
      temp_image[change_offset(l_in_image_pos + 4 + HOR_PIXELS_PER_GROUP)] = in_image[HOR_PIXELS_PER_GROUP * in_stride];
    }
}

void load_hor_lr(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  float act_val = in_image[0];
  if((lx < 5) && (lx != 0))
    {
      temp_image[change_offset(l_in_image_pos + 4 - 2 * lx)] = act_val;
    }
  temp_image[change_offset(l_in_image_pos + 4)] = act_val;
  for(int i = 1; i < HOR_PIXELS_PER_THREAD - 1; i++)
    {
      temp_image[change_offset(l_in_image_pos + 4 + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
  act_val = in_image[(HOR_PIXELS_PER_GROUP - HOR_BLOCK_SIZE_X) * in_stride];
  temp_image[change_offset(l_in_image_pos + 4 + HOR_PIXELS_PER_GROUP - HOR_BLOCK_SIZE_X)] = act_val;
  if((lx >= HOR_BLOCK_SIZE_X - 4) && (lx != HOR_BLOCK_SIZE_X - 1))
    {
      temp_image[change_offset(l_in_image_pos + 2 + HOR_PIXELS_PER_GROUP + HOR_BLOCK_SIZE_X - 2 * lx)] = act_val;
    }
}

void load_hor_m(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  // copy previous data
  if(lx < 7)
    {
      temp_image[change_offset(l_in_image_pos)] = temp_image[change_offset(l_in_image_pos + HOR_LOCAL_SIZE_X - 7)];
    }
  // get next data
  for(int i = 0; i < HOR_PIXELS_PER_THREAD; i++)
    {
      temp_image[change_offset(l_in_image_pos + 7 + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
}

void load_hor_r(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  float act_val;
  if(lx < 7)
    {
      temp_image[change_offset(l_in_image_pos)] = temp_image[change_offset(l_in_image_pos + HOR_LOCAL_SIZE_X - 7)];
    }
  for(int i = 0; i < HOR_PIXELS_PER_THREAD - 1; i++)
    {
      temp_image[change_offset(l_in_image_pos + 7 + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
  if(lx < HOR_BLOCK_SIZE_X - 3)
    {
      act_val = in_image[(HOR_PIXELS_PER_THREAD - 1) * HOR_BLOCK_SIZE_X * in_stride];
      temp_image[change_offset(l_in_image_pos + 7 + (HOR_PIXELS_PER_THREAD - 1) * HOR_BLOCK_SIZE_X)] = act_val;
    }
  if((lx < HOR_BLOCK_SIZE_X - 4) && (lx >= HOR_BLOCK_SIZE_X - 7))
    {
      temp_image[change_offset(l_in_image_pos + 4 + HOR_PIXELS_PER_GROUP + HOR_BLOCK_SIZE_X - 5 - 2 * lx)] = act_val;
    }
}

void load_hor_m_split(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  for(int i = 0; i < HOR_PIXELS_PER_THREAD; i++)
    {
      temp_image[change_offset(l_in_image_pos + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
  // copy previous data
  if(lx < 7)
    {
      temp_image[change_offset(l_in_image_pos + HOR_PIXELS_PER_GROUP)] = in_image[HOR_PIXELS_PER_GROUP * in_stride];
    }
}

void load_hor_r_split(__global float *in_image, __local float *temp_image, size_t lx, size_t l_in_image_pos, int in_stride)
{
  float act_val;
  for(int i = 0; i < HOR_PIXELS_PER_THREAD; i++)
    {
      temp_image[change_offset(l_in_image_pos + i * HOR_BLOCK_SIZE_X)] = in_image[i * HOR_BLOCK_SIZE_X * in_stride];
    }
  if(lx < 4)
    {
      act_val = in_image[HOR_PIXELS_PER_GROUP * in_stride];
      temp_image[change_offset(l_in_image_pos + HOR_PIXELS_PER_GROUP)] = act_val;
    }
  if(lx < 3)
	{
      temp_image[change_offset(l_in_image_pos + 6 + HOR_PIXELS_PER_GROUP - 2 * lx)] = act_val;
	}
}

#if HOR_PROC_TYPE == PROC_TYPE_BLAZ_NORMAL
void proc_hor(__local float *l_in_image, __global float *out_image_l, __global float *out_image_h, size_t l_in_image_pos, size_t out_stride)
{
  // create private memory
  __private float private_image[HOR_PIXELS_PER_THREAD + 5];
  for(int i = 0; i < HOR_PIXELS_PER_THREAD + 5; i++)
    {
    private_image[i] = l_in_image[change_offset(l_in_image_pos + i + 1)];
    }
  // compute wavelet transform
  private_image[0] += ALPHA * (private_image[1] + l_in_image[change_offset(l_in_image_pos)]);
  private_image[HOR_PIXELS_PER_THREAD + 4] += ALPHA * (private_image[HOR_PIXELS_PER_THREAD + 3] + l_in_image[change_offset(l_in_image_pos + HOR_PIXELS_PER_THREAD + 6)]);
  for(int i = 1; i < HOR_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2] += ALPHA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 0; i < HOR_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2 + 1] += BETA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  for(int i = 1; i < HOR_PAIRS_PER_THREAD + 2; i++)
    {
      private_image[i * 2] += GAMMA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 1; i < HOR_PAIRS_PER_THREAD + 1; i++)
    {
      private_image[i * 2 + 1] += DELTA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  // save output coefficients
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      out_image_l[i * out_stride] = ZETA1 * private_image[i * 2 + 3];
      out_image_h[i * out_stride] = ZETA2 * private_image[i * 2 + 4];
    }
}
#elif HOR_PROC_TYPE == PROC_TYPE_BLAZ_REGISTER
void proc_hor(__local float *l_in_image, __global float *out_image_l, __global float *out_image_h, size_t l_in_image_pos, size_t out_stride)
{
  // create private memory
  __private float private_image[5];
  float tmp_image_0, tmp_image_1, tmp_image_2;

  private_image[0] = l_in_image[change_offset(l_in_image_pos)];

  tmp_image_0 = private_image[0];
  private_image[0] = l_in_image[change_offset(l_in_image_pos + 2)];
  private_image[1] = l_in_image[change_offset(l_in_image_pos + 1)] + ALPHA * (tmp_image_0 + private_image[0]);

  tmp_image_0 = private_image[0];
  tmp_image_1 = private_image[1];
  private_image[0] = l_in_image[change_offset(l_in_image_pos + 4)];
  private_image[1] = l_in_image[change_offset(l_in_image_pos + 3)] + ALPHA * (tmp_image_0 + private_image[0]);
  private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);

  tmp_image_0 = private_image[0];
  tmp_image_1 = private_image[1];
  tmp_image_2 = private_image[2];
  private_image[0] = l_in_image[change_offset(l_in_image_pos + 6)];
  private_image[1] = l_in_image[change_offset(l_in_image_pos + 5)] + ALPHA * (tmp_image_0 + private_image[0]);
  private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);
  private_image[3] = tmp_image_1 + GAMMA * (tmp_image_2 + private_image[2]);

  for (int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
  {
    tmp_image_0 = private_image[0];
    tmp_image_1 = private_image[1];
    tmp_image_2 = private_image[2];
    private_image[0] = l_in_image[change_offset(l_in_image_pos + 7 + i * 2)];
    private_image[1] = l_in_image[change_offset(l_in_image_pos + 8 + i * 2)] + ALPHA * (tmp_image_0 + private_image[0]);
    private_image[2] = tmp_image_0 + BETA * (tmp_image_1 + private_image[1]);
    tmp_image_0 = private_image[3];
    private_image[3] = tmp_image_1 + GAMMA * (tmp_image_2 + private_image[2]);
    private_image[4] = tmp_image_2 + DELTA * (tmp_image_0 + private_image[3]);
    out_image_l[i * out_stride] = ZETA1 * private_image[4];
    out_image_h[i * out_stride] = ZETA2 * private_image[3];
  }
}
#else
void proc_hor_start(__local float *l_in_image, size_t l_in_image_pos, size_t lx)
{
  if(lx < 3) l_in_image[change_offset(l_in_image_pos + 1)] += ALPHA * (l_in_image[change_offset(l_in_image_pos + 2)] + l_in_image[change_offset(l_in_image_pos)]);
  #if WARP_SIZE < 3
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  
  if(lx < 2) l_in_image[change_offset(l_in_image_pos + 2)] += BETA * (l_in_image[change_offset(l_in_image_pos + 3)] + l_in_image[change_offset(l_in_image_pos + 1)]);
  #if WARP_SIZE < 2
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  
  if(lx < 1) l_in_image[change_offset(l_in_image_pos + 3)] += GAMMA * (l_in_image[change_offset(l_in_image_pos + 4)] + l_in_image[change_offset(l_in_image_pos + 2)]);
}

void proc_hor(__local float *l_in_image, __global float *out_image_l, __global float *out_image_h, size_t l_in_image_pos, size_t out_stride)
{
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      l_in_image[change_offset(l_in_image_pos + i * 2 + 7)] += ALPHA * (l_in_image[change_offset(l_in_image_pos + i * 2 + 8)] + l_in_image[change_offset(l_in_image_pos + i * 2 + 6)]);
    }
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      l_in_image[change_offset(l_in_image_pos + i * 2 + 6)] += BETA * (l_in_image[change_offset(l_in_image_pos + i * 2 + 7)] + l_in_image[change_offset(l_in_image_pos + i * 2 + 5)]);
    }
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      l_in_image[change_offset(l_in_image_pos + i * 2 + 5)] += GAMMA * (l_in_image[change_offset(l_in_image_pos + i * 2 + 6)] + l_in_image[change_offset(l_in_image_pos + i * 2 + 4)]);
    }
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      l_in_image[change_offset(l_in_image_pos + i * 2 + 4)] += DELTA * (l_in_image[change_offset(l_in_image_pos + i * 2 + 5)] + l_in_image[change_offset(l_in_image_pos + i * 2 + 3)]);
    }
  // save output coefficients
  for(int i = 0; i < HOR_PAIRS_PER_THREAD; i++)
    {
      out_image_l[i * out_stride] = ZETA1 * l_in_image[change_offset(l_in_image_pos + i * 2 + 4)];
      out_image_h[i * out_stride] = ZETA2 * l_in_image[change_offset(l_in_image_pos + i * 2 + 5)];
    }
}
#endif


#ifdef wavelet_hor_basic
__kernel void wavelet_hor(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);
	

  #ifndef WAVELET_OUTPUT_INTERLACED
	  size_t gx = get_global_id(0);
	  size_t gy = get_global_id(1);
    int gx_l = gx * HOR_PAIRS_PER_THREAD;
    int l_to_h_stride = width>>(1+depth);
    int out_stride = 1;
    int block_stride = HOR_PAIRS_PER_GROUP;
	  int in_stride = 1;
  #else
	  size_t gx = get_global_id(0) << depth;
	  size_t gy = get_global_id(1) << depth;
    int gx_l = gx * HOR_PIXELS_PER_THREAD;
    int l_to_h_stride = 1 << depth;
    int out_stride = 1 << (1 + depth);
    int block_stride = HOR_PIXELS_PER_GROUP << depth;
	  int in_stride = 1 << depth;
  #endif

  __local float temp_image[HOR_LOCAL_SIZE_ALIGN];

  __global float *g_in_image = in_image + lx * in_stride + gy * width;
  __global float *g_out_image_l = out_image + gx_l + gy * width;
  __global float *g_out_image_h = g_out_image_l + l_to_h_stride;
  size_t l_load_image_pos = lx + ly * HOR_LOCAL_SIZE_X;
  size_t l_proc_image_pos = lx * HOR_PIXELS_PER_THREAD + ly * HOR_LOCAL_SIZE_X;
  if(((width >> depth) / HOR_PIXELS_PER_GROUP) == 1)
	{
	  load_hor_lr(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
    #if HOR_ATOM == 0
	  barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    #if HOR_PROC_TYPE == PROC_TYPE_LAAN
      proc_hor_start(temp_image, l_proc_image_pos, lx);
    #endif
	  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
	  return;
	}
  load_hor_l(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  #if HOR_PROC_TYPE == PROC_TYPE_LAAN
    proc_hor_start(temp_image, l_proc_image_pos, lx);
  #endif
  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
  g_in_image += (HOR_PIXELS_PER_GROUP + 3) * in_stride;
  g_out_image_l += block_stride;
  g_out_image_h += block_stride;
  for(int i = 1; i < ((width >> depth) / HOR_PIXELS_PER_GROUP) - 1; i++)
  {
    #if HOR_ATOM == 0
    barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    load_hor_m(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
    #if HOR_ATOM == 0
    barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
    g_in_image += HOR_PIXELS_PER_GROUP * in_stride;
    g_out_image_l += block_stride;
    g_out_image_h += block_stride;
  }
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  load_hor_r(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
  #if HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
}
#endif


#ifdef wavelet_hor_split
__kernel void wavelet_hor(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);
  size_t grx = get_group_id(0);
  
  #ifndef WAVELET_OUTPUT_INTERLACED
	size_t gx = get_global_id(0);
	size_t gy = get_global_id(1);
    int gx_l = gx * HOR_PAIRS_PER_THREAD;
    int l_to_h_stride = width>>(1+depth);
    int out_stride = 1;
	int in_stride = 1;
  #else
	size_t gx = get_global_id(0) << depth;
	size_t gy = get_global_id(1) << depth;
    int gx_l = gx * HOR_PIXELS_PER_THREAD;
    int l_to_h_stride = 1 << depth;
    int out_stride = 1 << (1 + depth);
	int in_stride = 1 << depth;
  #endif

  __local float temp_image[HOR_LOCAL_SIZE_ALIGN];

  __global float *g_in_image = in_image + lx * in_stride + gy * width;
  __global float *g_out_image_l = out_image + gx_l + gy * width;
  __global float *g_out_image_h = g_out_image_l + l_to_h_stride;
  size_t l_load_image_pos = lx + ly * HOR_LOCAL_SIZE_X;
  size_t l_proc_image_pos = lx * HOR_PIXELS_PER_THREAD + ly * HOR_LOCAL_SIZE_X;

  if(grx == 0)
	{
	  if(grx == get_num_groups(0) - 1)
		{
		  load_hor_lr(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
		}
	  else
		{
		  load_hor_l(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
		}
	  #if HOR_ATOM == 0
	  barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    #if HOR_PROC_TYPE == PROC_TYPE_LAAN
      proc_hor_start(temp_image, l_proc_image_pos, lx);
    #endif
	  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
	}
  else if(grx != get_num_groups(0) - 1)
	{
	  g_in_image += (HOR_PIXELS_PER_GROUP * grx - 4) * in_stride;
	  load_hor_m_split(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
	  #if HOR_ATOM == 0
	  barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    #if HOR_PROC_TYPE == PROC_TYPE_LAAN
      proc_hor_start(temp_image, l_proc_image_pos, lx);
    #endif
	  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
	}
  else
	{
	  g_in_image += (HOR_PIXELS_PER_GROUP * grx - 4) * in_stride;
	  load_hor_r_split(g_in_image, temp_image, lx, l_load_image_pos, in_stride);
	  #if HOR_ATOM == 0
	  barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    #if HOR_PROC_TYPE == PROC_TYPE_LAAN
      proc_hor_start(temp_image, l_proc_image_pos, lx);
    #endif
	  proc_hor(temp_image, g_out_image_l, g_out_image_h, l_proc_image_pos, out_stride);
	}
}
#endif

inline void save_float(OUT_IMAGE_MEM_TYPE(float) out_image, int2 pos, uint width, float data)
{
  #if IMAGE_MEM_TYPE == MEM_TYPE_GLOBAL
  out_image[pos.x + width * pos.y] = data;
  #else
  write_imagef(out_image, pos, (float4)(data, 0.0f, 0.0f, 0.0f));
  #endif
}

inline void save_uchar4(OUT_IMAGE_MEM_TYPE(uchar4) out_image, int2 pos, uint width, uchar data)
{
#if IMAGE_MEM_TYPE == MEM_TYPE_GLOBAL
  out_image[pos.x + width * pos.y] = (uchar4)(data, data, data, 0);
#else
  write_imageui(out_image, pos, (uint4)(data, data, data, 0));
#endif
}

inline float load_float(IN_IMAGE_MEM_TYPE(float) in_image, int2 pos, uint width)
{
#if IMAGE_MEM_TYPE == MEM_TYPE_GLOBAL
  return in_image[pos.x + width * pos.y];
#else
  return read_imagef(in_image, image_samp, pos).x;
#endif
}

inline uchar4 load_uchar4(IN_IMAGE_MEM_TYPE(uchar4) in_image, int2 pos, uint width)
{
#if IMAGE_MEM_TYPE == MEM_TYPE_GLOBAL
  return in_image[pos.x + width * pos.y];
#else
  return convert_uchar4(read_imageui(in_image, image_samp, pos));
#endif
}

__kernel void conv_rgba_gray(OUT_IMAGE_MEM_TYPE(float) out_image, IN_IMAGE_MEM_TYPE(uchar4) in_image, uint width, uint height)
{
  int2 g_id = (int2)(get_global_id(0), get_global_id(1));
  if((g_id.x < width) && (g_id.y < height))
		{
      uchar4 in_pixel = load_uchar4(in_image, g_id, width);
      float out_pixel = 0.299f * in_pixel.s2 + 0.587f * in_pixel.s1 + 0.114f * in_pixel.s0;
      save_float(out_image, g_id, width, out_pixel);
		}
}

__kernel void resize_image(OUT_IMAGE_MEM_TYPE(float) out_image, IN_IMAGE_MEM_TYPE(float) in_image, uint2 out_size, uint2 in_size)
{
  int2 g_out = (int2)(get_global_id(0), get_global_id(1));
  float2 scale = convert_float2(in_size) / convert_float2(out_size);
  int2 g_in = convert_int2(scale * convert_float2(g_out));

  if((g_out.x >= out_size.x) || (g_out.y >= out_size.y)) return;

  save_float(out_image, g_out, out_size.x, load_float(in_image, g_in, in_size.x));
}

#ifndef WAVELET_OUTPUT_INTERLACED
__kernel void conv_gray_rgba(OUT_IMAGE_MEM_TYPE(uchar4) out_image, IN_IMAGE_MEM_TYPE(float) in_image, uint width, uint height, uint depth)
{
  int2 g_id = (int2)(get_global_id(0), get_global_id(1));

  if((g_id.x >= width) || (g_id.y >= height)) return;
	
	size_t l_width = width >> depth;
	size_t l_height = height >> depth;
	
  float in_pixel = load_float(in_image, g_id, width);
	float temp_val = fabs(in_pixel);
	
  if((g_id.x >= l_width) || (g_id.y >= l_height))
		{
			temp_val = log2(temp_val+1)*20;
		}

	unsigned char output_val = convert_uchar_sat(temp_val);

	uchar4 output = (uchar4){output_val, output_val, output_val, 0};
  save_uchar4(out_image, g_id, width, output_val);
}
#else
// neni hotove pro textury
__kernel void conv_gray_rgba(OUT_IMAGE_MEM_TYPE(uchar4) out_image, IN_IMAGE_MEM_TYPE(float) in_image, uint width, uint height, uint depth)
{
  int2 g_out = (int2)(get_global_id(0), get_global_id(1));
	
  if((g_out.x >= width) || (g_out.y >= height)) return;
	int act_depth = depth - 1;
	int shift_x, shift_y;
	for(int i = 0; i < depth; i++)
    {
      shift_x = (g_out.x >= (width >> (i + 1))) ? 1 : 0;
      shift_y = (g_out.y >= (height >> (i + 1))) ? 1 : 0;
			if(shift_x + shift_y > 0)
				{
					act_depth = i;
					break;
				}
    }
  int2 g_in = (int2)(((g_out.x % (width >> (act_depth + 1))) << (act_depth + 1)) + (shift_x << act_depth),
                     ((g_out.y % (height >> (act_depth + 1))) << (act_depth + 1)) + (shift_y << act_depth));
	
	size_t l_width = width >> depth;
	size_t l_height = height >> depth;
	
  float in_pixel = load_float(in_image, g_in, width);
	float temp_val = fabs(in_pixel);
	
  if((g_out.x >= l_width) || (g_out.y >= l_height))
		{
			temp_val = log2(temp_val+1)*20;
		}

	unsigned char output_val = convert_uchar_sat(temp_val);

  save_uchar4(out_image, g_out, width, output_val);
}
#endif




#ifndef COMB_BLOCK_SIZE
  #define COMB_BLOCK_SIZE 256
#endif

#define COMB_BLOCK_SIZE_RATED (COMB_BLOCK_SIZE + 1)

#ifndef COMB_BLOCK_SIZE_X
  #define COMB_BLOCK_SIZE_X 32 
#endif

#define COMB_BLOCK_SIZE_Y (COMB_BLOCK_SIZE / COMB_BLOCK_SIZE_X)

#ifndef COMB_PAIRS_PER_THREAD_X
  #define COMB_PAIRS_PER_THREAD_X 1
#endif

#ifndef COMB_PAIRS_PER_THREAD_Y
  #define COMB_PAIRS_PER_THREAD_Y 1
#endif

#define COMB_PIXELS_PER_THREAD_X (COMB_PAIRS_PER_THREAD_X * 2)

#define COMB_PIXELS_PER_THREAD_Y (COMB_PAIRS_PER_THREAD_Y * 2)

#define COMB_PAIRS_PER_GROUP_X (COMB_PAIRS_PER_THREAD_X * COMB_BLOCK_SIZE_X)

#define COMB_PAIRS_PER_GROUP_Y (COMB_PAIRS_PER_THREAD_Y * COMB_BLOCK_SIZE_Y)

#define COMB_PIXELS_PER_GROUP_X (COMB_PAIRS_PER_GROUP_X * 2)

#define COMB_PIXELS_PER_GROUP_Y (COMB_PAIRS_PER_GROUP_Y * 2)

#define COMB_LOCAL_SIZE_X (COMB_PIXELS_PER_GROUP_X + 7)

#define COMB_LOCAL_SIZE_Y (COMB_PIXELS_PER_GROUP_Y + 7)

#define COMB_LOCAL_SIZE (COMB_LOCAL_SIZE_X * COMB_LOCAL_SIZE_Y)

#define COMB_QUADS_PER_THREAD (COMB_PAIRS_PER_THREAD_X * COMB_PAIRS_PER_THREAD_Y)

#ifndef COMB_HOR_CORNERS_PROC
#define COMB_HOR_CORNERS_PROC 0
#endif

#define COMB_HOR_LOW_STRIDE (COMB_PAIRS_PER_GROUP_X)
#define COMB_HOR_LOW_STRIDE_LOCAL (COMB_PAIRS_PER_GROUP_X + 4)

#if (WARP_SIZE >= COMB_BLOCK_SIZE_X) && ((WARP_SIZE % COMB_BLOCK_SIZE_X) == 0)
#define COMB_HOR_ATOM 1
#else
#define COMB_HOR_ATOM 0
#endif

void load_comb_x(__global float *in_image, __local float *temp_image, size_t lx, size_t x_stride)
{
	__global float *in_image_first = in_image;
	__global float *in_image_last = in_image;
  if(get_group_id(0) == 0)
    {
			if(lx < 4) in_image_first = in_image_first + (8 - 2 * lx) * x_stride;
    }
  else if(get_group_id(0) == get_num_groups(0) - 1)
    {
			if(lx > 3) in_image_last = in_image_last + (6 - 2 * lx) * x_stride;
    }
	
	temp_image[0] = in_image_first[0];
	for(int i = 1; i < COMB_PIXELS_PER_THREAD_X; i++)
		{
			temp_image[i * COMB_BLOCK_SIZE_X] = in_image[i * COMB_BLOCK_SIZE_X * x_stride];
		}
	if(lx < 7)
		{
			temp_image[COMB_PIXELS_PER_GROUP_X] = in_image_last[COMB_PIXELS_PER_GROUP_X * x_stride];
		}
}

void load_comb_x_corner(__global float *in_image, __local float *l_in_image, __local float *l_in_image_copy,  bool is_copy, size_t lx, size_t x_stride)
{
	__global float *in_image_first = in_image;
	__global float *in_image_last = in_image;
	float act_data;
  if(get_group_id(0) == 0)
    {
			if(lx < 4) in_image_first = in_image_first + (8 - 2 * lx) * x_stride;
    }
  else if(get_group_id(0) == get_num_groups(0) - 1)
    {
			if(lx > 3) in_image_last = in_image_last + (6 - 2 * lx) * x_stride;
    }
	if(!is_copy)
		{
			l_in_image[0] = in_image_first[0];
			for(int i = 1; i < COMB_PIXELS_PER_THREAD_X; i++)
				{
					l_in_image[i * COMB_BLOCK_SIZE_X] = in_image[i * COMB_BLOCK_SIZE_X * x_stride];
				}
			if(lx < 7)
				{
					l_in_image[COMB_PIXELS_PER_GROUP_X] = in_image_last[COMB_PIXELS_PER_GROUP_X * x_stride];
				}
		}
	else
		{
			act_data = in_image_first[0];
			l_in_image[0] = act_data;
			l_in_image_copy[0] = act_data;
			for(int i = 1; i < COMB_PIXELS_PER_THREAD_X; i++)
				{
					act_data = in_image[i * COMB_BLOCK_SIZE_X * x_stride];
					l_in_image[i * COMB_BLOCK_SIZE_X] = act_data;
					l_in_image_copy[i * COMB_BLOCK_SIZE_X] = act_data;
				}
			if(lx < 7)
				{
					act_data = in_image_last[COMB_PIXELS_PER_GROUP_X * x_stride];
					l_in_image[COMB_PIXELS_PER_GROUP_X] = act_data;
					l_in_image_copy[COMB_PIXELS_PER_GROUP_X] = act_data;
				}
		}
}

void load_comb_t(__global float *in_image, __local float *temp_image, size_t lx, size_t ly, size_t x_stride, size_t y_stride)
{
  #if COMB_HOR_CORNERS_PROC == 1
  load_comb_x_corner(in_image + 4 * y_stride, temp_image + 4 * COMB_LOCAL_SIZE_X, temp_image + (4 - 2 * ly) * COMB_LOCAL_SIZE_X, (ly < 5) && (ly != 0), lx, x_stride);
  for(int j = 1; j < COMB_PIXELS_PER_THREAD_Y; j++)
  #else
  for(int j = 0; j < COMB_PIXELS_PER_THREAD_Y; j++)
  #endif
    {
      load_comb_x(in_image + (4 + j * COMB_BLOCK_SIZE_Y) * y_stride, temp_image + (4 + j * COMB_BLOCK_SIZE_Y) * COMB_LOCAL_SIZE_X, lx, x_stride);
    }
  if(ly < 3)
    {
      load_comb_x(in_image + (4 + COMB_PIXELS_PER_GROUP_Y) * y_stride, temp_image + (4 + COMB_PIXELS_PER_GROUP_Y) * COMB_LOCAL_SIZE_X, lx, x_stride);
    }
}

void load_comb_m(__global float *in_image, __local float *temp_image, size_t lx, size_t ly, size_t x_stride, size_t y_stride)
{
  for(int j = 0; j < COMB_PIXELS_PER_THREAD_Y; j++)
    {
      load_comb_x(in_image + j * COMB_BLOCK_SIZE_Y * y_stride, temp_image + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, lx, x_stride);
    }
  if(ly < 7)
    {
      load_comb_x(in_image + COMB_PIXELS_PER_GROUP_Y * y_stride, temp_image + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, lx, x_stride);
    }
}

void load_comb_b(__global float *in_image, __local float *temp_image, size_t lx, size_t ly, size_t x_stride, size_t y_stride)
{
  for(int j = 0; j < COMB_PIXELS_PER_THREAD_Y; j++)
    {
      load_comb_x(in_image + j * COMB_BLOCK_SIZE_Y * y_stride, temp_image + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, lx, x_stride);
    }
  if(ly < 4)
    {
      #if COMB_HOR_CORNERS_PROC == 1
      load_comb_x_corner(in_image + COMB_PIXELS_PER_GROUP_Y * y_stride, temp_image + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, temp_image + (COMB_PIXELS_PER_GROUP_Y - 2 * ly + 6) * COMB_LOCAL_SIZE_X, (ly < 3), lx, x_stride);
      #else
      load_comb_x(in_image + COMB_PIXELS_PER_GROUP_Y * y_stride, temp_image + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, lx, x_stride);
      #endif
    }
}

void load_comb_e(__global float *in_image, __local float *temp_image, size_t lx, size_t ly)
{
  for(int j = 0; j < COMB_PIXELS_PER_THREAD_Y; j++)
    {
      for(int i = 0; i < COMB_PIXELS_PER_THREAD_X; i++)
        {
          temp_image[i * COMB_BLOCK_SIZE_X + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X] = 0.0f;
        }
      if(lx < 7)
        {
          temp_image[COMB_PIXELS_PER_GROUP_X + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X] = 0.0f;
        }
    }
  if(ly < 7)
    {
      for(int i = 0; i < COMB_PIXELS_PER_THREAD_X; i++)
        {
          temp_image[i * COMB_BLOCK_SIZE_X + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X] = 0.0f;
        }
      if(lx < 7)
        {
          temp_image[COMB_PIXELS_PER_GROUP_X + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X] = 0.0f;
        }
    }
}

void load_comb_eh(__global float *in_image, __local float *temp_image, size_t lx, size_t ly)
{
  for(int j = 0; j < COMB_PIXELS_PER_THREAD_Y; j++)
    {
      for(int i = 0; i < COMB_PIXELS_PER_THREAD_X; i++)
        {
          temp_image[i * COMB_BLOCK_SIZE_X + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X] = 255.0f;
        }
      if(lx < 7)
        {
          temp_image[COMB_PIXELS_PER_GROUP_X + j * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X] = 255.0f;
        }
    }
  if(ly < 7)
    {
      for(int i = 0; i < COMB_PIXELS_PER_THREAD_X; i++)
        {
          temp_image[i * COMB_BLOCK_SIZE_X + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X] = 255.0f;
        }
      if(lx < 7)
        {
          temp_image[COMB_PIXELS_PER_GROUP_X + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X] = 255.0f;
        }
    }
}

void comb_hor_copy_local_to_private(__local float *l_in_image, __private float *p_in_image)
{
  for(int i = 0; i < COMB_PIXELS_PER_THREAD_X + 5; i++)
    {
      p_in_image[i] = l_in_image[i + 1];
    }
  p_in_image[0] += ALPHA * (p_in_image[1] + l_in_image[0]);
  p_in_image[COMB_PIXELS_PER_THREAD_X + 4] += ALPHA * (p_in_image[COMB_PIXELS_PER_THREAD_X + 3] + l_in_image[COMB_PIXELS_PER_THREAD_X + 6]);
}

void comb_hor_copy_private_to_local(__local float *l_in_image,__private float *private_image)
{
  // save output coefficients
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
    {
      #ifndef WAVELET_OUTPUT_INTERLACED
      l_in_image[i] = ZETA1 * private_image[i * 2 + 3];
      l_in_image[i + COMB_HOR_LOW_STRIDE] = ZETA2 * private_image[i * 2 + 4];
      #else
      l_in_image[i * 2] = ZETA1 * private_image[i * 2 + 3];
      l_in_image[i * 2 + 1] = ZETA2 * private_image[i * 2 + 4];
      #endif
    }
}

void comb_hor_copy_private_to_local_corners(__local float *l_in_image, __local float *l_in_image_copy, bool is_copy, __private float *private_image)
{
  // save output coefficients
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
    {
      float2 act_data = {ZETA1 * private_image[i * 2 + 3], ZETA2 * private_image[i * 2 + 4]};
      #ifndef WAVELET_OUTPUT_INTERLACED
      l_in_image[i] = act_data.x;
      l_in_image[i + COMB_HOR_LOW_STRIDE] = act_data.y;
      #else
      l_in_image[i * 2] = act_data.x;
      l_in_image[i * 2 + 1] = act_data.y;
      #endif
      if(is_copy)
        {
          #ifndef WAVELET_OUTPUT_INTERLACED
          l_in_image_copy[i] = act_data.x;
          l_in_image_copy[i + COMB_HOR_LOW_STRIDE] = act_data.y;
          #else
          l_in_image_copy[i * 2] = act_data.x;
          l_in_image_copy[i * 2 + 1] = act_data.y;
          #endif
        }
    }
}

void comb_hor_blazewicz(__private float *private_image)
{
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_X + 2; i++)
    {
      private_image[i * 2] += ALPHA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_X + 2; i++)
    {
      private_image[i * 2 + 1] += BETA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_X + 2; i++)
    {
      private_image[i * 2] += GAMMA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_X + 1; i++)
    {
      private_image[i * 2 + 1] += DELTA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
}

void proc_comb_hor_blazewicz(__local float *l_in_image_in, __local float *l_in_image_out, size_t ly)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_X + 5];
  for(int i = 0; i < COMB_PIXELS_PER_THREAD_Y; i++)
    {
      comb_hor_copy_local_to_private(l_in_image_in + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
      // compute wavelet transform
      comb_hor_blazewicz(private_image);
      barrier(CLK_LOCAL_MEM_FENCE);
      comb_hor_copy_private_to_local(l_in_image_out + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
    }
  if(ly < 7)
    {
      comb_hor_copy_local_to_private(l_in_image_in + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, private_image);      
      comb_hor_blazewicz(private_image);
    }
  barrier(CLK_LOCAL_MEM_FENCE);
  if(ly < 7)
    {
      comb_hor_copy_private_to_local(l_in_image_out + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, private_image);
    }
}

void proc_comb_hor_blazewicz_m(__local float *l_in_image_in, __local float *l_in_image_out, size_t ly)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_X + 5];
  for(int i = 0; i < COMB_PIXELS_PER_THREAD_Y; i++)
    {
      comb_hor_copy_local_to_private(l_in_image_in + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
      // compute wavelet transform
      comb_hor_blazewicz(private_image);
      #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      comb_hor_copy_private_to_local(l_in_image_out + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
    }
  if(ly < 7)
    {
      comb_hor_copy_local_to_private(l_in_image_in + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, private_image);      
      comb_hor_blazewicz(private_image);
    }
  #if COMB_HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  if(ly < 7)
    {
      comb_hor_copy_private_to_local(l_in_image_out + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, private_image);
    }
}

void proc_comb_hor_blazewicz_t(__local float *l_in_image_in, __local float *l_in_image_out, size_t ly)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_X + 5];
  comb_hor_copy_local_to_private(l_in_image_in + 4 * COMB_LOCAL_SIZE_X, private_image);
  // compute wavelet transform
  comb_hor_blazewicz(private_image);
  #if COMB_HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  comb_hor_copy_private_to_local_corners(l_in_image_out + 4 * COMB_LOCAL_SIZE_X, l_in_image_out + (4 - 2 * ly) * COMB_LOCAL_SIZE_X, ((ly < 5) && (ly != 0)), private_image);
  for(int i = 1; i < COMB_PIXELS_PER_THREAD_Y; i++)
    {
      comb_hor_copy_local_to_private(l_in_image_in + (4 + i * COMB_BLOCK_SIZE_Y) * COMB_LOCAL_SIZE_X, private_image);
      // compute wavelet transform
      comb_hor_blazewicz(private_image);
      #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      comb_hor_copy_private_to_local(l_in_image_out + (4 + i * COMB_BLOCK_SIZE_Y) * COMB_LOCAL_SIZE_X, private_image);
    }
  if(ly < 3)
    {
      comb_hor_copy_local_to_private(l_in_image_in + (4 + COMB_PIXELS_PER_GROUP_Y) * COMB_LOCAL_SIZE_X, private_image);      
      comb_hor_blazewicz(private_image);
    }
  #if COMB_HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  if(ly < 3)
    {
      comb_hor_copy_private_to_local(l_in_image_out + (4 + COMB_PIXELS_PER_GROUP_Y) * COMB_LOCAL_SIZE_X, private_image);
    }
}

void proc_comb_hor_blazewicz_b(__local float *l_in_image_in, __local float *l_in_image_out, size_t ly)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_X + 5];
  for(int i = 0; i < COMB_PIXELS_PER_THREAD_Y; i++)
    {
      comb_hor_copy_local_to_private(l_in_image_in + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
      // compute wavelet transform
      comb_hor_blazewicz(private_image);
      #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      comb_hor_copy_private_to_local(l_in_image_out + i * COMB_BLOCK_SIZE_Y * COMB_LOCAL_SIZE_X, private_image);
    }
  if(ly < 4)
    {
      comb_hor_copy_local_to_private(l_in_image_in + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, private_image);      
      comb_hor_blazewicz(private_image);
    }
  #if COMB_HOR_ATOM == 0
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif
  if(ly < 4)
    {
      comb_hor_copy_private_to_local_corners(l_in_image_out + COMB_PIXELS_PER_GROUP_Y * COMB_LOCAL_SIZE_X, l_in_image_out + (COMB_PIXELS_PER_GROUP_Y - 2 * ly + 6) * COMB_LOCAL_SIZE_X, (ly < 3), private_image);
    }
}

void proc_comb_vert_blazewicz(__local float *local_image, __global float *out_image_h, __global float *out_image_l, size_t y_stride)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_Y + 5];
  for (int i = 0; i < COMB_PIXELS_PER_THREAD_Y + 5; i++)
    {
      private_image[i] = local_image[(i + 1) * COMB_LOCAL_SIZE_X];
    }
  // compute wavelet transform
  private_image[0] += ALPHA * (private_image[1] + local_image[0]);
  private_image[COMB_PIXELS_PER_THREAD_Y + 4] += ALPHA * (private_image[COMB_PIXELS_PER_THREAD_Y + 3] + local_image[(COMB_PIXELS_PER_THREAD_Y + 6) * COMB_LOCAL_SIZE_X]);
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y + 2; i++)
    {
      private_image[i * 2] += ALPHA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_Y + 2; i++)
    {
      private_image[i * 2 + 1] += BETA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y + 2; i++)
    {
      private_image[i * 2] += GAMMA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y + 1; i++)
    {
      private_image[i * 2 + 1] += DELTA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  // save output coefficients
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_Y; i++)
    {
      out_image_h[i * y_stride] = ZETA1 * private_image[i * 2 + 3];
      out_image_l[i * y_stride] = ZETA2 * private_image[i * 2 + 4];
    }
}



#define COMB_PIXELS_PER_THREAD_Y_RATED (COMB_PIXELS_PER_THREAD_Y * 2)
#define COMB_PAIRS_PER_THREAD_Y_RATED (COMB_PAIRS_PER_THREAD_Y * 2)

void proc_comb_vert_rated_blazewicz(__local float *local_image, __global float *out_image_h, __global float *out_image_l, uint stride)
{
  // create private memory
  __private float private_image[COMB_PIXELS_PER_THREAD_Y_RATED + 5];
  for(int i = 0; i < COMB_PIXELS_PER_THREAD_Y_RATED + 5; i++)
    {
      private_image[i] = local_image[(i + 1) * COMB_LOCAL_SIZE_X];
    }
  // compute wavelet transform
  private_image[0] += ALPHA * (private_image[1] + local_image[0]);
  private_image[COMB_PIXELS_PER_THREAD_Y_RATED + 4] += ALPHA * (private_image[COMB_PIXELS_PER_THREAD_Y_RATED + 3] + local_image[(COMB_PIXELS_PER_THREAD_Y_RATED + 6) * COMB_LOCAL_SIZE_X]);
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y_RATED + 2; i++)
    {
      private_image[i * 2] += ALPHA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_Y_RATED + 2; i++)
    {
      private_image[i * 2 + 1] += BETA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y_RATED + 2; i++)
    {
      private_image[i * 2] += GAMMA * (private_image[i * 2 - 1] + private_image[i * 2 + 1]);
    }
  for(int i = 1; i < COMB_PAIRS_PER_THREAD_Y_RATED + 1; i++)
    {
      private_image[i * 2 + 1] += DELTA * (private_image[i * 2] + private_image[i * 2 + 2]);
    }
  // save output coefficients
  for(int i = 0; i < COMB_PAIRS_PER_THREAD_Y_RATED; i++)
    {
      out_image_h[i * stride] = ZETA1 * private_image[i * 2 + 3];
      out_image_l[i * stride] = ZETA2 * private_image[i * 2 + 4];
			/*out_image_h[i * width] = 0.0f;
      out_image_l[i * width] = 0.0f;*/
    }
}

#if defined(wavelet_sep_out_vert_rated) || defined(wavelet_sep_out_basic)
__kernel void wavelet_block(__global float *out_image, __global float *in_image, uint width, uint height, uint depth)
{
  size_t gx = get_global_id(0);
  size_t gy = get_global_id(1);
  size_t grx = get_group_id(0);
  size_t gry = get_group_id(1);
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);

  __local float temp_image[COMB_LOCAL_SIZE];

  size_t max_width = width >> depth;
  size_t max_height = height >> depth;

  #ifndef WAVELET_OUTPUT_INTERLACED
	  size_t x_in_stride = 1;
	  size_t x_out_stride = 1;
	
	  size_t y_in_stride = width;
	  size_t y_out_stride = width;

	  size_t x_l_stride = 1;
	  size_t y_l_stride = 1;

	  int lrx_stride = width >> (1 + depth);
	  int lhy_stride = (height >> (1 + depth)) * width;
  #else
	  size_t x_in_stride = 1 << depth;
	  size_t x_out_stride = 2 * x_in_stride;
	
	  size_t y_in_stride = width * x_in_stride;
	  size_t y_out_stride = 2 * y_in_stride;

	  size_t x_l_stride = 2;
	  size_t y_l_stride = 2;
	
	  int lrx_stride = COMB_BLOCK_SIZE_X << depth;
    int lhy_stride = y_in_stride;
  #endif

  int grx_size = COMB_BLOCK_SIZE_X * 2;
  int gry_size = COMB_BLOCK_SIZE_Y * 2;
  int grx_start = grx_size * grx;
  int gry_start = gry_size * gry;
  int gx_in = grx_start + (lx - 4);
  int gy_in = gry_start + (ly - 4);

  int tx_out_stride = COMB_PAIRS_PER_THREAD_X * x_out_stride;
  int ty_out_stride = COMB_PAIRS_PER_THREAD_Y * y_out_stride;

  int grx_out_stride = COMB_PAIRS_PER_GROUP_X * x_out_stride;
  int gry_out_stride = COMB_PAIRS_PER_GROUP_Y * y_out_stride;

  int bx_out_stride = COMB_BLOCK_SIZE_X * x_out_stride;
  int by_out_stride = COMB_BLOCK_SIZE_Y * y_out_stride;

  int bx_l_stride = COMB_BLOCK_SIZE_X * x_l_stride;
  int by_l_stride = COMB_BLOCK_SIZE_Y * y_l_stride;
   
  __global float *g_in_image = in_image + gx_in * x_in_stride + gy_in * y_in_stride;
  __local float *l_load_image_pos = temp_image + lx + ly * COMB_LOCAL_SIZE_X;

  __local float *l_proc_hor_image_pos_in = temp_image + lx * COMB_PIXELS_PER_THREAD_X + ly * COMB_LOCAL_SIZE_X;
  __local float *l_proc_hor_image_pos_out = temp_image + lx * COMB_PAIRS_PER_THREAD_X * x_l_stride + ly * COMB_LOCAL_SIZE_X;

  #if defined(wavelet_sep_out_basic)
    __global float *g_out_image_h_1 = out_image + gy * ty_out_stride + grx * grx_out_stride + lx * x_in_stride;
    __global float *g_out_image_l_1 = g_out_image_h_1 + lhy_stride;
    __global float *g_out_image_h_2 = g_out_image_h_1 + lrx_stride;
    __global float *g_out_image_l_2 = g_out_image_l_1 + lrx_stride;
    __local float *l_proc_vert_image_pos = temp_image + lx + ly * COMB_PIXELS_PER_THREAD_Y * COMB_LOCAL_SIZE_X;
  #elif defined(wavelet_sep_out_vert_rated)
    size_t lx_vert = get_local_id(0) + get_local_size(0) * (get_local_id(1) & 0x01);
    size_t ly_vert = get_local_id(1) >> 1;

    int ty_out_rated_stride = COMB_PAIRS_PER_THREAD_Y_RATED * y_out_stride;

    __global float *g_out_image_h = out_image + gry * gry_out_stride + ly_vert * ty_out_rated_stride + grx * grx_out_stride + lx * x_in_stride;
    __global float *g_out_image_l = g_out_image_h + lhy_stride;

    if((get_local_id(1) & 0x01) == 1)
      {
        g_out_image_h += lrx_stride;
        g_out_image_l += lrx_stride;
      }

    __local float *l_proc_vert_image_pos = temp_image + lx_vert + ly_vert * COMB_PIXELS_PER_THREAD_Y_RATED * COMB_LOCAL_SIZE_X;
  #endif
  
  if(gry == 0)
    {
      load_comb_t(g_in_image, l_load_image_pos, lx, ly, x_in_stride, y_in_stride);
      #if COMB_HOR_CORNERS_PROC == 1
        barrier(CLK_LOCAL_MEM_FENCE);
        proc_comb_hor_blazewicz(l_proc_hor_image_pos_in, l_proc_hor_image_pos_out, ly);
      #else
        #if COMB_HOR_ATOM == 0
        barrier(CLK_LOCAL_MEM_FENCE);
        #endif
        proc_comb_hor_blazewicz_t(l_proc_hor_image_pos_in, l_proc_hor_image_pos_out, ly);
      #endif
    }
  else if(gry == get_num_groups(1) - 1)
    {
      load_comb_b(g_in_image, l_load_image_pos, lx, ly, x_in_stride, y_in_stride);
      #if COMB_HOR_CORNERS_PROC == 1
        barrier(CLK_LOCAL_MEM_FENCE);
        proc_comb_hor_blazewicz(l_proc_hor_image_pos_in, l_proc_hor_image_pos_out, ly);
      #else
        #if COMB_HOR_ATOM == 0
        barrier(CLK_LOCAL_MEM_FENCE);
        #endif
        proc_comb_hor_blazewicz_b(l_proc_hor_image_pos_in, l_proc_hor_image_pos_out, ly);
      #endif
    }
  else
    {
      load_comb_m(g_in_image, l_load_image_pos, lx, ly, x_in_stride, y_in_stride);
      #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      proc_comb_hor_blazewicz_m(l_proc_hor_image_pos_in, l_proc_hor_image_pos_out, ly);
    }

  barrier(CLK_LOCAL_MEM_FENCE);
  #if defined(wavelet_sep_out_basic)
    for(int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
      {
        proc_comb_vert_blazewicz(l_proc_vert_image_pos + i * bx_l_stride, g_out_image_h_1 + i * bx_out_stride, g_out_image_l_1 + i * bx_out_stride, y_out_stride);
        proc_comb_vert_blazewicz(l_proc_vert_image_pos + i * bx_l_stride + COMB_HOR_LOW_STRIDE, g_out_image_h_2 + i * bx_out_stride, g_out_image_l_2 + i * bx_out_stride, y_out_stride);
      }
  #elif defined(wavelet_sep_out_vert_rated)
    proc_comb_vert_rated_blazewicz(l_proc_vert_image_pos, g_out_image_h, g_out_image_l, y_out_stride);
  #endif
}
#endif


#define COMB_BLOCK_LOC_T (-COMB_PAIRS_PER_GROUP_X)
#define COMB_BLOCK_LOC_B (COMB_PAIRS_PER_GROUP_X)
#define COMB_BLOCK_LOC_L (-1)
#define COMB_BLOCK_LOC_R (1)
#define COMB_BLOCK_LOC_TL (COMB_BLOCK_LOC_T+COMB_BLOCK_LOC_L)
#define COMB_BLOCK_LOC_BL (COMB_BLOCK_LOC_B+COMB_BLOCK_LOC_L)
#define COMB_BLOCK_LOC_TR (COMB_BLOCK_LOC_T+COMB_BLOCK_LOC_R)
#define COMB_BLOCK_LOC_BR (COMB_BLOCK_LOC_B+COMB_BLOCK_LOC_R)

#define COMB_BLOCK_STEP_BORDER_PAIRS (FILTER_LENGTH * 2 - 1)
#define COMB_BLOCK_STEP_BORDER_PIXELS (2 * COMB_BLOCK_STEP_BORDER_PAIRS)

#define COMB_BLOCK_ALL_BORDER_PAIRS (LIFTING_STEPS_COUNT * COMB_BLOCK_STEP_BORDER_PAIRS)
#define COMB_BLOCK_ALL_BORDER_PIXELS (2 * COMB_BLOCK_ALL_BORDER_PAIRS)


/*float2 load_block_x(__global float *in_image, int gx, size_t x_stride, size_t max_width)
{
  __global float *in_image_l = in_image;
  __global float *in_image_h = in_image + x_stride;
  if(get_group_id(0) == 0)
    {
      if(gx<0)
        {
          in_image_l = in_image_l + (- 4 * gx) * x_stride;
          in_image_h = in_image_h + (-2 - 4 * gx) * x_stride;
        }
    }
  else if(get_group_id(0) == get_num_groups(0) - 1)
    {
			if(gx >= max_width)
        {
          in_image_l = in_image_l + (2 - 4 * (gx - (max_width - 1))) * x_stride;
          in_image_h = in_image_h - 4 * (gx - (max_width - 1)) * x_stride;
        }
    }
  return (float2)(in_image_l[0], in_image_h[0]);
}

float4 load_block(IN_IMAGE_MEM_TYPE(float) orig_in_image, int gx, int gy, size_t x_stride, size_t y_stride, size_t max_width, size_t max_height)
{
  float4 out_data;
  #if IMAGE_MEM_TYPE == MEM_TYPE_TEXTURE
  out_data = (float4)(read_imagef(orig_in_image, image_samp, (int2)(gx * x_stride * 2, gy * x_stride * 2)).x,
                      read_imagef(orig_in_image, image_samp, (int2)((gx * 2 + 1) * x_stride, gy * x_stride * 2)).x,
                      read_imagef(orig_in_image, image_samp, (int2)(gx * x_stride * 2, (gy * 2 + 1) * x_stride)).x,
                      read_imagef(orig_in_image, image_samp, (int2)((gx * 2 + 1) * x_stride, (gy * 2 + 1) * x_stride)).x);
  #else
  __global float *in_image = orig_in_image + gx * x_stride * 2 + gy * y_stride * 2;

  if(get_group_id(1) == 0)
    {
      if(gy >= 0)
        {
          out_data = (float4)(load_block_x(in_image, gx, x_stride, max_width),
                              load_block_x(in_image + y_stride, gx, x_stride, max_width));
        }
      else
        {
          out_data = (float4)(load_block_x(in_image - (4 * gy * y_stride), gx, x_stride, max_width),
                              load_block_x(in_image - ((4 * gy + 1) * y_stride), gx, x_stride, max_width));
        }
    }
  else if(get_group_id(1) == get_num_groups(1) - 1)
    {
      if(gy < max_height)
        {
          out_data = (float4)(load_block_x(in_image, gx, x_stride, max_width),
	                            load_block_x(in_image + y_stride, gx, x_stride, max_width));
        }
      else
        {
          out_data = (float4)(load_block_x(in_image - y_stride * (2 + (gy - max_height) * 4), gx, x_stride, max_width),
                              load_block_x(in_image - y_stride * (3 + (gy - max_height) * 4), gx, x_stride, max_width));
        }
    }
  else
    {
      out_data = (float4)(load_block_x(in_image, gx, x_stride, max_width),
	                        load_block_x(in_image + y_stride, gx, x_stride, max_width));
    }
  #endif
  return out_data;
}*/

float2 load_block_x(__global float *in_image, int gx, size_t x_stride, size_t max_width)
{
  int act_x_stride = x_stride;
  __global float *in_image_start = in_image;
  if(gx < 0)
  {
    act_x_stride = -x_stride;
    in_image_start -= x_stride * 4 * gx;
  }
  if(gx >= (int)max_width)
  {
    act_x_stride = -x_stride;
    in_image_start -= x_stride * (2 + 4 * (gx - max_width));
  }
  return (float2)(in_image_start[0 * act_x_stride], in_image_start[1 * act_x_stride]);
}

float4 load_block(IN_IMAGE_MEM_TYPE(float) orig_in_image, int gx, int gy, size_t x_stride, size_t y_stride, size_t max_width, size_t max_height)
{
  float4 out_data;
#if IMAGE_MEM_TYPE == MEM_TYPE_TEXTURE
  out_data = (float4)(read_imagef(orig_in_image, image_samp, (int2)(gx * x_stride * 2, gy * x_stride * 2)).x,
    read_imagef(orig_in_image, image_samp, (int2)((gx * 2 + 1) * x_stride, gy * x_stride * 2)).x,
    read_imagef(orig_in_image, image_samp, (int2)(gx * x_stride * 2, (gy * 2 + 1) * x_stride)).x,
    read_imagef(orig_in_image, image_samp, (int2)((gx * 2 + 1) * x_stride, (gy * 2 + 1) * x_stride)).x);
#else
  __global float *in_image = orig_in_image + gx * x_stride * 2 + gy * y_stride * 2;
  __global float *in_image_start = in_image;
  int act_y_stride = y_stride;
  if(gy < 0)
  {
    act_y_stride = -y_stride;
    in_image_start -= y_stride * 4 * gy;
  }
  if(gy >= (int)max_height)
  {
    act_y_stride = -y_stride;
    in_image_start -= y_stride * (2 + 4 * (gy - max_height));
  }
  out_data = (float4)(load_block_x(in_image_start + 0 * act_y_stride, gx, x_stride, max_width),
                      load_block_x(in_image_start + 1 * act_y_stride, gx, x_stride, max_width));
#endif
  return out_data;
}

#if defined(WAVELET_FILTER_BODY)
  #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * BLOCK_IN_LOCAL_BLOCKS)

#elif defined(wavelet_block_in_polyphase)
  #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
  #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
  #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
  #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 3)
  
  #if (LIFTING_STEPS_COUNT > 1) && defined(DOUBLE_BUFFERING)
    #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 4)
    #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 5)
    #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 6)
    #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 7)

    #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 8)
  #else
    #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 0)
    #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 1)
    #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 2)
    #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 3)

    #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
  #endif
  
  #define BORDER_LINES_B (FILTER_LENGTH * 2 - 2)
  #define BORDER_LINES_R (FILTER_LENGTH * 2 - 2)
  #define BORDER_LINES_T (FILTER_LENGTH * 2 - 1)
  #define BORDER_LINES_L (FILTER_LENGTH * 2 - 1)

#elif defined(wavelet_block_in_convolution)
  #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
  #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
  #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
  #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 3)
  
  #define BORDER_LINES_T (LIFTING_STEPS_COUNT * (FILTER_LENGTH * 2 - 1))
  #define BORDER_LINES_L (LIFTING_STEPS_COUNT * (FILTER_LENGTH * 2 - 1))
  #define BORDER_LINES_B (LIFTING_STEPS_COUNT * (FILTER_LENGTH * 2 - 1) - 1)
  #define BORDER_LINES_R (LIFTING_STEPS_COUNT * (FILTER_LENGTH * 2 - 1) - 1)
  #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)

#elif defined(wavelet_block_in_monolithic)

  #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
  #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
  #if OPTIM_WARP == WAVELET_OPTIM_WARP_SHUFFLE
    #ifdef DOUBLE_BUFFERING
      #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 3)

      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
      #define BORDER_LINES_T (FILTER_LENGTH - 1)
      #define BORDER_LINES_B (FILTER_LENGTH - 1)
    #else
      #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 1)

      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 2)
      #define BORDER_LINES_T FILTER_LENGTH
      #define BORDER_LINES_B FILTER_LENGTH
    #endif
    #define BORDER_LINES_L (FILTER_LENGTH - 1)
    #define BORDER_LINES_R (FILTER_LENGTH - 1)
  #else
    #ifdef DOUBLE_BUFFERING
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 3)
      #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 4)
      #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 5)

      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 6)
      #define BORDER_LINES_L (FILTER_LENGTH - 1)
      #define BORDER_LINES_R (FILTER_LENGTH - 1)
    #else
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 2)

      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
      #define BORDER_LINES_L FILTER_LENGTH
      #define BORDER_LINES_R FILTER_LENGTH
    #endif
    #define BORDER_LINES_B (FILTER_LENGTH - 1)
    #define BORDER_LINES_T (FILTER_LENGTH - 1)
  #endif
  
#elif defined(wavelet_block_in_iwahashi)
  #if OPTIM_WARP == WAVELET_OPTIM_WARP_SHUFFLE
    #ifdef DOUBLE_BUFFERING
      #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)

      #define BORDER_LINES_T (FILTER_LENGTH - 1)
      #define BORDER_LINES_B (FILTER_LENGTH - 1)
      #if LIFTING_STEPS_COUNT > 1
        #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
        #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 3)

        #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
      #else
        #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 2)

        #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
      #endif
    #else
      #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 1)

      #define BORDER_LINES_T FILTER_LENGTH
      #define BORDER_LINES_B FILTER_LENGTH
      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 2)
    #endif

    #define COMB_BLOCK_LOC_LL2 COMB_BLOCK_LOC_LL1
    #define COMB_BLOCK_LOC_HL2 COMB_BLOCK_LOC_HL1
    #define COMB_BLOCK_LOC_LH2 COMB_BLOCK_LOC_LH1
    #define COMB_BLOCK_LOC_HH2 COMB_BLOCK_LOC_HH1

    #define BORDER_LINES_L (FILTER_LENGTH - 1)
    #define BORDER_LINES_R (FILTER_LENGTH - 1)
  #else
    #if OPTIM_THREAD == 0
      #ifdef DOUBLE_BUFFERING
        #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
        #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
        #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 3)

        #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 0)
        #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 2)
        #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 3)

        #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
        #define BORDER_LINES_L (FILTER_LENGTH - 1)
        #define BORDER_LINES_R (FILTER_LENGTH - 1)
        #define BORDER_LINES_B (FILTER_LENGTH - 1)
      #else
        #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 0)
        #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
        #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 2)

        #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 2)
        #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 0)
        #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 1)

        #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
        #define BORDER_LINES_L FILTER_LENGTH
        #define BORDER_LINES_R FILTER_LENGTH
        #define BORDER_LINES_B FILTER_LENGTH
      #endif
  
      #define BORDER_LINES_T (FILTER_LENGTH - 1) 
    #else
      #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
      #ifdef DOUBLE_BUFFERING
        #if LIFTING_STEPS_COUNT > 1
          #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 3)
          #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 4)
          #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 5)

          #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 6)
        #else
          #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 1)
          #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 2)
          #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 3)

          #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
        #endif
        #define BORDER_LINES_L (FILTER_LENGTH - 1)
        #define BORDER_LINES_R (FILTER_LENGTH - 1)
      #else
        #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 0)
        #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 1)
        #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 2)

        #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
        #define BORDER_LINES_L FILTER_LENGTH
        #define BORDER_LINES_R FILTER_LENGTH
      #endif
  
      #define BORDER_LINES_B (FILTER_LENGTH - 1)
      #define BORDER_LINES_T (FILTER_LENGTH - 1)
    #endif
  #endif
#elif defined(wavelet_block_in_explosive)
  #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
  #ifdef DOUBLE_BUFFERING
    #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
    #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
    #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 0)

    #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 1)
    #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 0)
    #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 2)
    #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 1)

    #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
    #define BORDER_LINES_L FILTER_LENGTH
    #define BORDER_LINES_T FILTER_LENGTH
    #if OPTIM_WARP == WAVELET_OPTIM_WARP_SHUFFLE
      #define BORDER_LINES_R (FILTER_LENGTH - 1)
    #else
      #define BORDER_LINES_R FILTER_LENGTH
    #endif
  #else
    #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 0)
    #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 1)
    #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 1)

    #define COMB_BLOCK_LOC_LL2 COMB_BLOCK_LOC_LL1
    #define COMB_BLOCK_LOC_HL2 COMB_BLOCK_LOC_HL1
    #define COMB_BLOCK_LOC_LH2 COMB_BLOCK_LOC_LH1
    #define COMB_BLOCK_LOC_HH2 COMB_BLOCK_LOC_HH1

    #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 2)

    #define BORDER_LINES_T (FILTER_LENGTH - 1)
    #if OPTIM_WARP == WAVELET_OPTIM_WARP_SHUFFLE
      #define BORDER_LINES_L (FILTER_LENGTH - 1)
      #define BORDER_LINES_R (FILTER_LENGTH - 1)
    #else
      #define BORDER_LINES_L FILTER_LENGTH
      #define BORDER_LINES_R FILTER_LENGTH
    #endif
  #endif
  
  #define BORDER_LINES_B (FILTER_LENGTH - 1)
#elif defined(wavelet_block_in_sweldens)
  #ifdef DOUBLE_BUFFERING
    #if (OPTIM_WARP != WAVELET_OPTIM_WARP_NONE) 
      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 4)
    #else
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 1)
    
      #define COMB_BLOCK_LOC_LH2 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_LL2 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HL2 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_HH2 (COMB_BLOCK_SIZE * 0)

      #define COMB_BLOCK_LOC_LH3 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_LL3 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_HL3 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HH3 (COMB_BLOCK_SIZE * 2)
    
      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
    #endif
  #else
    #if (OPTIM_WARP == WAVELET_OPTIM_WARP_LOCAL) 
      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 3)
    #else 
      #define COMB_BLOCK_LOC_LL1 (COMB_BLOCK_SIZE * 0)
      #define COMB_BLOCK_LOC_HL1 (COMB_BLOCK_SIZE * 1)
      #define COMB_BLOCK_LOC_LH1 (COMB_BLOCK_SIZE * 2)
      #define COMB_BLOCK_LOC_HH1 (COMB_BLOCK_SIZE * 3)
    
      #define COMB_BLOCK_LOC_LL2 COMB_BLOCK_LOC_LL1
      #define COMB_BLOCK_LOC_HL2 COMB_BLOCK_LOC_HL1
      #define COMB_BLOCK_LOC_LH2 COMB_BLOCK_LOC_LH1
      #define COMB_BLOCK_LOC_HH2 COMB_BLOCK_LOC_HH1

      #define COMB_BLOCK_LOC_LL3 COMB_BLOCK_LOC_LL2
      #define COMB_BLOCK_LOC_HL3 COMB_BLOCK_LOC_HL2
      #define COMB_BLOCK_LOC_LH3 COMB_BLOCK_LOC_LH2
      #define COMB_BLOCK_LOC_HH3 COMB_BLOCK_LOC_HH1
    
      #define BLOCK_IN_LOCAL_SIZE (COMB_BLOCK_SIZE * 2)
    #endif
  #endif

  #define BORDER_LINES_B FILTER_LENGTH
  #define BORDER_LINES_T FILTER_LENGTH
  #define BORDER_LINES_L (FILTER_LENGTH - 1)
  #define BORDER_LINES_R (FILTER_LENGTH - 1)
#endif

#define    CALL2D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,   ITER_START_2,   INC_1, INC_2, OP)\
           if((ITER_START_1<FILTER_LENGTH)&&(ITER_START_2<FILTER_LENGTH)&&((INC_1 != 0)||(INC_2 != 0)||(ITER_START_1!=0)||(ITER_START_2!=0))) FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1, ITER_START_2, INC_1, INC_2, OP);\
           if((ITER_START_1<FILTER_LENGTH)&&(ITER_START_2+1<FILTER_LENGTH)) FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1, ITER_START_2+1, INC_1, INC_2, OP);\
           if((ITER_START_1+1<FILTER_LENGTH)&&(ITER_START_2<FILTER_LENGTH)) FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+1, ITER_START_2, INC_1, INC_2, OP);\
           if((ITER_START_1+1<FILTER_LENGTH)&&(ITER_START_2+1<FILTER_LENGTH)) FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+1, ITER_START_2+1, INC_1, INC_2, OP);

#define    CALL2D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+2,   INC_1, INC_2, OP)\
           CALL2D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+2,   ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+2,   ITER_START_2+2,   INC_1, INC_2, OP)

#define    CALL2D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+4,   INC_1, INC_2, OP)\
           CALL2D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+4,   ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+4,   ITER_START_2+4,   INC_1, INC_2, OP)

#define   CALL2D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+8,   INC_1, INC_2, OP)\
           CALL2D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+8,   ITER_START_2,     INC_1, INC_2, OP)\
           CALL2D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+8,   ITER_START_2+8,   INC_1, INC_2, OP)

#define   CALL2D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+16,  INC_1, INC_2, OP)\
          CALL2D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+16,  ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+16,  ITER_START_2+16,  INC_1, INC_2, OP)

#define   CALL2D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+32,  INC_1, INC_2, OP)\
          CALL2D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+32,  ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+32,  ITER_START_2+32,  INC_1, INC_2, OP)

#define  CALL2D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+64,  INC_1, INC_2, OP)\
          CALL2D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+64,  ITER_START_2,     INC_1, INC_2, OP)\
          CALL2D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+64,  ITER_START_2+64,  INC_1, INC_2, OP)

#define  CALL2D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+128, INC_1, INC_2, OP)\
         CALL2D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+128, ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+128, ITER_START_2+128, INC_1, INC_2, OP)

#define  CALL2D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+256, INC_1, INC_2, OP)\
         CALL2D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+256, ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+256, ITER_START_2+256, INC_1, INC_2, OP)

#define CALL2D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+512, INC_1, INC_2, OP)\
         CALL2D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+512, ITER_START_2,     INC_1, INC_2, OP)\
         CALL2D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+512, ITER_START_2+512, INC_1, INC_2, OP)

#define CALL2D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
        CALL2D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
        CALL2D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+1024, INC_1, INC_2, OP)\
        CALL2D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+1024, ITER_START_2,    INC_1, INC_2, OP)\
        CALL2D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+1024, ITER_START_2+1024, INC_1, INC_2, OP)

#define CALL2D4096(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
        CALL2D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2,     INC_1, INC_2, OP)\
        CALL2D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1,     ITER_START_2+2048, INC_1, INC_2, OP)\
        CALL2D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+2048, ITER_START_2,    INC_1, INC_2, OP)\
        CALL2D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_START_1+2048, ITER_START_2+2048, INC_1, INC_2, OP)


#define    CALL1D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  if((ITER_START<FILTER_LENGTH)&&((INC != 0) || (ITER_START != 0))) FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP); if(ITER_START+1<FILTER_LENGTH)FUNC(VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+1, INC, OP);
#define    CALL1D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D2(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+2, INC, OP)
#define    CALL1D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D4(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+4, INC, OP)
#define   CALL1D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)    CALL1D8(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+8, INC, OP)
#define   CALL1D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D16(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+16, INC, OP)
#define   CALL1D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D32(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+32, INC, OP)
#define  CALL1D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)   CALL1D64(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+64, INC, OP)
#define  CALL1D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D128(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+128, INC, OP)
#define  CALL1D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D256(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+256, INC, OP)
#define CALL1D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP)  CALL1D512(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+512, INC, OP)
#define CALL1D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP) CALL1D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP) CALL1D1024(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+1024, INC, OP)
#define CALL1D4096(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP) CALL1D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START, INC, OP) CALL1D2048(FUNC, VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER_START+2048, INC, OP)

#define UNROLL_2D(VAR, BUF, STEP_POS, COEF_POS, DIR_POS_1, DIR_POS_2, ITER_1, ITER_2, INC_1, INC_2, OP) VAR OP BUF[STEP_POS + ITER_1]*BUF[STEP_POS + ITER_2]*temp_image[COEF_POS + DIR_POS_1 * (ITER_1 + INC_1) + DIR_POS_2 * (ITER_2 + INC_2)];

#define UNROLL_1D(VAR, BUF, STEP_POS, COEF_POS, DIR_POS, ITER, INC, OP) VAR OP BUF[STEP_POS + ITER]*temp_image[COEF_POS + DIR_POS * (ITER + INC)];

#define FIRST_1D(VAR, BUF, STEP_POS, OP, FIRST, FIRST_USE) if(FIRST_USE) VAR OP BUF[STEP_POS]*FIRST;

#define FIRST_2D(VAR, BUF, STEP_POS, OP, FIRST, FIRST_USE) if(FIRST_USE) VAR OP BUF[STEP_POS]*BUF[STEP_POS]*FIRST;

#define CALC_1D_LONG(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP) CALL1D8(UNROLL_1D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS, 0, 1, OP)
#define CALC_1D_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP, FIRST, FIRST_USE) FIRST_1D(VAR, BUF, STEP_ID * FILTER_LENGTH, OP, FIRST, FIRST_USE) CALL1D8(UNROLL_1D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS, 0, 0, OP)

#define CALC_2D_LONG_LONG(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 0, 1, 1, OP)
#define CALC_2D_LONG_LONG_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 0, 1, 1, OP)
#define CALC_2D_SHORT_LONG(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 0, 0, 1, OP)
#define CALC_2D_SHORT_LONG_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 1, 0, 0, 1, OP)
#define CALC_2D_LONG_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 0, 1, 0, OP)
#define CALC_2D_LONG_SHORT_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 1, 1, 0, OP)
#define CALC_2D_SHORT_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST) FIRST_2D(VAR, BUF, STEP_ID * FILTER_LENGTH, OP, FIRST, true) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 0, 0, 0, 0, OP)
#define CALC_2D_SHORT_SHORT_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST) FIRST_2D(VAR, BUF, STEP_ID * FILTER_LENGTH, OP, FIRST, false) CALL2D8(UNROLL_2D, VAR, BUF, STEP_ID * FILTER_LENGTH, COEF_POS, DIR_POS_1, DIR_POS_2, 1, 1, 0, 0, OP)

#define CALC_2D_FULL(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST)\
        CALC_2D_SHORT_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST)\
        CALC_2D_SHORT_LONG(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, -DIR_POS_2, OP)\
        CALC_2D_LONG_SHORT(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS_1, DIR_POS_2, OP)\
        CALC_2D_LONG_LONG(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS_1, -DIR_POS_2, OP)

#define CALC_2D_FULL_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST)\
        CALC_2D_SHORT_SHORT_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, DIR_POS_2, OP, FIRST)\
        CALC_2D_SHORT_LONG_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS_1, -DIR_POS_2, OP)\
        CALC_2D_LONG_SHORT_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS_1, DIR_POS_2, OP)\
        CALC_2D_LONG_LONG_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS_1, -DIR_POS_2, OP)

#define CALC_1D_FULL(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP, FIRST) CALC_1D_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP, FIRST, true)  CALC_1D_LONG(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS, OP)
#define CALC_1D_FULL_IMPROVED(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP, FIRST) CALC_1D_SHORT(VAR, BUF, STEP_ID, COEF_POS, DIR_POS, OP, FIRST, false)  CALC_1D_LONG(VAR, BUF, STEP_ID, COEF_POS, -DIR_POS, OP)

#if defined(WAVELET_FILTER_BODY)
#if (defined(wavelet_block_in_sweldens) || \
    defined(wavelet_block_in_convolution_sep_none) || defined(wavelet_block_in_convolution_sep_hor) || defined(wavelet_block_in_convolution_sep_vert) || defined(wavelet_block_in_convolution_sep_all) || \
    defined(wavelet_block_in_polyphase_sep_none) || defined(wavelet_block_in_polyphase_sep_hor) || defined(wavelet_block_in_polyphase_sep_vert) || defined(wavelet_block_in_polyphase_sep_all)) && (OPTIM_WARP == WAVELET_OPTIM_WARP_LOCAL)
void proc_block(volatile __local float *temp_image, float4 *act_data)
{
  WAVELET_FILTER_BODY();
}
#else
void proc_block(__local float *temp_image, float4 *act_data)
{
	WAVELET_FILTER_BODY();
}
#endif
#else

#ifdef wavelet_block_in_iwahashi
#if OPTIM_THREAD == 0
void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

  temp_image[COMB_BLOCK_LOC_HL1] = act_data[0].y;
  temp_image[COMB_BLOCK_LOC_LH1] = act_data[0].z;
  #pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT - 1; i+=2)
    {
      #ifndef DOUBLE_BUFFERING
        if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_LL1] = act_data[0].x;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, +=, act_data[0].z);
      CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_T, +=, act_data[0].y);
      CALC_2D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, +=, act_data[0].x);

      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_HH1] = act_data[0].w;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, +=, act_data[0].x);
      CALC_1D_FULL(act_data[0].y, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_B, +=, act_data[0].w);
      CALC_1D_FULL(act_data[0].z, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, +=, act_data[0].x);
      CALC_1D_FULL(act_data[0].z, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_R, +=, act_data[0].w);
      
      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_HL2] = act_data[0].y;
      temp_image[COMB_BLOCK_LOC_LH2] = act_data[0].z;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_R, +=, act_data[0].y);
      CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, +=, act_data[0].z);
      CALC_2D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, -=, act_data[0].w);

      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_LL2] = act_data[0].x;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].w, predicts, i+1, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_L, +=, act_data[0].z);
      CALC_1D_FULL(act_data[0].w, predicts, i+1, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_T, +=, act_data[0].y);
      CALC_2D_FULL(act_data[0].w, predicts, i+1, COMB_BLOCK_LOC_LL2, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, +=, act_data[0].x);

      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_HH2] = act_data[0].w;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].y, predicts, i+1, COMB_BLOCK_LOC_LL2, COMB_BLOCK_LOC_L, +=, act_data[0].x);
      CALC_1D_FULL(act_data[0].y, updates, i+1, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, +=, act_data[0].w);
      CALC_1D_FULL(act_data[0].z, predicts, i+1, COMB_BLOCK_LOC_LL2, COMB_BLOCK_LOC_T, +=, act_data[0].x);
      CALC_1D_FULL(act_data[0].z, updates, i+1, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_R, +=, act_data[0].w);

      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_HL1] = act_data[0].y;
      temp_image[COMB_BLOCK_LOC_LH1] = act_data[0].z;
      barrier(CLK_LOCAL_MEM_FENCE);

      CALC_1D_FULL(act_data[0].x, updates, i+1, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_R, +=, act_data[0].y);
      CALC_1D_FULL(act_data[0].x, updates, i+1, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_B, +=, act_data[0].z);
      CALC_2D_FULL(act_data[0].x, updates, i+1, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, -=, act_data[0].w);
    }
  #if LIFTING_STEPS_COUNT % 2 != 0 
    int i = LIFTING_STEPS_COUNT - 1;
    
    #if (!defined(DOUBLE_BUFFERING)) && (LIFTING_STEPS_COUNT != 1)
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_LL1] = act_data[0].x;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, +=, act_data[0].z);
    CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_T, +=, act_data[0].y);
    CALC_2D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, +=, act_data[0].x);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_HH1] = act_data[0].w;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, += , act_data[0].x);
    CALC_1D_FULL(act_data[0].y, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_B, += , act_data[0].w);
    CALC_1D_FULL(act_data[0].z, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, += , act_data[0].x);
    CALC_1D_FULL(act_data[0].z, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_R, += , act_data[0].w);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_HL2] = act_data[0].y;
    temp_image[COMB_BLOCK_LOC_LH2] = act_data[0].z;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_R, +=, act_data[0].y);
    CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, +=, act_data[0].z);
    CALC_2D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, -=, act_data[0].w);
  #endif

  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
}

#else
void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;
#pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
  {
    act_data[0].y += predicts[i] * act_data[0].x;
    act_data[0].w += predicts[i] * act_data[0].z;
    act_data[0].z += predicts[i] * act_data[0].x;
    act_data[0].w += predicts[i] * act_data[0].y;

#ifndef DOUBLE_BUFFERING
    if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[COMB_BLOCK_LOC_LL1] = act_data[0].x;
    temp_image[COMB_BLOCK_LOC_HL1] = act_data[0].y;
    temp_image[COMB_BLOCK_LOC_LH1] = act_data[0].z;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, +=, act_data[0].z);
    CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_T, +=, act_data[0].y);
    CALC_2D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, +=, act_data[0].x);

#ifndef DOUBLE_BUFFERING
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[COMB_BLOCK_LOC_HH2] = act_data[0].w;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL_IMPROVED(act_data[0].y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, +=, act_data[0].x);
    CALC_1D_FULL_IMPROVED(act_data[0].y, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, +=, act_data[0].w);
    CALC_1D_FULL_IMPROVED(act_data[0].z, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, +=, act_data[0].x);
    CALC_1D_FULL_IMPROVED(act_data[0].z, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_R, +=, act_data[0].w);

#ifndef DOUBLE_BUFFERING
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[COMB_BLOCK_LOC_HL2] = act_data[0].y;
    temp_image[COMB_BLOCK_LOC_LH2] = act_data[0].z;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_R, +=, act_data[0].y);
    CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, +=, act_data[0].z);
    CALC_2D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, -=, act_data[0].w);

    act_data[0].x += updates[i] * act_data[0].y;
    act_data[0].z += updates[i] * act_data[0].w;
    act_data[0].x += updates[i] * act_data[0].z;
    act_data[0].y += updates[i] * act_data[0].w;
  }
  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
}
#endif
#endif

#if defined(wavelet_block_in_monolithic)
void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;
  #pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
    {
      #ifndef DOUBLE_BUFFERING
        if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_LL1] = act_data[0].x;
      temp_image[COMB_BLOCK_LOC_HL1] = act_data[0].y;
      temp_image[COMB_BLOCK_LOC_LH1] = act_data[0].z;
      barrier(CLK_LOCAL_MEM_FENCE);

      #if OPTIM_THREAD == 1
        CALC_1D_FULL_IMPROVED(act_data[0].y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, += , act_data[0].x);
        CALC_1D_FULL_IMPROVED(act_data[0].z, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, += , act_data[0].x);
        CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, += , act_data[0].z);
        CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_T, += , act_data[0].y);
        CALC_2D_FULL_IMPROVED(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, += , act_data[0].x);

        act_data[0].y += predicts[i] * act_data[0].x;
        act_data[0].w += predicts[i] * act_data[0].z;
        act_data[0].z += predicts[i] * act_data[0].x;
        act_data[0].w += predicts[i] * act_data[0].y;
      #else
        CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, +=, act_data[0].z);
        CALC_1D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_T, +=, act_data[0].y);
        CALC_1D_FULL(act_data[0].y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, +=, act_data[0].x);
        CALC_1D_FULL(act_data[0].z, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, +=, act_data[0].x);
        CALC_2D_FULL(act_data[0].w, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, +=, act_data[0].x);
      #endif
      
      #ifndef DOUBLE_BUFFERING
        barrier(CLK_LOCAL_MEM_FENCE);
      #endif
      temp_image[COMB_BLOCK_LOC_HL2] = act_data[0].y;
      temp_image[COMB_BLOCK_LOC_LH2] = act_data[0].z;
      temp_image[COMB_BLOCK_LOC_HH2] = act_data[0].w;
      barrier(CLK_LOCAL_MEM_FENCE);
      
      #if OPTIM_THREAD == 1
        CALC_1D_FULL_IMPROVED(act_data[0].z, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_R, += , act_data[0].w);
        CALC_1D_FULL_IMPROVED(act_data[0].y, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, += , act_data[0].w);
        CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_R, += , act_data[0].y);
        CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, += , act_data[0].z);
        CALC_2D_FULL_IMPROVED(act_data[0].x, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, += , act_data[0].w);

        act_data[0].z += updates[i] * act_data[0].w;
        act_data[0].x += updates[i] * act_data[0].y;
        act_data[0].y += updates[i] * act_data[0].w;
        act_data[0].x += updates[i] * act_data[0].z;
      #else
        CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_R, +=, act_data[0].y);
        CALC_1D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, +=, act_data[0].z);
        CALC_1D_FULL(act_data[0].y, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, +=, act_data[0].w);
        CALC_1D_FULL(act_data[0].z, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_R, +=, act_data[0].w);
        CALC_2D_FULL(act_data[0].x, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, +=, act_data[0].w);
      #endif
    }
  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
}
#endif


#if defined(wavelet_block_in_explosive)
void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

  __private int ll_id[] = {COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_LL2};
  __private int hl_id[] = {COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_HL2};
  __private int lh_id[] = {COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_LH2};
  __private int hh_id[] = {COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_HH2};
#pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
  {
#if OPTIM_THREAD == 1
    act_data[0].y += predicts[i] * act_data[0].x;
    act_data[0].w += predicts[i] * act_data[0].z;
    act_data[0].z += predicts[i] * act_data[0].x;
    act_data[0].w += predicts[i] * act_data[0].y;
#endif

#ifndef DOUBLE_BUFFERING
    if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[ll_id[i%2]] = act_data[0].x;
    barrier(CLK_LOCAL_MEM_FENCE);

#if OPTIM_THREAD == 1
    CALC_1D_FULL_IMPROVED(act_data[0].y, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_L, += , act_data[0].x);
    CALC_1D_FULL_IMPROVED(act_data[0].z, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_T, += , act_data[0].x);
    CALC_2D_FULL_IMPROVED(act_data[0].w, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, -= , act_data[0].x);
#else
    CALC_1D_FULL(act_data[0].y, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_L, += , act_data[0].x);
    CALC_1D_FULL(act_data[0].z, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_T, += , act_data[0].x);
    CALC_2D_FULL(act_data[0].w, predicts, i, ll_id[i%2], COMB_BLOCK_LOC_T, COMB_BLOCK_LOC_L, -= , act_data[0].x);
#endif

#ifndef DOUBLE_BUFFERING
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[hl_id[i%2]] = act_data[0].y;
    temp_image[lh_id[i%2]] = act_data[0].z;
    barrier(CLK_LOCAL_MEM_FENCE);

#if OPTIM_THREAD == 1
    CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, lh_id[i%2], COMB_BLOCK_LOC_B, += , act_data[0].z);
    CALC_1D_FULL_IMPROVED(act_data[0].x, updates, i, hl_id[i%2], COMB_BLOCK_LOC_R, += , act_data[0].y);
    CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, lh_id[i%2], COMB_BLOCK_LOC_L, += , act_data[0].z);
    CALC_1D_FULL_IMPROVED(act_data[0].w, predicts, i, hl_id[i%2], COMB_BLOCK_LOC_T, += , act_data[0].y);
#else
    CALC_1D_FULL(act_data[0].x, updates, i, lh_id[i%2], COMB_BLOCK_LOC_B, += , act_data[0].z);
    CALC_1D_FULL(act_data[0].x, updates, i, hl_id[i%2], COMB_BLOCK_LOC_R, += , act_data[0].y);
    CALC_1D_FULL(act_data[0].w, predicts, i, lh_id[i%2], COMB_BLOCK_LOC_L, += , act_data[0].z);
    CALC_1D_FULL(act_data[0].w, predicts, i, hl_id[i%2], COMB_BLOCK_LOC_T, += , act_data[0].y);
#endif

#ifndef DOUBLE_BUFFERING
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
    temp_image[hh_id[i%2]] = act_data[0].w;
    barrier(CLK_LOCAL_MEM_FENCE);

#if OPTIM_THREAD == 1
    CALC_2D_FULL_IMPROVED(act_data[0].x, updates, i, hh_id[i%2], COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, += , act_data[0].w);
    CALC_1D_FULL_IMPROVED(act_data[0].y, updates, i, hh_id[i%2], COMB_BLOCK_LOC_B, += , act_data[0].w);
    CALC_1D_FULL_IMPROVED(act_data[0].z, updates, i, hh_id[i%2], COMB_BLOCK_LOC_R, += , act_data[0].w);

    act_data[0].x += updates[i] * act_data[0].y;
    act_data[0].z += updates[i] * act_data[0].w;
    act_data[0].x += updates[i] * act_data[0].z;
    act_data[0].y += updates[i] * act_data[0].w;
#else
    CALC_2D_FULL(act_data[0].x, updates, i, hh_id[i%2], COMB_BLOCK_LOC_B, COMB_BLOCK_LOC_R, += , act_data[0].w);
    CALC_1D_FULL(act_data[0].y, updates, i, hh_id[i%2], COMB_BLOCK_LOC_B, += , act_data[0].w);
    CALC_1D_FULL(act_data[0].z, updates, i, hh_id[i%2], COMB_BLOCK_LOC_R, += , act_data[0].w);
#endif
  }

  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
}
#endif

#if defined(wavelet_block_in_convolution)
#define HALF_FILTER_SIZE LIFTING_STEPS_COUNT * (2 * FILTER_LENGTH - 1)
#define FIR_FILTER_SIZE (HALF_FILTER_SIZE * 2 + 1)

#define FIR_FILTER_PART_LL (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 0)
#define FIR_FILTER_PART_HL (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 1)
#define FIR_FILTER_PART_LH (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 2)
#define FIR_FILTER_PART_HH (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 3)

void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

  __private float fir_ll[] = ACTUAL_FIR_LL1;
  __private float fir_hl[] = ACTUAL_FIR_HL1;
  __private float fir_lh[] = ACTUAL_FIR_LH1;
  __private float fir_hh[] = ACTUAL_FIR_HH1;

  #if OPTIM_THREAD == 1
    act_data[0].y += predicts[0] * act_data[0].x;
    act_data[0].w += predicts[0] * act_data[0].z;
    act_data[0].z += predicts[0] * act_data[0].x;
    act_data[0].w += predicts[0] * act_data[0].y;
  #endif

  temp_image[COMB_BLOCK_LOC_LL1] = act_data[0].x;
  temp_image[COMB_BLOCK_LOC_HL1] = act_data[0].y;
  temp_image[COMB_BLOCK_LOC_LH1] = act_data[0].z;
  temp_image[COMB_BLOCK_LOC_HH1] = act_data[0].w;

  barrier(CLK_LOCAL_MEM_FENCE);

  float4 act_data2;
  act_data2 = act_data;

  int tmp_index;
  float tmp_val;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL;
  act_data[0].x = fir_ll[tmp_index] * act_data2.x;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL;
  act_data[0].y = fir_hl[tmp_index] * act_data2.y;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH;
  act_data[0].z = fir_lh[tmp_index] * act_data2.z;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH;
  act_data[0].w = fir_hh[tmp_index] * act_data2.w;

  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL;
  if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.x;
  if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.x;
  if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.x;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL;
  if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.y;
  if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.y;
  if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.y;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH;
  if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.z;
  if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.z;
  if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.z;
  tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH;
  if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.w;
  if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.w;
  if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.w;
#pragma unroll
  for(int j = -HALF_FILTER_SIZE; j <= HALF_FILTER_SIZE; j++)
  {
    #pragma unroll
    for(int i = -HALF_FILTER_SIZE; i <= HALF_FILTER_SIZE; i++)
    {
      if((j == 0) && (i == 0)) continue;
      tmp_val = temp_image[COMB_BLOCK_LOC_LL1 + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
      tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL;
      if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
      if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
      if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
      if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];


      if(i != HALF_FILTER_SIZE)
        {
          tmp_val = temp_image[COMB_BLOCK_LOC_HL1 + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }
      tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL;

      if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
      if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
      if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
      if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];


      if(j != HALF_FILTER_SIZE)
        {
          tmp_val = temp_image[COMB_BLOCK_LOC_LH1 + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }
      tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH;

      if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
      if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
      if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
      if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];


      if((i != HALF_FILTER_SIZE) && (j != HALF_FILTER_SIZE))
        {
          tmp_val = temp_image[COMB_BLOCK_LOC_HH1 + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }
      tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH;

      if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
      if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
      if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
      if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];
    }
  }
#if OPTIM_THREAD == 1
  act_data[0].x += updates[0] * act_data[0].y;
  act_data[0].z += updates[0] * act_data[0].w;
  act_data[0].x += updates[0] * act_data[0].z;
  act_data[0].y += updates[0] * act_data[0].w;

  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
#endif
}
#endif

#if defined(wavelet_block_in_polyphase)
#define HALF_FILTER_SIZE (2 * FILTER_LENGTH - 1)
#define FIR_FILTER_SIZE (HALF_FILTER_SIZE * 2 + 1)

#define FIR_FILTER_PART_LL (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 0)
#define FIR_FILTER_PART_HL (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 1)
#define FIR_FILTER_PART_LH (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 2)
#define FIR_FILTER_PART_HH (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 3)
#define FIR_FILTER_STEP    (FIR_FILTER_SIZE * FIR_FILTER_SIZE * 4)

void proc_block(volatile __local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

  __private float fir_ll[] = ACTUAL_FIR_LL1;
  __private float fir_hl[] = ACTUAL_FIR_HL1;
  __private float fir_lh[] = ACTUAL_FIR_LH1;
  __private float fir_hh[] = ACTUAL_FIR_HH1;

  __private size_t ll_id[] = {COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_LL2};
  __private size_t hl_id[] = {COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_HL2};
  __private size_t lh_id[] = {COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_LH2};
  __private size_t hh_id[] = {COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_HH2};

#pragma unroll
  for(int k = 0; k < LIFTING_STEPS_COUNT; k++)
  {
    #if OPTIM_THREAD == 1
      act_data[0].y += predicts[k*FILTER_LENGTH] * act_data[0].x;
      act_data[0].w += predicts[k*FILTER_LENGTH] * act_data[0].z;
      act_data[0].z += predicts[k*FILTER_LENGTH] * act_data[0].x;
      act_data[0].w += predicts[k*FILTER_LENGTH] * act_data[0].y;
    #endif
    #ifndef DOUBLE_BUFFERING
      if(k != 0) barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[ll_id[k%2]] = act_data[0].x;
    temp_image[hl_id[k%2]] = act_data[0].y;
    temp_image[lh_id[k%2]] = act_data[0].z;
    temp_image[hh_id[k%2]] = act_data[0].w;
    barrier(CLK_LOCAL_MEM_FENCE);
    
    float4 act_data2;
    act_data2 = act_data;

    int tmp_index;
    float tmp_val;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL + FIR_FILTER_STEP * k;
    act_data[0].x = fir_ll[tmp_index] * act_data2.x;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL + FIR_FILTER_STEP * k;
    act_data[0].y = fir_hl[tmp_index] * act_data2.y;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH + FIR_FILTER_STEP * k;
    act_data[0].z = fir_lh[tmp_index] * act_data2.z;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH + FIR_FILTER_STEP * k;
    act_data[0].w = fir_hh[tmp_index] * act_data2.w;

    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL + FIR_FILTER_STEP * k;
    if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.x;
    if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.x;
    if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.x;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL + FIR_FILTER_STEP * k;
    if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.y;
    if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.y;
    if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.y;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH + FIR_FILTER_STEP * k;
    if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.z;
    if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.z;
    if(fir_hh[tmp_index] != 0.0f) act_data[0].w += fir_hh[tmp_index] * act_data2.z;
    tmp_index = (0+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (0+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH + FIR_FILTER_STEP * k;
    if(fir_ll[tmp_index] != 0.0f) act_data[0].x += fir_ll[tmp_index] * act_data2.w;
    if(fir_hl[tmp_index] != 0.0f) act_data[0].y += fir_hl[tmp_index] * act_data2.w;
    if(fir_lh[tmp_index] != 0.0f) act_data[0].z += fir_lh[tmp_index] * act_data2.w;
    
    #pragma unroll
    for(int j = -HALF_FILTER_SIZE; j <= HALF_FILTER_SIZE; j++)
    {
      #pragma unroll
      for(int i = -HALF_FILTER_SIZE; i <= HALF_FILTER_SIZE; i++)
      {
        if((j == 0) && (i == 0)) continue;
        tmp_val = temp_image[ll_id[k%2] + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_LL + FIR_FILTER_STEP * k;
        if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
        if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
        if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
        if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];


        if(i != HALF_FILTER_SIZE)
        {
          tmp_val = temp_image[hl_id[k%2] + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }

        tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_HL + FIR_FILTER_STEP * k;

        if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
        if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
        if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
        if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];
    


        if(j != HALF_FILTER_SIZE)
        {
          tmp_val = temp_image[lh_id[k%2] + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }
        tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_LH + FIR_FILTER_STEP * k;
       
        if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
        if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
        if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
        if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];
       


        if((i != HALF_FILTER_SIZE) && (j != HALF_FILTER_SIZE))
        {
          tmp_val = temp_image[hh_id[k%2] + COMB_BLOCK_LOC_R * i + COMB_BLOCK_LOC_B * j];
        }
        tmp_index = (i+HALF_FILTER_SIZE) + FIR_FILTER_SIZE * (j+HALF_FILTER_SIZE) + FIR_FILTER_PART_HH + FIR_FILTER_STEP * k;

        if(fir_ll[tmp_index] != 0.0f)act_data[0].x += tmp_val * fir_ll[tmp_index];
        if(fir_hl[tmp_index] != 0.0f)act_data[0].y += tmp_val * fir_hl[tmp_index];
        if(fir_lh[tmp_index] != 0.0f)act_data[0].z += tmp_val * fir_lh[tmp_index];
        if(fir_hh[tmp_index] != 0.0f)act_data[0].w += tmp_val * fir_hh[tmp_index];
        
      }
    }
    #if OPTIM_THREAD == 1
      act_data[0].x += updates[k*FILTER_LENGTH] * act_data[0].y;
      act_data[0].z += updates[k*FILTER_LENGTH] * act_data[0].w;

      act_data[0].x += updates[k*FILTER_LENGTH] * act_data[0].z;
      act_data[0].y += updates[k*FILTER_LENGTH] * act_data[0].w;
    #endif
  }
  #if OPTIM_THREAD == 1
    act_data[0].x *= ZETA1 * ZETA1;
    act_data[0].w *= ZETA2 * ZETA2;
  #endif
}
#endif


#ifdef wavelet_block_in_sweldens
void proc_block(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;
  __private size_t ll_id[] = {COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_LL2, COMB_BLOCK_LOC_LL3};
  __private size_t lh_id[] = {COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_LH3};
  __private size_t hl_id[] = {COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_HL3};
  __private size_t hh_id[] = {COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_HH3};

  temp_image[lh_id[0]] = act_data[0].z;

#pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
  {
    #ifndef DOUBLE_BUFFERING
      if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[ll_id[i]] = act_data[0].x;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].y, predicts, i, ll_id[i], COMB_BLOCK_LOC_L, +=, act_data[0].x);
    CALC_1D_FULL(act_data[0].w, predicts, i, lh_id[i], COMB_BLOCK_LOC_L, +=, act_data[0].z);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[hl_id[i]] = act_data[0].y;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].z, predicts, i, ll_id[i], COMB_BLOCK_LOC_T, +=, act_data[0].x);
    CALC_1D_FULL(act_data[0].w, predicts, i, hl_id[i], COMB_BLOCK_LOC_T, +=, act_data[0].y);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[hh_id[i]] = act_data[0].w;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].x, updates, i, hl_id[i], COMB_BLOCK_LOC_R, +=, act_data[0].y);
    CALC_1D_FULL(act_data[0].z, updates, i, hh_id[i], COMB_BLOCK_LOC_R, +=, act_data[0].w);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[lh_id[i+1]] = act_data[0].z;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data[0].x, updates, i, lh_id[i+1], COMB_BLOCK_LOC_B, +=, act_data[0].z);
    CALC_1D_FULL(act_data[0].y, updates, i, hh_id[i], COMB_BLOCK_LOC_B, +=, act_data[0].w);
  }
  act_data[0].x *= ZETA1 * ZETA1;
  act_data[0].w *= ZETA2 * ZETA2;
}
#endif


/*#ifdef wavelet_block_in_sweldens_improved
float4 proc_hor_laan(volatile __local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

#pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
  {
    #if (!defined(DOUBLE_BUFFERING)) && (COMB_HOR_ATOM == 0)
      if(i != 0) barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_LL1] = act_data.x;
    temp_image[COMB_BLOCK_LOC_LH1] = act_data.z;
    #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif

    CALC_1D_FULL(act_data.y, predicts, i, COMB_BLOCK_LOC_LL1, COMB_BLOCK_LOC_L, +=, act_data.x);
    CALC_1D_FULL(act_data.w, predicts, i, COMB_BLOCK_LOC_LH1, COMB_BLOCK_LOC_L, +=, act_data.z);

    #if (!defined(DOUBLE_BUFFERING)) && (COMB_HOR_ATOM == 0)
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_HL1] = act_data.y;
    temp_image[COMB_BLOCK_LOC_HH1] = act_data.w;
    #if COMB_HOR_ATOM == 0
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif

    CALC_1D_FULL(act_data.x, updates, i, COMB_BLOCK_LOC_HL1, COMB_BLOCK_LOC_R, +=, act_data.y);
    CALC_1D_FULL(act_data.z, updates, i, COMB_BLOCK_LOC_HH1, COMB_BLOCK_LOC_R, +=, act_data.w);
  }
  return act_data;
}

float4 proc_vert_laan(__local float *temp_image, float4 *act_data)
{
  __private float predicts[] = PREDICT_COEF;
  __private float updates[] = UPDATE_COEF;

#pragma unroll
  for(int i = 0; i < LIFTING_STEPS_COUNT; i++)
  {
    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_LL2] = act_data.x;
    temp_image[COMB_BLOCK_LOC_HL2] = act_data.y;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data.z, predicts, i, COMB_BLOCK_LOC_LL2, COMB_BLOCK_LOC_T, +=, act_data.x);
    CALC_1D_FULL(act_data.w, predicts, i, COMB_BLOCK_LOC_HL2, COMB_BLOCK_LOC_T, +=, act_data.y);

    #ifndef DOUBLE_BUFFERING
      barrier(CLK_LOCAL_MEM_FENCE);
    #endif
    temp_image[COMB_BLOCK_LOC_LH2] = act_data.z;
    temp_image[COMB_BLOCK_LOC_HH2] = act_data.w;
    barrier(CLK_LOCAL_MEM_FENCE);

    CALC_1D_FULL(act_data.x, updates, i, COMB_BLOCK_LOC_LH2, COMB_BLOCK_LOC_B, +=, act_data.z);
    CALC_1D_FULL(act_data.y, updates, i, COMB_BLOCK_LOC_HH2, COMB_BLOCK_LOC_B, +=, act_data.w);
  }
  act_data.x *= ZETA1 * ZETA1;
  act_data.w *= ZETA2 * ZETA2;
  return act_data;
}

void proc_block(__local float *temp_image, float4 *act_data)
{
  float4 out_data = proc_hor_laan(temp_image, act_data);
  out_data = proc_vert_laan(temp_image, out_data);
  return out_data;
}
#endif*/
#endif

#if defined(wavelet_block_in_sweldens) || \
    defined(wavelet_block_in_iwahashi) || \
    defined(wavelet_block_in_explosive) || \
    defined(wavelet_block_in_monolithic) || \
    defined(wavelet_block_in_polyphase) || \
    defined(wavelet_block_in_convolution) || \
    defined(wavelet_block_in_convolution_sep_none) || defined(wavelet_block_in_convolution_sep_hor) || defined(wavelet_block_in_convolution_sep_vert) || defined(wavelet_block_in_convolution_sep_all) || \
    defined(wavelet_block_in_polyphase_sep_none) || defined(wavelet_block_in_polyphase_sep_hor) || defined(wavelet_block_in_polyphase_sep_vert) || defined(wavelet_block_in_polyphase_sep_all)

#define LOCAL_LEFT_BORDER (BORDER_LINES_L + (BORDER_LINES_T * COMB_PAIRS_PER_GROUP_X))
#define LOCAL_RIGHT_BORDER (BORDER_LINES_R + (BORDER_LINES_B * COMB_PAIRS_PER_GROUP_X))
#define KERNEL_LOCAL_SIZE (BLOCK_IN_LOCAL_SIZE * COMB_QUADS_PER_THREAD + LOCAL_LEFT_BORDER + LOCAL_RIGHT_BORDER)

#ifdef MEMLESS_EXEC
__kernel void wavelet_block(float4 in_image, OUT_IMAGE_MEM_TYPE(float) out_image, uint width, uint repeat_count)
{
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);

  __local float temp_image2[KERNEL_LOCAL_SIZE];
  __local float *temp_image = temp_image2 + LOCAL_LEFT_BORDER;
  
  size_t lxy = get_local_size(0) * get_local_id(1) + get_local_id(0);
  __local float *l_image_pos = temp_image + lxy;
  float4 out_data[COMB_QUADS_PER_THREAD];
  #pragma unroll
  for(int i = 0; i < COMB_QUADS_PER_THREAD; i++)
  {
    out_data[i] = in_image;
  }
  #if REPEAT_COUNT != 1
    for(int i = 0; i < repeat_count; i++)
      {
        proc_block(l_image_pos, out_data);
      }
  #else
    proc_block(l_image_pos, out_data);
  #endif

  if(out_data[0].x == 10000000.0f)
  {
    for(int i = 0; i < COMB_QUADS_PER_THREAD; i++)
    {
      save_float(out_image, (int2)(lx, ly * 4 + 0 + get_local_size(1) * 4 * i), width, out_data[i].x);
      save_float(out_image, (int2)(lx, ly * 4 + 1 + get_local_size(1) * 4 * i), width, out_data[i].y);
      save_float(out_image, (int2)(lx, ly * 4 + 2 + get_local_size(1) * 4 * i), width, out_data[i].z);
      save_float(out_image, (int2)(lx, ly * 4 + 3 + get_local_size(1) * 4 * i), width, out_data[i].w);
    }
  }
}
#else
/*__kernel void wavelet_block(OUT_IMAGE_MEM_TYPE(float) out_image, IN_IMAGE_MEM_TYPE(float) in_image, uint width, uint height, uint depth)
{
  size_t grx = get_group_id(0);
  size_t gry = get_group_id(1);
  size_t lx = get_local_id(0);
  size_t ly = get_local_id(1);

  __local float temp_image2[KERNEL_LOCAL_SIZE];
  __local float *temp_image = temp_image2 + LOCAL_LEFT_BORDER;
#ifndef WAVELET_OUTPUT_INTERLACED
  size_t x_stride = 1;
  size_t x_out_stride = 1;

  int lr_stride = width >> (1 + depth);
  int tb_stride = height >> (1 + depth);
#else
  size_t x_stride = 1 << depth;
  size_t x_out_stride = 2 * x_stride;


  int lr_stride = x_stride;
  int tb_stride = x_stride;
#endif

  size_t max_width = width >> (depth + 1);
  size_t max_height = height >> (depth + 1);
  //if(get_global_id(0) == 0 && get_global_id(1) == 0) printf("%d,%d  %d,%d  %d,%d %d,%d,%d,%d\n", COMB_PAIRS_PER_GROUP_X, COMB_PAIRS_PER_GROUP_Y, COMB_PAIRS_PER_THREAD_X, COMB_PAIRS_PER_THREAD_Y, (int)get_local_size(0), (int)get_local_size(1), BORDER_LINES_L, BORDER_LINES_R, BORDER_LINES_B, BORDER_LINES_T);
  int grx_size = (COMB_PAIRS_PER_GROUP_X - COMB_BLOCK_ALL_BORDER_PAIRS * 2);
  int gry_size = (COMB_PAIRS_PER_GROUP_Y - COMB_BLOCK_ALL_BORDER_PAIRS * 2);
  int grx_start = grx_size * grx;
  int gry_start = gry_size * gry;
  int gx = grx_start + (lx - COMB_BLOCK_ALL_BORDER_PAIRS);
  int gy = gry_start + (ly - COMB_BLOCK_ALL_BORDER_PAIRS);

  size_t lxy = get_local_size(0) * COMB_PAIRS_PER_THREAD_X *  get_local_id(1) + get_local_id(0);

  __local float *l_image_pos = temp_image + lxy;

  float4 out_data[COMB_QUADS_PER_THREAD];
  #pragma unroll
  for(int j = 0; j < COMB_PAIRS_PER_THREAD_Y; j++)
  {
    #pragma unroll
    for(int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
    {
      out_data[j * COMB_PAIRS_PER_THREAD_X + i] = load_block(in_image, gx + i * COMB_BLOCK_SIZE_X, gy + j * COMB_BLOCK_SIZE_Y, x_stride, width * x_stride, max_width, max_height);
    }
  }

  proc_block(l_image_pos, out_data);
  
  #pragma unroll
  for(int j = 0; j < COMB_PAIRS_PER_THREAD_Y; j++)
  {
    #pragma unroll
    for(int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
    {
      int act_lx = lx + i * COMB_BLOCK_SIZE_X;
      int act_ly = ly + j * COMB_BLOCK_SIZE_Y;
      int act_gx = gx + i * COMB_BLOCK_SIZE_X;
      int act_gy = gy + j * COMB_BLOCK_SIZE_Y;
      if((act_lx >= COMB_BLOCK_ALL_BORDER_PAIRS) && (act_lx < COMB_PAIRS_PER_GROUP_X - COMB_BLOCK_ALL_BORDER_PAIRS) && (act_ly+i >= COMB_BLOCK_ALL_BORDER_PAIRS) && (act_ly < COMB_PAIRS_PER_GROUP_Y - COMB_BLOCK_ALL_BORDER_PAIRS) && (act_gx < max_width) && (act_gy < max_height))
      {
        save_float(out_image, (int2)(act_gx * x_out_stride, act_gy * x_out_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].x);
        save_float(out_image, (int2)(act_gx * x_out_stride + lr_stride, act_gy * x_out_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].y);
        save_float(out_image, (int2)(act_gx * x_out_stride, act_gy * x_out_stride + tb_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].z);
        save_float(out_image, (int2)(act_gx * x_out_stride + lr_stride, act_gy * x_out_stride + tb_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].w);
      }
    }
  }
}*/
__kernel void wavelet_block(OUT_IMAGE_MEM_TYPE(float) out_image, IN_IMAGE_MEM_TYPE(float) in_image, uint width, uint height, uint depth)
{
	size_t grx = get_group_id(0);
	size_t gry = get_group_id(1);
	size_t lx = get_local_id(0) * COMB_PAIRS_PER_THREAD_X;
	size_t ly = get_local_id(1) * COMB_PAIRS_PER_THREAD_Y;

	__local float temp_image2[KERNEL_LOCAL_SIZE];
	__local float *temp_image = temp_image2 + LOCAL_LEFT_BORDER;
#ifndef WAVELET_OUTPUT_INTERLACED
	size_t x_stride = 1;
	size_t x_out_stride = 1;

	int lr_stride = width >> (1 + depth);
	int tb_stride = height >> (1 + depth);
#else
	size_t x_stride = 1 << depth;
	size_t x_out_stride = 2 * x_stride;


	int lr_stride = x_stride;
	int tb_stride = x_stride;
#endif

	size_t max_width = width >> (depth + 1);
	size_t max_height = height >> (depth + 1);
	
	int grx_size = (COMB_PAIRS_PER_GROUP_X - COMB_BLOCK_ALL_BORDER_PAIRS * 2);
	int gry_size = (COMB_PAIRS_PER_GROUP_Y - COMB_BLOCK_ALL_BORDER_PAIRS * 2);
	int grx_start = grx_size * grx;
	int gry_start = gry_size * gry;
	int gx = grx_start + (lx - COMB_BLOCK_ALL_BORDER_PAIRS);
	int gy = gry_start + (ly - COMB_BLOCK_ALL_BORDER_PAIRS);

	//if (get_global_id(0) == 32 && get_global_id(1) == 8) printf("%d,%d  %d,%d  %d,%d %d,%d,%d,%d %d,%d %d,%d %d,%d %d %d %d,%d %d %d,%d\n", COMB_PAIRS_PER_GROUP_X, COMB_PAIRS_PER_GROUP_Y, COMB_PAIRS_PER_THREAD_X, COMB_PAIRS_PER_THREAD_Y, (int)get_local_size(0), (int)get_local_size(1), BORDER_LINES_L, BORDER_LINES_R, BORDER_LINES_B, BORDER_LINES_T, grx_size, gry_size, grx_start, gry_start, gx, gy, KERNEL_LOCAL_SIZE, depth, width, height, COMB_BLOCK_ALL_BORDER_PAIRS, (int)get_global_size(0), (int)get_global_size(1));

	size_t lxy = get_local_size(0) * get_local_id(1) + get_local_id(0);

	__local float *l_image_pos = temp_image + lxy;

	float4 out_data[COMB_QUADS_PER_THREAD];
#pragma unroll
	for (int j = 0; j < COMB_PAIRS_PER_THREAD_Y; j++)
	{
#pragma unroll
		for (int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
		{
			out_data[j * COMB_PAIRS_PER_THREAD_X + i] = load_block(in_image, gx + i, gy + j, x_stride, width * x_stride, max_width, max_height);
		}
	}

	proc_block(l_image_pos, out_data);

#pragma unroll
	for (int j = 0; j < COMB_PAIRS_PER_THREAD_Y; j++)
	{
#pragma unroll
		for (int i = 0; i < COMB_PAIRS_PER_THREAD_X; i++)
		{
			int act_lx = lx + i;
			int act_ly = ly + j;
			int act_gx = gx + i;
			int act_gy = gy + j;
			if ((act_lx >= COMB_BLOCK_ALL_BORDER_PAIRS) && (act_lx < COMB_PAIRS_PER_GROUP_X - COMB_BLOCK_ALL_BORDER_PAIRS) && (act_ly >= COMB_BLOCK_ALL_BORDER_PAIRS) && (act_ly < COMB_PAIRS_PER_GROUP_Y - COMB_BLOCK_ALL_BORDER_PAIRS) && (act_gx < max_width) && (act_gy < max_height))
			{
				save_float(out_image, (int2)(act_gx * x_out_stride, act_gy * x_out_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].x);
				save_float(out_image, (int2)(act_gx * x_out_stride + lr_stride, act_gy * x_out_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].y);
				save_float(out_image, (int2)(act_gx * x_out_stride, act_gy * x_out_stride + tb_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].z);
				save_float(out_image, (int2)(act_gx * x_out_stride + lr_stride, act_gy * x_out_stride + tb_stride), width, out_data[j * COMB_PAIRS_PER_THREAD_X + i].w);/**/
			}
		}
	}
}
#endif
#endif

