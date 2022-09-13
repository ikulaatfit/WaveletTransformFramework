#include "WaveletKernelGeneratorStep.h"

std::array<unsigned char, 4> empty_coef = { 255,255,255,255 };

std::array<float, 4> operator-(std::array<float, 4> &input)
{
	return { -input[0], -input[1], -input[2] , -input[3] };
}

const std::string orig_reg_name_f4("act_data");
const std::string temp_reg_name_f1("tmp_data");
const std::string temp_reg_name_f4("act_data2");
//const std::string empty_string("");

const std::vector<std::string> subband_labels = { std::string("LL"),std::string("HL"),std::string("LH"),std::string("HH") };

WaveletKernelGeneratorStep::WaveletKernelGeneratorStep(s_wavelet_type_info *wavelet_info, std::array<int, 2> pairs_per_thread, bool optim_thread, bool read_once, std::string function_name, WaveletKernelGeneratorStep *parent)
{
	this->pairs_per_thread = pairs_per_thread;
	this->optim_thread = optim_thread;
	this->op_count = 0;
	this->load_count = 0;
	this->store_count = 0;
	this->shuffle_count = 0;
	this->barrier_count = 0;
	this->wavelet_info = wavelet_info;
	this->read_once = read_once;
	this->function_name = function_name;
	this->parent = parent;
}

void WaveletKernelGeneratorStep::appendNormBody()
{
	for (int j = 0; j < this->pairs_per_thread[1]; j++)
	{
		for (int i = 0; i < this->pairs_per_thread[0]; i++)
		{
			if (this->wavelet_info->norm != 1.0f)
			{
				std::array<int, 2> reg_pos = { i, j };
				std::array<int, 2> pos = { 0,0 };
				std::string out = getValue(std::string("act_data"), { 0,0,0,0 }, pos, reg_pos, SUB_LL, false);
				body << set(out, mul(out, getFloat(this->wavelet_info->norm * this->wavelet_info->norm)));
				out = getValue(std::string("act_data"), { 0,0,0,0 }, pos, reg_pos, SUB_HH, false);
				body << set(out, mul(out, getFloat(1.0f / (this->wavelet_info->norm * this->wavelet_info->norm))));
			}
		}
	}
}



void WaveletKernelGeneratorStep::appendImprovedPredictBodyH(unsigned int iter_id)
{
	this->appendLoadFromLocalBody(empty_coef, SUB_LL, SUB_HL, 0, 0, wavelet_info->coef[iter_id * wavelet_info->width * 2], '+');
	this->appendLoadFromLocalBody(empty_coef, SUB_LH, SUB_HH, 0, 0, wavelet_info->coef[iter_id * wavelet_info->width * 2], '+');
}

void WaveletKernelGeneratorStep::appendImprovedPredictBodyV(unsigned int iter_id)
{
	this->appendLoadFromLocalBody(empty_coef, SUB_LL, SUB_LH, 0, 0, wavelet_info->coef[iter_id * wavelet_info->width * 2], '+');
	this->appendLoadFromLocalBody(empty_coef, SUB_HL, SUB_HH, 0, 0, wavelet_info->coef[iter_id * wavelet_info->width * 2], '+');
}

void WaveletKernelGeneratorStep::appendImprovedUpdateBodyH(unsigned int iter_id)
{
	this->appendLoadFromLocalBody(empty_coef, SUB_HL, SUB_LL, 0, 0, wavelet_info->coef[(1 + 2 * iter_id) * wavelet_info->width], '+');
	this->appendLoadFromLocalBody(empty_coef, SUB_HH, SUB_LH, 0, 0, wavelet_info->coef[(1 + 2 * iter_id) * wavelet_info->width], '+');
}

void WaveletKernelGeneratorStep::appendImprovedUpdateBodyV(unsigned int iter_id)
{
	this->appendLoadFromLocalBody(empty_coef, SUB_LH, SUB_LL, 0, 0, wavelet_info->coef[(1 + 2 * iter_id) * wavelet_info->width], '+');
	this->appendLoadFromLocalBody(empty_coef, SUB_HH, SUB_HL, 0, 0, wavelet_info->coef[(1 + 2 * iter_id) * wavelet_info->width], '+');
}

