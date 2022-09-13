#include "WaveletKernelGenerator.h"
#include <map>
#include <cfloat>

std::array<unsigned char, 4> polyphase_sep_double_hor[] = { {0,2,1,3} };
std::array<unsigned char, 4> polyphase_sep_double_vert[] = { {0,1,2,3} };
std::array<unsigned char, 4> polyphase_sep_single_hor[] = { {0,0,1,1} };
std::array<unsigned char, 4> polyphase_sep_single_vert[] = { {0,1,0,1} };

std::array<unsigned char, 4> polyphase_sep_double[] = { {0,1,2,3},{4,5,6,7} };
std::array<unsigned char, 4> polyphase_sep_single[] = { {0,1,2,3},{0,1,2,3} };
std::array<unsigned char, 4> polyphase_sep_single1[] = { { 0,1,2,3},{0,1,2,3} };



WaveletKernelGenerator::WaveletKernelGenerator(e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool double_buffering, bool optim_thread, std::array<int, 2> pairs_per_thread)
{
	this->kernel_type = kernel_type;
	this->pairs_per_thread = pairs_per_thread;
	this->double_buffering = double_buffering;
	this->optim_thread = (kernel_type == WAVELET_KERNEL_SWELDENS) ? false : optim_thread;
	this->optim_warp = WAVELET_OPTIM_WARP_NONE;
	this->op_count = 0;
	this->load_count = 0;
	this->store_count = 0;
	this->shuffle_count = 0;
	this->barrier_count = 0;
	this->wavelet_info = wavelet_info;
	this->read_once = true;
}


void WaveletKernelGenerator::appendRegDeclareBody(std::ostream &body, std::string type, std::string name)
{
	//body << "DECLARE_VAR(" << type << "," << name << ");\n";
	std::string declare_var = "DECLARE_VAR(" + type + "," + name + ");\n";
	steps.back()->appendToSubstepBody(declare_var);
}

void WaveletKernelGenerator::appendRegVectDeclareBody(std::ostream &body, std::string type, std::string name, size_t vect_size = 0)
{
	if (vect_size == 0) vect_size = this->pairs_per_thread[0] * this->pairs_per_thread[1];
	//body << "DECLARE_VAR(" << type << "," << name << "[" << vect_size << "]);\n";
	std::string declare_var = "DECLARE_VAR(" + type + "," + name + "[" + std::to_string(vect_size) + "]);\n";
	steps.back()->appendToSubstepBody(declare_var);
}

void WaveletKernelGenerator::appendRegSetBody(std::ostream &body, std::string name, std::string value)
{
	for (int j = 0; j < this->pairs_per_thread[1]; j++)
	{
		for (int i = 0; i < this->pairs_per_thread[0]; i++)
		{
			int id = j * this->pairs_per_thread[0] + i;
			std::string out = name + "[" + std::to_string(id) + "]";
			std::string in = value + "[" + std::to_string(id) + "]";
			std::string assign_text = steps.back()->set(out, in);
			steps.back()->appendToSubstepBody(assign_text);
		}
	}
}




bool WaveletKernelGenerator::isRegGenNeeded(const std::vector<float> &fir_l, const std::vector<float> &fir_h, bool is_hor_used, bool is_vert_used) const
{
	int act_size = 1;
	if (is_hor_used) act_size = std::max(act_size, this->pairs_per_thread[0]);
	if (is_vert_used) act_size = std::max(act_size, this->pairs_per_thread[1]);
	for(int i = 1-act_size; i < act_size; i++)
	{
		int l_coef_pos = (fir_l.size() >> 1) + (fir_l.size() >> 2) + i;
		int h_coef_pos = (fir_h.size() >> 2) + i;

		if(l_coef_pos >= 0 && l_coef_pos < fir_l.size() && (fir_l[l_coef_pos] != 0.0f)) return true;
		if(h_coef_pos >= 0 && h_coef_pos < fir_h.size() && (fir_h[h_coef_pos] != 0.0f)) return true;
	}
	return false;
}

bool WaveletKernelGenerator::isRegGenNeeded(const std::vector<float> &fir_ll, const std::vector<float> &fir_hl, const std::vector<float> &fir_lh, const std::vector<float> &fir_hh, unsigned int subband_row_size) const
{
	int half_subband_row = subband_row_size << 1;
	int act_size = std::max(this->pairs_per_thread[0], this->pairs_per_thread[1]);
	const std::vector<float> *fir[4] = { &fir_ll, &fir_hl, &fir_lh, &fir_hh };
	for (int l = 0; l < 4; l++)
	{
		for (int m = 0; m < 4; m++)
		{
			for (int x = 1 - act_size; x < act_size; x++)
			{
				for (int y = 1 - act_size; y < act_size; y++)
				{
					if(((x >= -half_subband_row) && (y <= half_subband_row)) &&
						((m != l) || (x != 0) || y != 0) &&
						 ((*fir[m])[((int)(((*fir[m]).size() >> 2)*l + ((*fir[m]).size() >> 3))) + x + y * subband_row_size] != 0.0f)) return true;
				}
			}
		}
	}
	return false;
}

bool WaveletKernelGenerator::isRegTempGenNeeded(const std::vector<float> &fir_l, const std::vector<float> &fir_h) const
{
	if (!this->read_once) return false;
	for (int i = 0; i < fir_l.size(); i++)
	{
		if ((i == (fir_l.size() >> 1) + (fir_l.size() >> 2)) && (i == (fir_l.size() >> 2))) continue;
		if ((fir_l[i] != 0.0f) && (fir_h[i] != 0.0f)) return true;
	}
	return false;
}


std::array<unsigned char, 4> sweldens_double[] = { {0,2,1,1},{2,1,0,0},{1,0,2,2} };
std::array<unsigned char, 4> sweldens_single[] = { {0,1,1,0},{0,1,1,0},{0,1,1,0} };
std::array<unsigned char, 4> sweldens_double_local[] = { {0,3,1,2},{0,3,1,2},{0,3,1,2} };
std::array<unsigned char, 4> sweldens_single_local[] = { {0,2,1,1},{2,1,0,0},{1,0,2,2} };
std::array<unsigned char, 4> sweldens_mask[] = { {0,255,255,255},{255,0,255,255},{255,255,255,0},{255,255,1,255},
								   {1,255,255,255},{255,1,255,255},{255,255,255,1},{255,255,2,255},
								   {2,255,255,255},{255,2,255,255},{255,255,255,2},{255,255,0,255} };
