#ifndef _COM_
#define _COM_
#include <stdlib.h>
#include <string>
#include <assert.h>
using namespace std;

#define DEBUG 0
#define PATCH_CNT 2
#define COMAND_CNT 10
#define EXE_PATH "G:\\MutiPatch\\app_encoder.exe "
#define SWITCH_YUV 		int n = cfg.find(yuv_org[i]);\
						cfg.replace(n, yuv_org[i].length(), yuv_org[i + 1]);\
						n = cfg.find(yuv_rec[i]);\
						cfg.replace(n, yuv_rec[i].length(), yuv_rec[i + 1]);\
						n = cfg.find(bin[i]);\
						cfg.replace(n, bin[i].length(), bin[i + 1]);\
						n = cfg.find(w + width[i]);\
						cfg.replace(n + 3, width[i].length(), width[i + 1]);\
						n = cfg.find(h + height[i]);\
						cfg.replace(n + 3, height[i].length(), height[i + 1]);

typedef struct __CFG_INFO__ 
{
	string cfg_info[COMAND_CNT];
	/*
	0   cfg_file;   
	1   yuv_file;   
	2   pic_width;  
	3   pic_height;
	4   f_value;
	5   intra_prid;
	6   frame_cnt;
	7   qp;
	8   output_file;
	9   rec_file;
	*/
	string encode_app;
	string head_info_app;
	string ffmpeg_app;
}CFG_INFO;

#endif // !_COM_

