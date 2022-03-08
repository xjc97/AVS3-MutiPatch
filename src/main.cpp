// MutiPatch.cpp: 定义应用程序的入口点。

#include "common.h"
#include "bs_process.h"
#include "encoder.h"
#include "yuv_process.h"
#include "app_args.h"

bool is_file_exist(string path) {
#if defined(__linux__)
    if (access(path.c_str(), 0) == 0)return true;
    else return false;
#else
    if (_access(path.c_str(), 0) == 0)return true;
    else return false;
#endif
}

int main(int argc, const char** argv)
{
    /* config parse */
    CFG_INFO config;
    args_parse(argc, argv, config);
    int frame_cnt = atoi(config.cfg_info[6].c_str());
    int pic_width = atoi(config.cfg_info[2].c_str());
    int pic_height = atoi(config.cfg_info[3].c_str());
    int patch_width[PATCH_CNT] = { 0 };
    int patch_height[PATCH_CNT] = { 0 };

    const char* pragma[] = { " --config ",  " -i ", " -w ", " -h ", " -z ", " -p ", " -f ", " -q ", " -o ", " -r " };
    string cfg = config.encode_app;
    for (int i = 0; i < COMAND_CNT; i++)
    {
        if (config.cfg_info[i].length())
        {
            cfg.append(pragma[i]);
            cfg += config.cfg_info[i];
        }
    }
#if SPLIT_MODE != 0
    for (int i = 0; i < PATCH_CNT; i++)
    {
        if (i & 1)
        {
            patch_width[i] = pic_width - patch_width[i - 1];
        }
        else
        {
            patch_width[i] = ((pic_width / 64) / 2) * 64;
        }
        if (PATCH_CNT == 2)
        {
            patch_height[i] = pic_height;
        }
        else
        {
            if (i <= 1)
            {
                patch_height[i] = ((pic_height / 64) / 2) * 64;
            }
            else
            {
                patch_height[i] = pic_height - patch_height[0];
            }
        }
    }
#else
    for (int i = 0; i < PATCH_CNT; i++)
    {
        if (i & 1)
        {
            patch_height[i] = pic_height - patch_height[i - 1];
        }
        else
        {
            patch_height[i] = 128;
        }

        patch_width[i] = pic_width;
    }
#endif
	/* split org yuv */

    //yuv_split(config, patch_width, patch_height, pic_width, pic_height);

    string yuv_org[5];
    string yuv_rec[5];
    string bin[5];
    for (int i = 0; i < 5; i++)
    {
        yuv_org[i] = "patch" + to_string(i);
        yuv_rec[i] = yuv_org[i] + "_rec";
        bin[i] = yuv_org[i] + "_out";
    }

	
    /* run encoder */

    encoder(config, patch_width, patch_height, yuv_org, yuv_rec, bin);

    /* bitstream merge */
    bin_process(bin, frame_cnt);

	/* generate rec yuv */
#if DEBUG
    yuv_joint(config, yuv_rec, patch_width, patch_height);
#endif

#if !DEBUG
    for (int i = 0; i < PATCH_CNT; i++)
    {
        yuv_org[i].append(".yuv");
        bin[i].append(".bin");
        if (is_file_exist(yuv_org[i]))
        {
            remove(yuv_org[i].data());
        }
        if (is_file_exist(bin[i]))
        {
            remove(bin[i].data());
        }
    }
#endif

	return 0;
}