std::array<unsigned char, 4> sweldens_mask_init = { 255,255,0,255 };

std::array<unsigned char, 4> sweldens_double_shuffle[] = { {0,1,2,3} };
std::array<unsigned char, 4> sweldens_single_shuffle[] = { {0,1,0,1} };
std::array<unsigned char, 4> sweldens_mask_shuffle[] = { {255,255,255,255},{0,0,255,255},{255,255,255,255},{255,255,0,0},
										  {255,255,255,255},{0,0,255,255},{255,255,255,255},{255,255,0,0},
										  {255,255,255,255},{0,0,255,255},{255,255,255,255},{255,255,0,0} };
std::array<unsigned char, 4> sweldens_mask_shuffle_init = empty_coef;

void WaveletKernelGenerator::getSweldensBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init;
	if (this->optim_warp == WAVELET_OPTIM_WARP_SHUFFLE)
	{
		coef_local_id = (double_buffering) ? polyphase_sep_double_vert : polyphase_sep_single_vert;
		mask = sweldens_mask_shuffle;
		mask_init = sweldens_mask_shuffle_init;
	}
	else
	{
		if (this->optim_warp == WAVELET_OPTIM_WARP_LOCAL) coef_local_id = (double_buffering) ? sweldens_double_local : sweldens_single_local;
		else coef_local_id = (double_buffering) ? sweldens_double : sweldens_single;
		mask = sweldens_mask;
		mask_init = sweldens_mask_init;
	}

	std::array<unsigned char, 4> act_coef = empty_coef;

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[0 + (i % 3) * 4], act_coef, (i != 0) && (!double_buffering) && (this->optim_warp != WAVELET_OPTIM_WARP_LOCAL), (this->optim_warp != WAVELET_OPTIM_WARP_LOCAL));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[1 + (i % 3) * 4], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[2 + (i % 3) * 4], act_coef, (!double_buffering) && (this->optim_warp != WAVELET_OPTIM_WARP_LOCAL), (this->optim_warp != WAVELET_OPTIM_WARP_LOCAL));

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[3 + (i % 3) * 4], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
	}
	this->steps.back()->appendSubstepNormBody();
}


std::array<unsigned char, 4> explosive_double[] = { {0,1,2,0},{1,0,2,1} };
std::array<unsigned char, 4> explosive_single[] = { {0,0,1,1},{0,0,1,1} };
std::array<unsigned char, 4> explosive_mask[] = { {0,255,255,255},{255,0,0,255},{255,255,255,0},{1,255,255,255},{255,1,1,255},{255,255,255,1} };

void WaveletKernelGenerator::getExplosiveBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? explosive_double : explosive_single;
	std::array<unsigned char, 4> *mask = explosive_mask;
	std::array<unsigned char, 4> act_coef = empty_coef;

	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		if (optim_thread) this->steps.back()->appendSubstepsImprovedPredictBody(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[0 + (i % 2) * 3], act_coef, (i != 0) && (!double_buffering));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubstep2DBody(3, act_coef, act_predict_coef, '-');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[1 + (i % 2) * 3], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[2 + (i % 2) * 3], act_coef, !double_buffering);

		this->steps.back()->appendSubstep2DBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		if (optim_thread) this->steps.back()->appendSubstepsImprovedUpdateBody(i);
	}
	this->steps.back()->appendSubstepNormBody();
}

std::array<unsigned char, 4> iwahashi_double[] = { {0,1,2,3},{0,1,2,3} };
std::array<unsigned char, 4> iwahashi_single[] = { {1,0,2,2},{2,0,1,1} };
std::array<unsigned char, 4> iwahashi_double_improved[] = { {0,1,2,255},{255,3,4,5} };
std::array<unsigned char, 4> iwahashi_double_improved_1[] = { {0,1,2,255},{255,1,2,3} };
std::array<unsigned char, 4> iwahashi_single_improved[] = { {0,1,2,255},{255,0,1,2} };

std::array<unsigned char, 4> iwahashi_mask_improved[] = { {0, 0, 0, 255}, {255, 255, 255, 1}, {255, 1, 1, 255}, {0, 0, 0, 255}, {255, 255, 255, 1}, {255, 1, 1, 255} };
std::array<unsigned char, 4> iwahashi_mask_basic[] = { {0, 255, 255, 255}, {255, 255, 255, 0}, {255, 1, 1, 255}, {1, 255, 255, 255}, {255, 255, 255, 1}, {255, 0, 0, 255} };
std::array<unsigned char, 4> iwahashi_mask_init_improved = { 255, 255, 255, 255 };
std::array<unsigned char, 4> iwahashi_mask_init_basic = { 255, 0, 0, 255 };

std::array<unsigned char, 4> iwahashi_single_shuffle[] = { {0, 1, 0, 1}, {0, 1, 0, 1} };
std::array<unsigned char, 4> iwahashi_double_shuffle[] = { {0, 1, 2, 3}, {0, 1, 2, 3} };
std::array<unsigned char, 4> iwahashi_double_shuffle_1[] = { {0, 1, 1, 2} };
std::array<unsigned char, 4> iwahashi_mask_init_shuffle = { 255, 255, 255, 255 };
std::array<unsigned char, 4> iwahashi_mask_shuffle[] = { {0, 0, 255, 255}, {255, 255, 255, 0}, {255, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 0}, {255, 255, 0, 255} };

void WaveletKernelGenerator::getIwahashiBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init;
	if (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE)
	{
		coef_local_id = (double_buffering) ? ((wavelet_info->steps == 1) ? iwahashi_double_shuffle_1 : iwahashi_double_shuffle) : iwahashi_single_shuffle;
		mask_init = iwahashi_mask_init_shuffle;
		mask = iwahashi_mask_shuffle;
	}
	else
	{
		if (optim_thread)
		{
			coef_local_id = (double_buffering) ? ((wavelet_info->steps == 1) ? iwahashi_double_improved_1 : iwahashi_double_improved) : iwahashi_single_improved;
			mask_init = iwahashi_mask_init_improved;
			mask = iwahashi_mask_improved;
		}
		else
		{
			coef_local_id = (double_buffering) ? iwahashi_double : iwahashi_single;
			mask_init = iwahashi_mask_init_basic;
			mask = iwahashi_mask_basic;
		}
	}
	std::array<unsigned char, 4> act_coef = empty_coef;

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		if (optim_thread) this->steps.back()->appendSubstepsImprovedPredictBody(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[0 + (i % 2) * 3], act_coef, (i != 0) && (!double_buffering));

		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubstep2DBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[1 + (i % 2) * 3], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[2 + (i % 2) * 3], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubstep2DBody(0, act_coef, act_update_coef, '-');

		if (optim_thread) this->steps.back()->appendSubstepsImprovedUpdateBody(i);
	}
	this->steps.back()->appendSubstepNormBody();
}