void WaveletKernelGeneratorStep::appendLoadFromLocalBody(std::array<unsigned char, 4> coef_local_id, e_subband subbband_src_id, e_subband subband_dst_id, int i, int j, float input_coef, unsigned char input_op, const std::string &src_reg, const std::string &dst_reg)
{
	float coef;
	if (input_op == '-')
	{
		coef = -input_coef;
	}
	else
	{
		coef = input_coef;
	}
	for (int y_thr = 0; y_thr < this->pairs_per_thread[1]; y_thr++)
	{
		for (int x_thr = 0; x_thr < this->pairs_per_thread[0]; x_thr++)
		{
			std::array<int, 2> dst_reg_pos = { x_thr, y_thr };
			std::array<int, 2> dst_pos = { 0,0 };
			std::array<int, 2> src_pos = { positive_div(x_thr + i, this->pairs_per_thread[0]), positive_div(y_thr + j, this->pairs_per_thread[1]) };
			std::array<int, 2> src_reg_pos = { positive_mod(x_thr + i, this->pairs_per_thread[0]), positive_mod(y_thr + j, this->pairs_per_thread[1]) };
			int src_reg_id = src_reg_pos[1] * this->pairs_per_thread[0] + src_reg_pos[0];
			int dst_reg_id = dst_reg_pos[1] * this->pairs_per_thread[0] + dst_reg_pos[0];

			std::string out = getValue(dst_reg, coef_local_id, dst_pos, dst_reg_pos, subband_dst_id, false);
			std::string a = getValue(src_reg, coef_local_id, src_pos, src_reg_pos, subbband_src_id, true);

			if (src_pos[0] == 0 && src_pos[1] == 0) // intra thread calculations
			{
				if ((subbband_src_id == subband_dst_id) && (dst_reg_id == src_reg_id)) // update same value
				{
					if (coef == 1.0f)
					{
						body << set(out, a);
					}
					else if (coef == 0.0f)
					{
						body << set(out, getFloat(0.0));
					}
					else
					{
						body << set(out, mul(getFloat(coef), a));
					}
				}
				else if (coef != 0.0f)
				{
					body << set(out, fma(getFloat(coef), a, out));
				}
			}
			else if (coef != 0.0f) // inter thread calculations
			{
				if (coef != 1.0f) body << set(out, fma(getFloat(coef), a, out));
				else body << set(out, add(a, out));
			}
		}
	}
}

void WaveletKernelGeneratorStep::appendLoadFromLocalBody(std::array<unsigned char, 4> coef_local_id, e_subband subband_src_id, int i, int j, std::array<float, 4> input_coefs, unsigned char input_op, const std::string &src_reg, const std::string &dst_reg, const std::string &tmp_reg)
{
	std::array<float, 4> coefs;
	unsigned char op;
	if (input_op == '-')
	{
		coefs = -input_coefs;
		op = '+';
	}
	else
	{
		coefs = input_coefs;
		op = '+';
	}
	int dst_count = 0;
	int last_id;
	// dst:src ratio
	for (int k = 0; k < 4; k++)
	{
		if ((coefs[k] != FLT_MAX) && (coefs[k] != 0.0f))
		{
			dst_count++;
			last_id = k;
		}
	}

	if ((i == 0) && (j == 0)) // intra thread calculations
	{
		for (int subband_dst_id = 0; subband_dst_id < 4; subband_dst_id++)
		{
			if (subband_dst_id == subband_src_id) continue;
			float coef = coefs[subband_dst_id];
			if (coef != FLT_MAX) this->appendLoadFromLocalBody(coef_local_id, subband_src_id, (e_subband)subband_dst_id, i, j, coef, op, src_reg, dst_reg);
		}
	}
	else if (dst_count > 0)// inter thread calculations
	{
		for (int y_thr = 0; y_thr < this->pairs_per_thread[1]; y_thr++)
		{
			for (int x_thr = 0; x_thr < this->pairs_per_thread[0]; x_thr++)
			{
				std::array<int, 2> dst_reg_pos = { x_thr, y_thr };
				std::array<int, 2> dst_pos = { 0,0 };
				std::array<int, 2> src_pos = { positive_div(x_thr + i, this->pairs_per_thread[0]), positive_div(y_thr + j, this->pairs_per_thread[1]) };
				std::array<int, 2> src_reg_pos = { positive_mod(x_thr + i, this->pairs_per_thread[0]), positive_mod(y_thr + j, this->pairs_per_thread[1]) };

				std::string a = getValue(src_reg, coef_local_id, src_pos, src_reg_pos, subband_src_id, true);

				if (read_once)
				{
					if (dst_count > 1)
					{
						body << set(tmp_reg, a);
					}
					else
					{
						std::string out = getValue(dst_reg, coef_local_id, dst_pos, dst_reg_pos, (e_subband)last_id, false);
						if (coefs[last_id] != 1.0f) body << set(out, fma(a, getFloat(coefs[last_id]), out));
						else body << set(out, add(a, out));
					}
				}
				if ((dst_count > 1) || (!read_once))
				{
					std::string in_reg;
					if (read_once) in_reg = tmp_reg;
					else in_reg = a;
					for (int k = 0; k < 4; k++)
					{
						if ((coefs[k] != FLT_MAX) && (coefs[k] != 0.0f))
						{
							std::string out = getValue(dst_reg, coef_local_id, dst_pos, dst_reg_pos, (e_subband)k, false);
							if (coefs[k] != 1.0f) body << set(out, fma(in_reg, getFloat(coefs[k]), out));
							else body << set(out, add(in_reg, out));
						}
					}
				}
			}
		}
	}
}

