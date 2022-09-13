#include "WaveletKernelGeneratorGpu.h"

WaveletKernelGeneratorGpu::WaveletKernelGeneratorGpu(std::array<int, 2> comb_sizes, e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool double_buffering, bool optim_thread, wavelet_optim_warp optim_warp, unsigned int warp_size, std::array<int, 2> pairs_per_thread) : WaveletKernelGenerator(kernel_type, wavelet_info, double_buffering, optim_thread, pairs_per_thread)
{
	this->comb_sizes = comb_sizes;
	this->simd_size = warp_size;
	this->optim_warp = optim_warp;
}

WaveletKernelGeneratorGpu::~WaveletKernelGeneratorGpu()
{
	
}

bool WaveletKernelGeneratorGpu::isHorOpAtomic()
{
	return (this->simd_size % (this->comb_sizes[0] * this->pairs_per_thread[0]) == 0);
}

std::string WaveletKernelGeneratorGpu::getBody()
{
	if ((!this->isHorOpAtomic()) && (this->optim_warp != WAVELET_OPTIM_WARP_NONE))
	{
		this->optim_warp = WAVELET_OPTIM_WARP_NONE;
		fprintf(stderr, "Warp size is smaller than number of threads in row. Setting warp optimization to disabled.");
	}
	std::ostringstream str("");
	if (this->kernel_type != WAVELET_KERNEL_OTHER)
	{
		this->appendStep();
		str << this->getFilterBody();
		this->findBorders();
	}
	return str.str();
}

void WaveletKernelGeneratorGpu::appendStep()
{
	steps.push_back(std::shared_ptr<WaveletKernelGeneratorStep>(new WaveletKernelGeneratorStepGpu(this->comb_sizes, this->wavelet_info, pairs_per_thread, optim_thread, optim_warp, true, "k_" + std::to_string(steps.size()))));
}

void WaveletKernelGeneratorGpu::appendBarrierBody(std::ostream &body)
{
	body << "barrier(CLK_LOCAL_MEM_FENCE);\n";
	this->barrier_count++;
}

void WaveletKernelGeneratorGpu::appendLocalBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask)
{
	for (int y_thr = 0; y_thr < this->pairs_per_thread[1]; y_thr++)
	{
		for (int x_thr = 0; x_thr < this->pairs_per_thread[0]; x_thr++)
		{
			int dst_reg_id = y_thr * this->pairs_per_thread[0] + x_thr;
			for (int k = 0; k < 4; k++)
			{
				if (proc_mask[k] != 255)
				{
					int comb_sizes_count = comb_sizes[0] * comb_sizes[1];
					int pair_per_thread_count = pairs_per_thread[0] * pairs_per_thread[1];
					body << "temp_image[" << ((unsigned int)(coef_local_id[proc_mask[k]][k]) * comb_sizes_count * pair_per_thread_count + dst_reg_id * comb_sizes_count) << "] = act_data[" << dst_reg_id << "].s" << k << ";\n";
					this->store_count++;;
				}
			}
		}
	}
}

void WaveletKernelGeneratorGpu::updateActCoef(std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef)
{
	for (int i = 0; i < 4; i++)
	{
		if (proc_mask[i] != 255) act_coef[i] = coef_local_id[proc_mask[i]][i];
	}
}

void WaveletKernelGeneratorGpu::appendUpdateLocalBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef)
{
	appendLocalBody(body, coef_local_id, proc_mask);
	updateActCoef(coef_local_id, proc_mask, act_coef);
}

void WaveletKernelGeneratorGpu::appendUpdateLocalBarrierBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef, bool start_barrier, bool end_barrier)
{
	if ((proc_mask[0] == 255) &&
		(proc_mask[1] == 255) &&
		(proc_mask[2] == 255) &&
		(proc_mask[3] == 255)) return;
	body << this->steps.back()->getBody();
	this->appendStep();
	if (start_barrier) this->appendBarrierBody(body);
	appendUpdateLocalBody(body, coef_local_id, proc_mask, act_coef);
	if (end_barrier) this->appendBarrierBody(body);
#if DEBUG_LEVEL == DEBUG_ALL
	if (start_barrier || end_barrier) this->step_borders.add_step(proc_mask, act_coef);
#endif
}