std::array<unsigned char, 4> convolution_double[] = { {0,1,2,3} };
std::array<unsigned char, 4> convolution_single[] = { {0,1,2,3} };

std::array<unsigned char, 4> convolution_mask[] = { {0,0,0,0} };

void WaveletKernelGenerator::getConvolutionBody(std::ostream &stream)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? convolution_double : convolution_single;
	std::array<unsigned char, 4> act_coef = empty_coef;
	
	getFirStepBody(stream, wavelet_info->steps, 0, coef_local_id, convolution_mask, act_coef);
	if (optim_thread) this->steps.back()->appendSubstepNormBody();
}

std::array<unsigned char, 4> polyphase_double[] = { {0,1,2,3},{4,5,6,7} };
std::array<unsigned char, 4> polyphase_single[] = { {0,1,2,3},{0,1,2,3} };

std::array<unsigned char, 4> polyphase_mask[] = { {0,0,0,0},{1,1,1,1} };

void WaveletKernelGenerator::getPolyphaseBody(std::ostream &stream)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? polyphase_double : polyphase_single;
	std::array<unsigned char, 4> act_coef = empty_coef;
	
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		getFirStepBody(stream, 1, i, coef_local_id, polyphase_mask, act_coef);
	}
	if (optim_thread) this->steps.back()->appendSubstepNormBody();
}

void WaveletKernelGenerator::getFirStepBody(std::ostream &body, unsigned int stages, unsigned int iter_id, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> *mask, std::array<unsigned char, 4> &act_coef)
{
	std::vector<float> fir_ll;
	std::vector<float> fir_hl;
	std::vector<float> fir_lh;
	std::vector<float> fir_hh;

	unsigned int subband_row_size;

	getFirData2D(stages, iter_id, fir_ll, fir_hl, fir_lh, fir_hh, subband_row_size);

	int half_filter_size = stages * (2 * wavelet_info->width - 1);
	
	
	//body << "{\n";
	std::string start_block("{\n");
	steps.back()->appendToSubstepBody(start_block);
	appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);

	// calculation of predict for per-thread values
	if (optim_thread) this->steps.back()->appendSubstepsImprovedPredictBody(iter_id * stages);

	// copy data to shared memory

	appendUpdateLocalBarrierBody(body, coef_local_id, mask[iter_id], act_coef, (iter_id != 0) && (!double_buffering));

	// check necessity of temporary buffer
	bool gen_needed = this->isRegGenNeeded(fir_ll, fir_hl, fir_lh, fir_hh, subband_row_size) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

	// calculate per-thread values
	std::string reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;
	if (gen_needed)
	{
		appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);
		appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);
	}
	this->steps.back()->appendSubstep2DGenBody(half_filter_size, act_coef, fir_ll, fir_hl, fir_lh, fir_hh, reg_name_f4, temp_reg_name_f1);

	//body << "\n";
	std::string new_line("\n");
	steps.back()->appendToSubstepBody(new_line);
	// calculation of update for per-thread values
	if (optim_thread) this->steps.back()->appendSubstepsImprovedUpdateBody(iter_id * stages + stages - 1);
	//body << "}\n";
	std::string end_block("}\n");
	steps.back()->appendToSubstepBody(end_block);
}


// none lifting - done all variants
std::array<unsigned char, 4> convolution_sep_none_double[] = { {0, 1, 2, 3}, {4, 5, 6, 7}, {4, 5, 6, 7} };
std::array<unsigned char, 4> convolution_sep_none_single[] = { {0, 1, 2, 3}, {0, 1, 2, 3}, {4, 5, 6, 7} };