void WaveletKernelGeneratorStep::append2DGenBody(int half_filter_size, std::array<unsigned char, 4> coef_local_id, std::vector<float> &ll_coef, std::vector<float> &hl_coef, std::vector<float> &lh_coef, std::vector<float> &hh_coef, const std::string &src_reg, const std::string &tmp_data)
{
	int fir_filter_size = (1 + 2 * half_filter_size);
	int coef_part_size = fir_filter_size * fir_filter_size;
	std::vector<float> *coef[4] = { &ll_coef, &hl_coef, &lh_coef, &hh_coef };
	for (int l = 0; l < 4; l++)
	{
		int tmp_index = (half_filter_size)+fir_filter_size * (half_filter_size)+l * coef_part_size;
		appendLoadFromLocalBody(coef_local_id, (e_subband)l, (e_subband)l, 0, 0, (*(coef[l]))[tmp_index], '+', src_reg, orig_reg_name_f4);
	}
	for (int j = -half_filter_size; j <= half_filter_size; j++)
	{
		for (int i = -half_filter_size; i <= half_filter_size; i++)
		{
			for (int l = 0; l < 4; l++)
			{
				int tmp_index = (i + half_filter_size) + fir_filter_size * (j + half_filter_size) + l * coef_part_size;
				appendLoadFromLocalBody(coef_local_id, (e_subband)l, i, j, { ll_coef[tmp_index], hl_coef[tmp_index], lh_coef[tmp_index], hh_coef[tmp_index] }, '+', src_reg, orig_reg_name_f4, tmp_data);
			}
		}
	}
}

void WaveletKernelGeneratorStep::append1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg)
{
	int filter_stride = (int)(coef.size() / 2);
	int half_filter_size = (int)(coef.size() / 4);
	unsigned int id_l = (id ^ 0x1) & 0x1;
	unsigned int id_r = id & 0x1;

	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)id, (e_subband)id, 0, 0, coef[half_filter_size + id_r * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id ^ 0x1), (e_subband)id, 0, 0, coef[half_filter_size + id_l * filter_stride], '+', src_reg);
	// inter thread calculation
	for (int i = -half_filter_size; i <= half_filter_size; i++)
	{
		if (i == 0) continue;
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id & 0x2), (e_subband)id, i, 0, coef[i + half_filter_size], '+', src_reg);
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id & 0x2) + 1), (e_subband)id, i, 0, coef[i + half_filter_size + filter_stride], '+', src_reg);
	}
}

