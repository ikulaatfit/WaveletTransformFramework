#ifndef WaveletKernelGeneratorGpu_H
#define WaveletKernelGeneratorGpu_H
#include "WaveletKernelGenerator.h"
#include "WaveletKernelGeneratorStepGpu.h"

class WaveletKernelGeneratorGpu: public WaveletKernelGenerator
{
public:
	WaveletKernelGeneratorGpu(std::array<int, 2> comb_sizes, e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool double_buffering, bool optim_thread, wavelet_optim_warp optim_warp, unsigned int warp_size, std::array<int, 2> pairs_per_thread);
	~WaveletKernelGeneratorGpu() override;
	std::string getBody() override;

protected:
	std::array<int, 2> comb_sizes;
	unsigned int simd_size;
	void appendStep() override;
	bool isHorOpAtomic() override;
	void appendLocalBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask);
	void appendBarrierBody(std::ostream &body);
	void updateActCoef(std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef);
	void appendUpdateLocalBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef);
	void appendUpdateLocalBarrierBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef, bool start_barrier, bool end_barrier) override;
private:
	
};

#endif