std::array<unsigned char, 4> convolution_sep_none_mask[] = { {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef, {1, 1, 1, 1}, empty_coef, empty_coef, empty_coef,
										 {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef, {1, 1, 1, 1}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_none_mask_local[] = { {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef, {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef,
											   {2, 2, 2, 2}, empty_coef, empty_coef, empty_coef, {2, 2, 2, 2}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_none_mask_shuffle[] = { empty_coef, empty_coef, empty_coef, empty_coef, {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef,
												 empty_coef, empty_coef, empty_coef, empty_coef, {1, 1, 1, 1}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_none_mask_init = empty_coef;

void WaveletKernelGenerator::getConvolutionSepNoneBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? convolution_sep_none_double : convolution_sep_none_single;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init = convolution_sep_none_mask_init;

	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_none_mask_shuffle;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = convolution_sep_none_mask_local;
		break;
	case WAVELET_OPTIM_WARP_NONE:
		mask = convolution_sep_none_mask;
		break;
	}

	std::array<unsigned char, 4> act_coef = empty_coef;

	std::string reg_name_f4;

	std::vector<float> fir_l;
	std::vector<float> fir_h;

	getFirData1D(wavelet_info->steps, 0, fir_l, fir_h);

	if (this->isRegTempGenNeeded(fir_l, fir_h)) appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);

	bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, true, true)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

	reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;

	if (gen_needed) appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	// horizontal part
	if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyH(0);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask[0], act_coef, false, (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

	if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

	this->steps.back()->appendSubstep1DHorizontalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
	this->steps.back()->appendSubstep1DHorizontalGenBody(2, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

	if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyH(wavelet_info->steps - 1);

	// vertical part
	if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyV(0);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask[4], act_coef, !double_buffering);

	if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

	this->steps.back()->appendSubstep1DVerticalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
	this->steps.back()->appendSubstep1DVerticalGenBody(1, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

	if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyV(wavelet_info->steps - 1);

	this->steps.back()->appendSubstepNormBody();
}

void WaveletKernelGenerator::getPolyphaseSepNoneBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? convolution_sep_none_double : convolution_sep_none_single;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init = convolution_sep_none_mask_init;
	std::array<unsigned char, 4> act_coef = empty_coef;
	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_none_mask_shuffle;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = convolution_sep_none_mask_local;
		break;
	case WAVELET_OPTIM_WARP_NONE:
		mask = convolution_sep_none_mask;
		break;
	}

	std::string reg_name_f4;

	appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);
	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		std::vector<float> fir_l;
		std::vector<float> fir_h;

		getFirData1D(1, i, fir_l, fir_h);
		bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, true, true)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

		reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;

		// horizontal part
		if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyH(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 0], act_coef, (i != 0) && (!double_buffering) && (this->optim_warp == WAVELET_OPTIM_WARP_NONE), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

		this->steps.back()->appendSubstep1DHorizontalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
		this->steps.back()->appendSubstep1DHorizontalGenBody(2, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

		if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyH(i);

		// vertical part
		if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyV(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 4], act_coef, !double_buffering);

		if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

		this->steps.back()->appendSubstep1DVerticalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
		this->steps.back()->appendSubstep1DVerticalGenBody(1, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

		if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyV(i);
	}
	this->steps.back()->appendSubstepNormBody();
}


// horizontal lifting - done all variants
std::array<unsigned char, 4> convolution_sep_hor_double[] = { {0, 3, 1, 5}, {2, 255, 4, 1}, {0, 3, 5, 1}, {2, 255, 4, 5} };
std::array<unsigned char, 4> convolution_sep_hor_single[] = { {0, 2, 1, 3}, {0, 255, 1, 3}, {0, 2, 1, 3}, {0, 255, 1, 3} };
std::array<unsigned char, 4> convolution_sep_hor_double_1[] = { {0, 2, 1, 3}, {0, 255, 1, 4}, {0, 2, 1, 3}, {0, 255, 1, 4} };
std::array<unsigned char, 4> convolution_sep_hor_local_single[] = { {0, 0, 1, 1}, {2, 255, 3, 1}, {4, 4, 5, 5}, {2, 255, 3, 5} };
std::array<unsigned char, 4> convolution_sep_hor_local_double[] = { {0, 0, 1, 1}, {2, 255, 3, 1}, {4, 4, 5, 5}, {6, 255, 7, 5} };
std::array<unsigned char, 4> convolution_sep_hor_shuffle_double[] = { {0, 1, 2, 3}, {4, 5, 6, 7}, {4, 5, 6, 7} };
std::array<unsigned char, 4> convolution_sep_hor_shuffle_single[] = { {0, 1, 2, 3}, {0, 1, 2, 3}, {4, 5, 6, 7} };

std::array<unsigned char, 4> convolution_sep_hor_mask[] = { {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {1, 255, 1, 255}, empty_coef, empty_coef, empty_coef,
										{0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {1, 255, 1, 255}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_hor_mask_local[] = { {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {1, 255, 1, 255}, empty_coef, empty_coef, empty_coef,
											  {2, 255, 2, 255}, {255, 2, 255, 2}, {2, 255, 2, 255}, {255, 2, 255, 2}, {3, 255, 3, 255}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_hor_mask_shuffle[] = { empty_coef, empty_coef, empty_coef, empty_coef, {0, 0, 0, 0}, empty_coef, empty_coef, empty_coef,
												empty_coef, empty_coef, empty_coef, empty_coef, {1, 1, 1, 1}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_hor_mask_improved[] = { {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {1, 255, 1, 1}, empty_coef, empty_coef, empty_coef,
												 {2, 255, 2, 255}, {255, 2, 255, 2}, {2, 255, 2, 255}, {255, 2, 255, 2}, {3, 255, 3, 3}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_hor_mask_local_improved[] = { {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {1, 255, 1, 1}, empty_coef, empty_coef, empty_coef,
											  {2, 255, 2, 255}, {255, 2, 255, 2}, {2, 255, 2, 255}, {255, 2, 255, 2}, {3, 255, 3, 3}, empty_coef, empty_coef, empty_coef };
std::array<unsigned char, 4> convolution_sep_hor_mask_init = empty_coef;

void WaveletKernelGenerator::getConvolutionSepHorBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init = convolution_sep_hor_mask_init;

	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_NONE:
		mask = (this->optim_thread) ? convolution_sep_hor_mask_improved : convolution_sep_hor_mask;
		coef_local_id = (double_buffering) ? convolution_sep_hor_double_1 : convolution_sep_hor_single;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = (this->optim_thread) ? convolution_sep_hor_mask_local_improved : convolution_sep_hor_mask_local;
		coef_local_id = (double_buffering) ? convolution_sep_hor_local_double : convolution_sep_hor_local_single;
		break;
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_hor_mask_shuffle;
		coef_local_id = (double_buffering) ? convolution_sep_hor_shuffle_double : convolution_sep_hor_shuffle_single;
		break;
	}

	std::array<unsigned char, 4> act_coef = empty_coef;

	std::string reg_name_f4;

	std::vector<float> fir_l;
	std::vector<float> fir_h;

	getFirData1D(wavelet_info->steps, 0, fir_l, fir_h);
	if (this->isRegTempGenNeeded(fir_l, fir_h)) appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);
	bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, false, true)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);
	reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	// horizontal part
	bool optim_thread_backup = this->optim_thread;
	this->setOptimizeThread(false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == 0) ? 0 : 2], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (i != 0) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == wavelet_info->steps - 1) ? 3 : 1], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');
	}
	this->setOptimizeThread(optim_thread_backup);

	// vertical part
	if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyV(0);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask[4], act_coef, !double_buffering);

	if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

	this->steps.back()->appendSubstep1DVerticalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
	this->steps.back()->appendSubstep1DVerticalGenBody(1, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

	if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyV(wavelet_info->steps - 1);
	this->steps.back()->appendSubstepNormBody();
}

void WaveletKernelGenerator::getPolyphaseSepHorBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init = convolution_sep_hor_mask_init;
	std::array<unsigned char, 4> act_coef = empty_coef;

	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_hor_mask_shuffle;
		coef_local_id = (double_buffering) ? convolution_sep_hor_shuffle_double : convolution_sep_hor_shuffle_single;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = (this->optim_thread) ? convolution_sep_hor_mask_local_improved : convolution_sep_hor_mask_local;
		coef_local_id = (double_buffering) ? convolution_sep_hor_local_double : convolution_sep_hor_local_single;
		break;
	case WAVELET_OPTIM_WARP_NONE:
		mask = (this->optim_thread) ? convolution_sep_hor_mask_improved : convolution_sep_hor_mask;
		coef_local_id = (double_buffering) ? ((wavelet_info->steps == 1) ? convolution_sep_hor_double_1 : convolution_sep_hor_double) : convolution_sep_hor_single;
		break;
	}

	std::string reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);
	appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		std::vector<float> fir_l;
		std::vector<float> fir_h;

		getFirData1D(1, i, fir_l, fir_h);
		bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, false, true)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

		reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : temp_reg_name_f4;

		//horizontal part
		bool optim_thread_backup = this->optim_thread;
		this->setOptimizeThread(false);

		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		if (this->optim_warp != WAVELET_OPTIM_WARP_SHUFFLE) appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 0], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (i != 0) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');

		if (this->optim_warp != WAVELET_OPTIM_WARP_SHUFFLE) appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 3], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		this->setOptimizeThread(optim_thread_backup);

		//vertical part
		if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyV(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 4], act_coef, !double_buffering);

		if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

		this->steps.back()->appendSubstep1DVerticalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
		this->steps.back()->appendSubstep1DVerticalGenBody(1, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

		if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyV(i);
	}
	this->steps.back()->appendSubstepNormBody();
}


