#include "encoder.h"
using namespace std;
#if THREAD_EN
void thread_core(string cfg)
{
    system(cfg.data());
}
#endif
void encoder(CFG_INFO& config_info, int patch_width[PATCH_CNT], int patch_height[PATCH_CNT], string yuv_org[5], string yuv_rec[5], string bin[5])
{
    string head_info = config_info.head_info_app;
    head_info.append(" --config ");
    head_info += config_info.cfg_info[0];
    head_info.append(" -o head.bin -f ");
    head_info += config_info.cfg_info[6];

    /* generate head bitstream */
    system(head_info.data());

    string width[PATCH_CNT + 1];
    string height[PATCH_CNT + 1];
    string w = "-w ";
    string h = "-h ";

    for (int i = 0; i < PATCH_CNT; i++)
    {
        width[i] = to_string(patch_width[i]);
        height[i] = to_string(patch_height[i]);
    }

    const char* pragma[] = { " --config ",  " -i ", " -w ", " -h ", " -z ", " -p ", " -f ", " -q ", " -o ", " -r " };
    string cfg = config_info.encode_app;
    for (int i = 0; i < COMAND_CNT; i++)
    {
        if (config_info.cfg_info[i].length())
        {
            cfg.append(pragma[i]);
            if (pragma[i] == " -w ")
            {
                cfg += width[0];
            }
            else if (pragma[i] == " -h ")
            {
                cfg += height[0];
            }
            else if (pragma[i] == " -o ")
            {
                cfg += bin[0] + ".bin ";
            }
            else if (pragma[i] == " -r ")
            {
                cfg += yuv_rec[0] + ".yuv ";
            }
            else if (pragma[i] == " -i ")
            {
                cfg += yuv_org[0] + ".yuv ";
            }
            else
            {
                cfg += config_info.cfg_info[i];
            }
        }
    }

#if THREAD_EN
    thread task[PATCH_CNT];
#endif

    /* encode patch separatly */
    for (int i = 0; i < PATCH_CNT; i++)
    {
#if THREAD_EN
        task[i] = thread(thread_core, cfg);
#else
        system(cfg.data());
#endif
        SWITCH_YUV
    }
#if THREAD_EN
    for (auto& t : task) {
        t.join();
    }
#endif
}