void WaveletKernelGeneratorStep::append1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg, const std::string &tmp_data)
{
	int filter_stride = (int)(l_coef.size() / 2);
	int half_filter_size = (int)(l_coef.size() / 4);
	unsigned int id_l = (id ^ 0x1) & 0x1;
	unsigned int id_r = id & 0x1;

	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)id, (e_subband)id, 0, 0, l_coef[half_filter_size + id_r * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id ^ 0x1), (e_subband)id, 0, 0, l_coef[half_filter_size + id_l * filter_stride], '+', src_reg);

	id_l = ((id + 1) ^ 0x1) & 0x1;
	id_r = (id + 1) & 0x1;
	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id + 1), (e_subband)(id + 1), 0, 0, h_coef[half_filter_size + id_r * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id + 1) ^ 0x1), (e_subband)(id + 1), 0, 0, h_coef[half_filter_size + id_l * filter_stride], '+', src_reg);
	// inter thread calculation
	for (int i = -half_filter_size; i <= half_filter_size; i++)
	{
		if (i == 0) continue;
		std::array<float, 4> coef = { 0.0f,0.0f,0.0f,0.0f };
		coef[id] = l_coef[i + half_filter_size];
		coef[id + 1] = h_coef[i + half_filter_size];
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id & 0x2), i, 0, coef, '+', src_reg, orig_reg_name_f4, tmp_data);
		coef[id] = l_coef[i + half_filter_size + filter_stride];
		coef[id + 1] = h_coef[i + half_filter_size + filter_stride];
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id & 0x2) + 1), i, 0, coef, '+', src_reg, orig_reg_name_f4, tmp_data);
	}
}

void WaveletKernelGeneratorStep::append1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg)
{
	int filter_stride = (int)(coef.size() / 2);
	int half_filter_size = (int)(coef.size() / 4);
	unsigned int id_t = ((id ^ 0x2) & 0x2) >> 1;
	unsigned int id_b = (id & 0x2) >> 1;

	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)id, (e_subband)id, 0, 0, coef[half_filter_size + id_b * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id ^ 0x2), (e_subband)id, 0, 0, coef[half_filter_size + id_t * filter_stride], '+', src_reg);
	// inter thread calculation
	for (int j = -half_filter_size; j <= half_filter_size; j++)
	{
		if (j == 0) continue;
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id & 0x1), (e_subband)id, 0, j, coef[j + half_filter_size], '+', src_reg);
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id & 0x1) + 2), (e_subband)id, 0, j, coef[j + half_filter_size + filter_stride], '+', src_reg);
	}
}

void WaveletKernelGeneratorStep::append1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg, const std::string &tmp_data)
{
	int filter_stride = (int)(l_coef.size() / 2);
	int half_filter_size = (int)(l_coef.size() / 4);
	unsigned int id_t = ((id ^ 0x2) & 0x2) >> 1;
	unsigned int id_b = (id & 0x2) >> 1;

	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)id, (e_subband)id, 0, 0, l_coef[half_filter_size + id_b * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id ^ 0x2), (e_subband)id, 0, 0, l_coef[half_filter_size + id_t * filter_stride], '+', src_reg);

	id_t = (((id + 2) ^ 0x2) & 0x2) >> 1;
	id_b = ((id + 2) & 0x2) >> 1;
	// output value initialization
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id + 2), (e_subband)(id + 2), 0, 0, h_coef[half_filter_size + id_b * filter_stride], '+');
	// intra thread calculation
	this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id + 2) ^ 0x2), (e_subband)(id + 2), 0, 0, h_coef[half_filter_size + id_t * filter_stride], '+', src_reg);
	// inter thread calculation
	for (int j = -half_filter_size; j <= half_filter_size; j++)
	{
		if (j == 0) continue;
		std::array<float, 4> coef = { 0.0f,0.0f,0.0f,0.0f };
		coef[id] = l_coef[j + half_filter_size];
		coef[id + 2] = h_coef[j + half_filter_size];
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)(id & 0x1), 0, j, coef, '+', src_reg, orig_reg_name_f4, tmp_data);
		coef[id] = l_coef[j + half_filter_size + filter_stride];
		coef[id + 2] = h_coef[j + half_filter_size + filter_stride];
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)((id & 0x1) + 2), 0, j, coef, '+', src_reg, orig_reg_name_f4, tmp_data);
	}
}