// vert lifting - done all variants
std::array<unsigned char, 4> convolution_sep_vert_double[] = { {0, 1, 255, 5}, {2, 3, 4, 5}, {0, 1, 255, 3}, {2, 5, 4, 3} };
std::array<unsigned char, 4> convolution_sep_vert_single[] = { {0, 1, 255, 3}, {0, 1, 2, 3}, {0, 1, 255, 3}, {0, 1, 2, 3} };
std::array<unsigned char, 4> convolution_sep_vert_local[] = { {0, 1, 255, 3}, {0, 1, 2, 3}, {0, 1, 255, 4}, {0, 1, 2, 4} };

std::array<unsigned char, 4> convolution_sep_vert_mask[] = { {0, 0, 255, 255}, empty_coef, empty_coef, empty_coef, {1, 1, 255, 255}, {255, 255, 1, 1}, {1, 1, 255, 255}, {255, 255, 1, 1},
										 {0, 0, 255, 255}, empty_coef, empty_coef, empty_coef, {1, 1, 255, 255}, {255, 255, 1, 1}, {1, 1, 255, 255}, {255, 255, 1, 1} };
std::array<unsigned char, 4> convolution_sep_vert_mask_improved[] = { {0, 0, 255, 0}, empty_coef, empty_coef, empty_coef, {1, 1, 255, 255}, {255, 255, 1, 1}, {1, 1, 255, 255}, {255, 255, 1, 1},
												  {2, 2, 255, 2}, empty_coef, empty_coef, empty_coef, {3, 3, 255, 255}, {255, 255, 3, 3}, {3, 3, 255, 255}, {255, 255, 3, 3} };

std::array<unsigned char, 4> convolution_sep_vert_mask_init = { 255, 255, 1, 1 };
std::array<unsigned char, 4> convolution_sep_vert_mask_init_improved = { 255, 255, 1, 255 };

void WaveletKernelGenerator::getConvolutionSepVertBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (this->optim_warp == WAVELET_OPTIM_WARP_LOCAL) ? convolution_sep_vert_local : ((double_buffering) ? convolution_sep_vert_double : convolution_sep_vert_single);
	std::array<unsigned char, 4> *mask = (this->optim_thread) ? convolution_sep_vert_mask_improved : convolution_sep_vert_mask;
	std::array<unsigned char, 4> mask_init = (this->optim_thread) ? convolution_sep_vert_mask_init_improved : convolution_sep_vert_mask_init;

	std::array<unsigned char, 4> act_coef = empty_coef;

	std::string reg_name_f4;

	std::vector<float> fir_l;
	std::vector<float> fir_h;

	getFirData1D(wavelet_info->steps, 0, fir_l, fir_h);
	if (this->isRegTempGenNeeded(fir_l, fir_h)) appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);
	bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, true, false)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);
	reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	// horizontal pass
	if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyH(0);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask[0], act_coef, false, (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

	if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

	this->steps.back()->appendSubstep1DHorizontalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
	this->steps.back()->appendSubstep1DHorizontalGenBody(2, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

	if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyH(wavelet_info->steps - 1);

	// vertical pass
	bool optim_thread_backup = this->optim_thread;
	this->setOptimizeThread(false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == 0) ? 4 : 6], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == wavelet_info->steps - 1) ? 7 : 5], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
	}
	this->setOptimizeThread(optim_thread_backup);
	this->steps.back()->appendSubstepNormBody();
}

void WaveletKernelGenerator::setOptimizeThread(bool optim_thread)
{
	this->optim_thread = optim_thread;
	if (this->steps.size() != 0) this->steps.back()->optim_thread = optim_thread;
}

void WaveletKernelGenerator::getPolyphaseSepVertBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (this->optim_warp == WAVELET_OPTIM_WARP_LOCAL) ? convolution_sep_vert_local : ((double_buffering) ? convolution_sep_vert_double : convolution_sep_vert_single);
	std::array<unsigned char, 4> *mask = (this->optim_thread) ? convolution_sep_vert_mask_improved : convolution_sep_vert_mask;
	std::array<unsigned char, 4> mask_init = (this->optim_thread) ? convolution_sep_vert_mask_init_improved : convolution_sep_vert_mask_init;
	std::array<unsigned char, 4> act_coef = empty_coef;

	std::string reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);
	appendRegDeclareBody(body, std::string("float"), temp_reg_name_f1);
	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		std::vector<float> fir_l;
		std::vector<float> fir_h;

		getFirData1D(1, i, fir_l, fir_h);
		bool gen_needed = (this->isRegGenNeeded(fir_l, fir_h, true, false)) || (optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

		reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : temp_reg_name_f4;

		//horizontal part
		if (optim_thread) this->steps.back()->appendSubstepImprovedPredictBodyH(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 0], act_coef, (i != 0) && (!double_buffering) && (this->optim_warp == WAVELET_OPTIM_WARP_NONE), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		if (gen_needed) appendRegSetBody(body, temp_reg_name_f4, orig_reg_name_f4);

		this->steps.back()->appendSubstep1DHorizontalGenBody(0, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);
		this->steps.back()->appendSubstep1DHorizontalGenBody(2, act_coef, fir_l, fir_h, reg_name_f4, temp_reg_name_f1);

		if (optim_thread) this->steps.back()->appendSubstepImprovedUpdateBodyH(i);

		//vertical part
		bool optim_thread_backup = this->optim_thread;
		this->setOptimizeThread(false);

		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 4], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 7], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');

		this->setOptimizeThread(optim_thread_backup);
	}
	this->steps.back()->appendSubstepNormBody();
}


