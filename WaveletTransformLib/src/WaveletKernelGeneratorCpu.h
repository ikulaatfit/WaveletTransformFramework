#ifndef WaveletKernelGeneratorCpu_H
#define WaveletKernelGeneratorCpu_H
#include "WaveletKernelGenerator.h"
#include "WaveletKernelGeneratorStepCpu.h"

template <e_intrinsics intrinsics, bool fma_enabled>
class WaveletKernelGeneratorCpu: public WaveletKernelGenerator
{
public:
	WaveletKernelGeneratorCpu(e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool optim_thread, std::array<int, 2> pairs_per_thread);
	~WaveletKernelGeneratorCpu() override;
	std::string getBody() override;

protected:

	std::string getBodyPrefix();
	void appendStep() override;
	bool isHorOpAtomic() override;
	std::string getScalarFunctions();
	void appendUpdateLocalBarrierBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef, bool start_barrier, bool end_barrier) override;
};


template <e_intrinsics intrinsics, bool fma_enabled>
void WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::appendUpdateLocalBarrierBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef, bool start_barrier, bool end_barrier)
{
	if ((proc_mask[0] == 255) &&
		(proc_mask[1] == 255) &&
		(proc_mask[2] == 255) &&
		(proc_mask[3] == 255)) return;
	if ((reinterpret_cast<WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *>(&*this->steps.back()))->output_args.size() != 0)
	{
		//body << this->steps.back()->getBody();
		this->appendStep();
	}
}

template <e_intrinsics intrinsics, bool fma_enabled>
WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::WaveletKernelGeneratorCpu(e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool optim_thread, std::array<int, 2> pairs_per_thread) : WaveletKernelGenerator(kernel_type, wavelet_info, true, optim_thread, pairs_per_thread)
{
}

template <e_intrinsics intrinsics, bool fma_enabled>
WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::~WaveletKernelGeneratorCpu()
{
}

template <e_intrinsics intrinsics, bool fma_enabled>
std::string WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::getScalarFunctions()
{
	return std::string() +
		"\nstatic inline float _scalar_set1_ps(float val)\n" +
		"{\n" +
		"return val;\n" +
		"}\n" +
		"\nstatic inline float _scalar_add_ps(float a, float b)\n" +
		"{\n" +
		"return a + b;\n" +
		"}\n" +
		"\nstatic inline float _scalar_mul_ps(float a, float b)\n" +
		"{\n" +
		"return a * b;\n" +
		"}\n" +
		"\nstatic inline float _scalar_fmadd_ps(float a, float b, float c)\n" +
		"{\n" +
		"return a * b + c;\n" +
		"}\n";
}

template <e_intrinsics intrinsics, bool fma_enabled>
inline std::string WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::getBodyPrefix()
{
	return std::string();
}


template <>
inline std::string WaveletKernelGeneratorCpu<INTRINSICS_NONE, true>::getBodyPrefix()
{
	return getScalarFunctions();
}

template <>
inline std::string WaveletKernelGeneratorCpu<INTRINSICS_NONE, false>::getBodyPrefix()
{
	return getScalarFunctions();
}

template <e_intrinsics intrinsics, bool fma_enabled>
std::string WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::getBody()
{
	if (this->kernel_type == WAVELET_KERNEL_OTHER) return std::string();

	this->appendStep();
	this->getFilterBody();
	this->findBorders();
	

	std::ostringstream out_data;
	out_data << getBodyPrefix();
	for(int i = 0; i < steps.size(); i++)
	{
		out_data << steps[i]->getBody();
	}
	return out_data.str();
}

template <e_intrinsics intrinsics, bool fma_enabled>
void WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::appendStep()
{
	
	std::vector<std::array<unsigned int, 4>> memory_locations;
	bool write_all_subbands = false;
	if(steps.size() == 0)
	{
		write_all_subbands = true;
#ifdef WRITE_ALL_OUTPUTS_IN_STEP_1
		memory_locations = std::vector<std::array<unsigned int, 4>>(pairs_per_thread[0] * pairs_per_thread[1], { std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max() });
#else
		memory_locations = std::vector<std::array<unsigned int, 4>>(pairs_per_thread[0] * pairs_per_thread[1], {0,1,2,3});
#endif
	}
	else
	{
		if (((WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*steps.back()))->isEmpty()) return;
		memory_locations = ((WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled> *)(&*steps.back()))->output_memory_locations;
	}
	steps.push_back(std::shared_ptr<WaveletKernelGeneratorStep>(new WaveletKernelGeneratorStepCpu<intrinsics, fma_enabled>(this->wavelet_info, pairs_per_thread, optim_thread, memory_locations, "K" + std::to_string(steps.size()), NULL, write_all_subbands)));
}
;
template <e_intrinsics intrinsics, bool fma_enabled>
bool WaveletKernelGeneratorCpu<intrinsics, fma_enabled>::isHorOpAtomic()
{
	return false;
}

#endif