void WaveletKernelGeneratorStep::append1DLeftBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	unsigned int id_x = id & 0x1;
	unsigned int id_l = id ^ 0x1;
	for (int i = -((int)(wavelet_info->width)) + id_x; i < 0; i++)
	{
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_l, (e_subband)id, i, 0, coef[-1 * (i - id_x + 1)], op, src_reg);
	}
	if ((!optim_thread) && id_x) this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_l, (e_subband)id, 0, 0, coef[0], op, src_reg);
}

void WaveletKernelGeneratorStep::append1DRightBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	unsigned int id_x = id & 0x1;
	unsigned int id_r = id ^ 0x1;
	for (int i = 1; i < (int)(wavelet_info->width + id_x); i++)
	{
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_r, (e_subband)id, i, 0, coef[i - id_x], op, src_reg);
	}
	if ((!optim_thread) && (!id_x)) this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_r, (e_subband)id, 0, 0, coef[0], op, src_reg);
}

void WaveletKernelGeneratorStep::append1DTopBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	unsigned int id_y = (id & 0x2) >> 1;
	unsigned int id_t = id ^ 0x2;
	for (int j = -((int)(wavelet_info->width)) + (int)id_y; j < 0; j++)
	{
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_t, (e_subband)id, 0, j, coef[-1 * (j - (int)id_y + 1)], op, src_reg);
	}
	if ((!optim_thread) && id_y) this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_t, (e_subband)id, 0, 0, coef[0], op, src_reg);
}

void WaveletKernelGeneratorStep::append1DBottomBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	unsigned int id_y = (id & 0x2) >> 1;
	unsigned int id_b = id ^ 0x2;
	for (int j = 1; j < (int)(wavelet_info->width + id_y); j++)
	{
		this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_b, (e_subband)id, 0, j, coef[j - (int)id_y], op, src_reg);
	}
	if ((!optim_thread) && (!id_y)) this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_b, (e_subband)id, 0, 0, coef[0], op, src_reg);
}

void WaveletKernelGeneratorStep::append2DBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	unsigned int id_c = id ^ 0x3;
	int id_y = (id & 0x2) >> 1;
	int id_x = id & 0x1;
	for (int j = -((int)(wavelet_info->width)) + id_y; j < ((int)(wavelet_info->width + id_y)); j++)
	{
		if ((j == 0) && optim_thread) continue;
		float act_coef_y = (j - id_y + 1 <= 0) ? coef[-1 * (j - id_y + 1)] : coef[j - id_y];
		for (int i = -((int)(wavelet_info->width)) + id_x; i < ((int)(wavelet_info->width + id_x)); i++)
		{
			if ((i == 0) && optim_thread) continue;
			float act_coef_x = (i - id_x + 1 <= 0) ? coef[-1 * (i - id_x + 1)] : coef[i - id_x];
			float act_coef = act_coef_x * act_coef_y;
			this->appendLoadFromLocalBody(coef_local_id, (e_subband)id_c, (e_subband)id, i, j, act_coef, op, src_reg);
		}
	}
}

std::string WaveletKernelGeneratorStep::getBody() const
{
	return body.str();
}

void WaveletKernelGeneratorStep::appendToBody(std::string &val)
{
	this->body << val;
}
void WaveletKernelGeneratorStep::appendToSubstepBody(std::string &val)
{
	if (this->steps.size() == 0)
	{
		this->appendStep();
	}
	this->steps.back()->appendToBody(val);
}