//all lifting - done all variants
std::array<unsigned char, 4> convolution_sep_all_double[] = { {0, 1, 2, 3},{0,1,1,0},{0,1,2,3} };
std::array<unsigned char, 4> convolution_sep_all_single[] = { {0, 1, 1, 0},{0,1,1,0},{0,1,2,3} };
std::array<unsigned char, 4> convolution_sep_all_single_1[] = { {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0} };

std::array<unsigned char, 4> convolution_sep_all_mask[] = { {0, 255, 255, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 255, 255}, {255, 255, 0, 0}, {0, 0, 255, 255}, {255, 255, 0, 0},
										{0, 255, 255, 255}, {255, 0, 255, 0}, {0, 255, 0, 255}, {255, 0, 255, 0}, {0, 255, 255, 255}, {255, 255, 0, 0}, {0, 0, 255, 255}, {255, 255, 0, 0} };
std::array<unsigned char, 4> convolution_sep_all_mask_local[] = { {1, 255, 255, 255}, {255, 1, 255, 1}, {1, 255, 1, 255}, {255, 1, 255, 1}, {2, 255, 255, 255}, {255, 255, 2, 2}, {2, 2, 255, 255}, {255, 255, 2, 2},
											  {1, 255, 255, 255}, {255, 1, 255, 1}, {1, 255, 1, 255}, {255, 1, 255, 1}, {2, 255, 255, 255}, {255, 255, 2, 2}, {2, 2, 255, 255}, {255, 255, 2, 2} };
std::array<unsigned char, 4> convolution_sep_all_mask_shuffle[] = { {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 255, 255}, {255, 255, 0, 0}, {0, 0, 255, 255}, {255, 255, 0, 0},
												{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 255, 255}, {255, 255, 0, 0}, {0, 0, 255, 255}, {255, 255, 0, 0} };
std::array<unsigned char, 4> convolution_sep_all_mask_init = { 255, 255, 0, 255 };
std::array<unsigned char, 4> convolution_sep_all_mask_init_local = { 255, 255, 2, 255 };
std::array<unsigned char, 4> convolution_sep_all_mask_init_shuffle = { 255, 255, 255, 255 };

void WaveletKernelGenerator::getConvolutionSepAllBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? convolution_sep_all_double : convolution_sep_all_single_1;
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init;

	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_all_mask_shuffle;
		mask_init = convolution_sep_all_mask_init_shuffle;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = convolution_sep_all_mask_local;
		mask_init = convolution_sep_all_mask_init_local;
		break;
	case WAVELET_OPTIM_WARP_NONE:
		mask = convolution_sep_all_mask;
		mask_init = convolution_sep_all_mask_init;
		break;
	}

	std::array<unsigned char, 4> act_coef = empty_coef;

	std::string reg_name_f4;

	std::vector<float> fir_l;
	std::vector<float> fir_h;

	getFirData1D(wavelet_info->steps, 0, fir_l, fir_h);
	bool gen_needed = /*(this->isRegGenNeeded(fir_l, fir_h)) || */(optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);
	reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : orig_reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);

	// horizontal pass
	bool optim_thread_backup = this->optim_thread;
	this->setOptimizeThread(false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == 0) ? 0 : 2], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (i != 0) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == wavelet_info->steps - 1) ? 3 : 1], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');
	}

	// vertical pass
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == 0) ? 4 : 6], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i == wavelet_info->steps - 1) ? 7 : 5], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
	}
	this->setOptimizeThread(optim_thread_backup);
	this->steps.back()->appendSubstepNormBody();
	
	
}

void WaveletKernelGenerator::getPolyphaseSepAllBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id = (double_buffering) ? convolution_sep_all_double : ((wavelet_info->steps == 1) ? convolution_sep_all_single_1 : convolution_sep_all_single);
	std::array<unsigned char, 4> *mask;
	std::array<unsigned char, 4> mask_init;
	std::array<unsigned char, 4> act_coef = empty_coef;

	switch (this->optim_warp)
	{
	case WAVELET_OPTIM_WARP_SHUFFLE:
		mask = convolution_sep_all_mask_shuffle;
		mask_init = convolution_sep_all_mask_init_shuffle;
		break;
	case WAVELET_OPTIM_WARP_LOCAL:
		mask = convolution_sep_all_mask_local;
		mask_init = convolution_sep_all_mask_init_local;
		break;
	case WAVELET_OPTIM_WARP_NONE:
		mask = convolution_sep_all_mask;
		mask_init = convolution_sep_all_mask_init;
		break;
	}

	std::string reg_name_f4;

	appendRegVectDeclareBody(body, std::string("float4"), temp_reg_name_f4);

	appendUpdateLocalBarrierBody(body, coef_local_id, mask_init, act_coef, false, false);
	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		std::vector<float> fir_l;
		std::vector<float> fir_h;

		getFirData1D(1, i, fir_l, fir_h);
		bool gen_needed = /*(this->isRegGenNeeded(fir_l, fir_h)) || */(optim_warp == WAVELET_OPTIM_WARP_SHUFFLE);

		reg_name_f4 = (gen_needed) ? temp_reg_name_f4 : temp_reg_name_f4;

		//horizontal part
		bool optim_thread_backup = this->optim_thread;
		this->setOptimizeThread(false);

		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		if (this->optim_warp != WAVELET_OPTIM_WARP_SHUFFLE) appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 0], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (i != 0) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');

		if (this->optim_warp != WAVELET_OPTIM_WARP_SHUFFLE) appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 3], act_coef, (this->optim_warp == WAVELET_OPTIM_WARP_NONE) && (!double_buffering), (this->optim_warp == WAVELET_OPTIM_WARP_NONE));

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		//vertical part
		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 4], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[(i % 2) * 8 + 7], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');

		this->setOptimizeThread(optim_thread_backup);
	}
	this->steps.back()->appendSubstepNormBody();
}



std::array<unsigned char, 4> monolithic_double[] = { {0,1,2,255},{255,3,4,5} };
std::array<unsigned char, 4> monolithic_single[] = { {0,1,2,255},{255,0,1,2} };
std::array<unsigned char, 4> monolithic_mask[] = { {0,0,0,255},{255,1,1,1} };

std::array<unsigned char, 4> monolithic_double_shuffle[] = { {0,1,2,3} };
std::array<unsigned char, 4> monolithic_single_shuffle[] = { {0,1,0,1} };
std::array<unsigned char, 4> monolithic_mask_shuffle[] = { {0,0,255,255},{255,255,0,0} };