void WaveletKernelGeneratorStep::appendSubstepsImprovedPredictBody(unsigned int iter_id)
{
	this->appendSubstepImprovedPredictBodyH(iter_id);
	this->appendSubstepImprovedPredictBodyV(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstepsImprovedUpdateBody(unsigned int iter_id)
{
	this->appendSubstepImprovedUpdateBodyH(iter_id);
	this->appendSubstepImprovedUpdateBodyV(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstepNormBody()
{
	this->appendStep();
	this->steps.back()->appendNormBody();
}

void WaveletKernelGeneratorStep::appendSubstepImprovedPredictBodyH(unsigned int iter_id)
{
	this->appendStep();
	this->steps.back()->appendImprovedPredictBodyH(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstepImprovedPredictBodyV(unsigned int iter_id)
{
	this->appendStep();
	this->steps.back()->appendImprovedPredictBodyV(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstepImprovedUpdateBodyH(unsigned int iter_id)
{
	this->appendStep();
	this->steps.back()->appendImprovedUpdateBodyH(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstepImprovedUpdateBodyV(unsigned int iter_id)
{
	this->appendStep();
	this->steps.back()->appendImprovedUpdateBodyV(iter_id);
}

void WaveletKernelGeneratorStep::appendSubstep2DGenBody(int half_filter_size, std::array<unsigned char, 4> coef_local_id, std::vector<float> &ll_coef, std::vector<float> &hl_coef, std::vector<float> &lh_coef, std::vector<float> &hh_coef, const std::string &src_reg, const std::string &tmp_data)
{
	this->appendStep();
	this->steps.back()->append2DGenBody(half_filter_size, coef_local_id, ll_coef, hl_coef, lh_coef, hh_coef, src_reg, tmp_data);
}

void WaveletKernelGeneratorStep::appendSubstep1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DHorizontalGenBody(id, coef_local_id, coef, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep1DHorizontalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg, const std::string &tmp_data)
{
	this->appendStep();
	this->steps.back()->append1DHorizontalGenBody(id, coef_local_id, l_coef, h_coef, src_reg, tmp_data);
}

void WaveletKernelGeneratorStep::appendSubstep1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &coef, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DVerticalGenBody(id, coef_local_id, coef, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep1DVerticalGenBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, std::vector<float> &l_coef, std::vector<float> &h_coef, const std::string &src_reg, const std::string &tmp_data)
{
	this->appendStep();
	this->steps.back()->append1DVerticalGenBody(id, coef_local_id, l_coef, h_coef, src_reg, tmp_data);
}

void WaveletKernelGeneratorStep::appendSubstep1DLeftBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DLeftBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep1DRightBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DRightBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep1DTopBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DTopBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep1DBottomBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append1DBottomBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubstep2DBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	this->appendStep();
	this->steps.back()->append2DBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubsteps1DHorizontalBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	appendSubstep1DLeftBody(id, coef_local_id, coef, op, src_reg);
	appendSubstep1DRightBody(id, coef_local_id, coef, op, src_reg);
}

void WaveletKernelGeneratorStep::appendSubsteps1DVerticalBody(unsigned int id, std::array<unsigned char, 4> coef_local_id, float *coef, unsigned char op, const std::string &src_reg)
{
	appendSubstep1DTopBody(id, coef_local_id, coef, op, src_reg);
	appendSubstep1DBottomBody(id, coef_local_id, coef, op, src_reg);
}

s_local_borders WaveletKernelGeneratorStep::getLocalBorders() const
{
	s_local_borders local_borders = this->local_borders;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		local_borders.resize((*it)->local_borders);
	}
	return local_borders;
}
v_step_borders WaveletKernelGeneratorStep::getStepBorders() const
{
	v_step_borders step_borders = this->step_borders;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		step_borders.elements.insert(step_borders.elements.end(),  (*it)->step_borders.elements.begin(), (*it)->step_borders.elements.end());
	}
	return step_borders;
}
int WaveletKernelGeneratorStep::getOpCount() const
{
	int op_count = this->op_count;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		op_count += (*it)->op_count;
	}
	return op_count;
}
int WaveletKernelGeneratorStep::getShuffleCount() const
{
	int shuffle_count = this->shuffle_count;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		shuffle_count += (*it)->shuffle_count;
	}
	return shuffle_count;
}
int WaveletKernelGeneratorStep::getStoreCount() const
{
	int store_count = this->store_count;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		store_count += (*it)->store_count;
	}
	return store_count;
}
int WaveletKernelGeneratorStep::getLoadCount() const
{
	int load_count = this->load_count;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		load_count += (*it)->load_count;
	}
	return load_count;
}
int WaveletKernelGeneratorStep::getBarrierCount() const
{
	int barrier_count = this->barrier_count;
	for (std::vector<std::shared_ptr<WaveletKernelGeneratorStep>>::const_iterator it = steps.begin(); it != steps.end(); it++)
	{
		barrier_count += (*it)->barrier_count;
	}
	return barrier_count;
}