void WaveletKernelGenerator::getMonolithicBody(std::ostream &body)
{
	std::array<unsigned char, 4> *coef_local_id;
	std::array<unsigned char, 4> *mask;
	if (this->optim_warp != WAVELET_OPTIM_WARP_SHUFFLE)
	{
		coef_local_id = (double_buffering) ? monolithic_double : monolithic_single;
		mask = monolithic_mask;
	}
	else
	{
		coef_local_id = (double_buffering) ? monolithic_double_shuffle : monolithic_single_shuffle;
		mask = monolithic_mask_shuffle;
	}

	std::array<unsigned char, 4> act_coef = empty_coef;

	for (unsigned int i = 0; i < wavelet_info->steps; i++)
	{
		float *act_predict_coef = wavelet_info->coef + wavelet_info->width * 2 * i;
		float *act_update_coef = wavelet_info->coef + wavelet_info->width * (2 * i + 1);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[0], act_coef, (i != 0) && (!double_buffering));

		this->steps.back()->appendSubsteps1DHorizontalBody(3, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(3, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(1, act_coef, act_predict_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(2, act_coef, act_predict_coef, '+');

		this->steps.back()->appendSubstep2DBody(3, act_coef, act_predict_coef, '+');

		if (optim_thread) this->steps.back()->appendSubstepsImprovedPredictBody(i);

		appendUpdateLocalBarrierBody(body, coef_local_id, mask[1], act_coef, !double_buffering);

		this->steps.back()->appendSubsteps1DHorizontalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(0, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DVerticalBody(1, act_coef, act_update_coef, '+');
		this->steps.back()->appendSubsteps1DHorizontalBody(2, act_coef, act_update_coef, '+');

		this->steps.back()->appendSubstep2DBody(0, act_coef, act_update_coef, '+');

		if (optim_thread) this->steps.back()->appendSubstepsImprovedUpdateBody(i);
	}
	this->steps.back()->appendSubstepNormBody();
	
	
}


void WaveletKernelGenerator::createFirFilter1D(unsigned int stages, unsigned int iter_id, cv::Mat &fir_l_m, cv::Mat &fir_h_m, bool improved)
{
	if ((stages == 0) || (wavelet_info->width == 0)) return;
	float *coef = &this->wavelet_info->coef[iter_id * stages * 2 * wavelet_info->width];
	unsigned int half_filter_size = 2 * stages * (2 * wavelet_info->width - 1);
	unsigned int filter_size = (1 + 2 * half_filter_size);
	unsigned int matrix_size = filter_size + 1;
	unsigned int filter_start = wavelet_info->width * 2 - 1;

	cv::Mat fir_1D = cv::Mat::eye(matrix_size, matrix_size, CV_32F);

	//fprintf(stderr, "\nMatrix calculation info:\n");
	//fprintf(stderr,   " horizontal matrix info:\n");
	//std::cerr << fir_1D << std::endl;
	for (unsigned int j = filter_start, coef_id = 0; j <= half_filter_size; j += wavelet_info->width * 2 - 1, coef_id++)
	{
		cv::Mat fir_1D_stage = cv::Mat::eye(matrix_size, matrix_size, CV_32F);
		for (unsigned int i = j; i < filter_size - j; i += 2)
		{
			for (unsigned int k = 0; k < wavelet_info->width; k++)
			{
				if ((!improved) || (k != 0) || (j != half_filter_size)) {
					//if(((!improved) || (k != 0) || (j != half_filter_size)) && ((!improved) || j != filter_start)) {
					fir_1D_stage.at<float>(i + (k * 2 + 1), i) = coef[coef_id*wavelet_info->width + k];
				}
				if ((!improved) || (k != 0) || (j != filter_start)) {
					//if(((!improved) || (k != 0) || (j != filter_start)) && ((!improved) || j != half_filter_size)) {
					fir_1D_stage.at<float>(i - (k * 2 + 1), i) = coef[coef_id*wavelet_info->width + k];
				}

			}
		}
		//std::cerr << std::endl << fir_1D_stage << std::endl;
		fir_1D *= fir_1D_stage;
		//std::cerr << std::endl << fir_1D << std::endl;
	}
	fir_l_m = fir_1D.col(half_filter_size);
	fir_h_m = fir_1D.col(half_filter_size + 1);
	//std::cerr << std::endl << fir_l_m << std::endl;
	//std::cerr << std::endl << fir_h_m << std::endl;
}


void WaveletKernelGenerator::createFirFilter2D(unsigned int stages, unsigned int iter_id, cv::Mat &fir_ll_m, cv::Mat &fir_hl_m, cv::Mat &fir_lh_m, cv::Mat &fir_hh_m)
{
	if ((stages == 0) || (wavelet_info->width == 0)) return;
	unsigned int half_filter_size = 2 * stages * (2 * wavelet_info->width - 1);

	cv::Mat fir_1D;
	cv::Mat fir_1D2;

	createFirFilter1D(stages, iter_id, fir_1D, fir_1D2, optim_thread);

	fir_ll_m = fir_1D * fir_1D.t();
	fir_hl_m = fir_1D2 * fir_1D.t();
	fir_lh_m = fir_1D * fir_1D2.t();
	fir_hh_m = fir_1D2 * fir_1D2.t();

	//std::cerr << std::endl << fir_ll_m << std::endl;
	//std::cerr << std::endl << fir_hl_m << std::endl;
	//std::cerr << std::endl << fir_lh_m << std::endl;
	//std::cerr << std::endl << fir_hh_m << std::endl;

	// calculate processed elements
	int proc_count = 0;
	for (int j = 0; j < fir_ll_m.rows; j++)
	{
		for (int i = 0; i < fir_ll_m.cols; i++)
		{
			if ((fir_ll_m.at<float>(i, j) != 0.0f) && (i != half_filter_size || j != half_filter_size || fir_ll_m.at<float>(i, j) != 1.0f)) proc_count++;
			if ((fir_hl_m.at<float>(i, j) != 0.0f) && (i != half_filter_size + 1 || j != half_filter_size || fir_hl_m.at<float>(i, j) != 1.0f)) proc_count++;
			if ((fir_lh_m.at<float>(i, j) != 0.0f) && (i != half_filter_size || j != half_filter_size + 1 || fir_lh_m.at<float>(i, j) != 1.0f)) proc_count++;
			if ((fir_hh_m.at<float>(i, j) != 0.0f) && (i != half_filter_size + 1 || j != half_filter_size + 1 || fir_hh_m.at<float>(i, j) != 1.0f)) proc_count++;
		}
	}
	if (optim_thread) proc_count += 8;
	fprintf(stderr, "\n nonzero coef count: %d\n", proc_count);
}

void WaveletKernelGenerator::getFirData1D(unsigned int stages, unsigned int iter_id, std::vector<float> &fir_l, std::vector<float> &fir_h)
{
	if ((stages == 0) || (wavelet_info->width == 0) || (wavelet_info->steps == 0)) return;

	cv::Mat fir_l_m;
	cv::Mat fir_h_m;

	if ((iter_id + 1) * stages > this->wavelet_info->steps) return;

	unsigned int half_filter_size = 2 * stages * (2 * wavelet_info->width - 1);
	unsigned int filter_size = (1 + 2 * half_filter_size);
	unsigned int matrix_size = filter_size + 1;
	unsigned int fir_filter_size = matrix_size >> 1;

	this->createFirFilter1D(stages, iter_id, fir_l_m, fir_h_m, optim_thread);

	fir_l.resize(matrix_size);
	fir_h.resize(matrix_size);

	for (unsigned int i = 0; i < fir_filter_size; i++)
	{
		for (unsigned int k = 0; k < 2; k++)
		{
			fir_l[i + fir_filter_size * k] = fir_l_m.at<float>(i * 2 + (k % 2), 0);
			fir_h[i + fir_filter_size * k] = fir_h_m.at<float>(i * 2 + (k % 2), 0);
		}
	}
}

void WaveletKernelGenerator::getFirData2D(unsigned int stages, unsigned int iter_id, std::vector<float> &fir_ll, std::vector<float> &fir_hl, std::vector<float> &fir_lh, std::vector<float> &fir_hh, unsigned int &subband_row_size)
{
	if ((stages == 0) || (wavelet_info->width == 0) || (wavelet_info->steps == 0)) return;

	cv::Mat fir_ll_m;
	cv::Mat fir_hl_m;
	cv::Mat fir_lh_m;
	cv::Mat fir_hh_m;

	if ((iter_id + 1) * stages > this->wavelet_info->steps) return;

	float norm = ((iter_id + 1) * stages == this->wavelet_info->steps) ? this->wavelet_info->norm : 1.0f;

	unsigned int half_filter_size = 2 * stages * (2 * wavelet_info->width - 1);
	unsigned int filter_size = (1 + 2 * half_filter_size);
	unsigned int matrix_size = filter_size + 1;
	unsigned int fir_filter_size = matrix_size >> 1;

	this->createFirFilter2D(stages, iter_id, fir_ll_m, fir_hl_m, fir_lh_m, fir_hh_m);

	fir_ll.resize(matrix_size * matrix_size);
	fir_hl.resize(matrix_size * matrix_size);
	fir_lh.resize(matrix_size * matrix_size);
	fir_hh.resize(matrix_size * matrix_size);

	for (unsigned int j = 0; j < fir_filter_size; j++)
	{
		for (unsigned int i = 0; i < fir_filter_size; i++)
		{
			for (unsigned int k = 0; k < 4; k++)
			{
				fir_ll[i + j * fir_filter_size + fir_filter_size * fir_filter_size * k] = fir_ll_m.at<float>(i * 2 + (k % 2), j * 2 + (k / 2)) * ((optim_thread) ? 1.0f : (norm * norm));
				fir_hl[i + j * fir_filter_size + fir_filter_size * fir_filter_size * k] = fir_hl_m.at<float>(i * 2 + (k % 2), j * 2 + (k / 2));
				fir_lh[i + j * fir_filter_size + fir_filter_size * fir_filter_size * k] = fir_lh_m.at<float>(i * 2 + (k % 2), j * 2 + (k / 2));
				fir_hh[i + j * fir_filter_size + fir_filter_size * fir_filter_size * k] = fir_hh_m.at<float>(i * 2 + (k % 2), j * 2 + (k / 2)) / ((optim_thread) ? 1.0f : (norm * norm));
			}
		}
	}
	subband_row_size = fir_filter_size;
}


std::string WaveletKernelGenerator::getFilterBody()
{
	std::ostringstream body;
	body.setf(std::ios_base::showpoint);
	switch (this->kernel_type)
	{
	case WAVELET_KERNEL_SWELDENS:
		this->getSweldensBody(body);
		break;
	case WAVELET_KERNEL_IWAHASHI:
		this->getIwahashiBody(body);
		break;
	case WAVELET_KERNEL_EXPLOSIVE:
		this->getExplosiveBody(body);
		break;
	case WAVELET_KERNEL_MONOLITHIC:
		this->getMonolithicBody(body);
		break;
	case WAVELET_KERNEL_POLYPHASE:
		this->getPolyphaseBody(body);
		break;
	case WAVELET_KERNEL_CONVOLUTION:
		this->getConvolutionBody(body);
		break;
	case WAVELET_KERNEL_CONVOLUTION_SEP_NONE:
		this->getConvolutionSepNoneBody(body);
		break;
	case WAVELET_KERNEL_CONVOLUTION_SEP_HOR:
		this->getConvolutionSepHorBody(body);
		break;
	case WAVELET_KERNEL_CONVOLUTION_SEP_VERT:
		this->getConvolutionSepVertBody(body);
		break;
	case WAVELET_KERNEL_CONVOLUTION_SEP_ALL:
		this->getConvolutionSepAllBody(body);
		break;
	case WAVELET_KERNEL_POLYPHASE_SEP_NONE:
		this->getPolyphaseSepNoneBody(body);
		break;
	case WAVELET_KERNEL_POLYPHASE_SEP_HOR:
		this->getPolyphaseSepHorBody(body);
		break;
	case WAVELET_KERNEL_POLYPHASE_SEP_VERT:
		this->getPolyphaseSepVertBody(body);
		break;
	case WAVELET_KERNEL_POLYPHASE_SEP_ALL:
		this->getPolyphaseSepAllBody(body);
		break;
	}
	if(this->steps.size() != 0)
	{
		body << this->steps.back()->getBody();
	}
	return body.str();
}

void WaveletKernelGenerator::findBorders()
{
	for(int i = 0; i < this->steps.size(); i++)
	{
		this->local_borders.resize(this->steps[i]->getLocalBorders());
		this->op_count += this->steps[i]->getOpCount();
		this->shuffle_count += this->steps[i]->getShuffleCount();
		this->store_count += this->steps[i]->getStoreCount();
		this->load_count += this->steps[i]->getLoadCount();
		this->barrier_count += this->steps[i]->getBarrierCount();
	}
}