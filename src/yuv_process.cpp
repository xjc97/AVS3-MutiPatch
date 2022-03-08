#include "yuv_process.h"

void yuv_joint(CFG_INFO& config, string* yuv_rec, int patch_width[PATCH_CNT], int patch_height[PATCH_CNT])
{
    string cfg;
    string width[PATCH_CNT], height[PATCH_CNT];

    for (int i = 0; i < PATCH_CNT; i++)
    {
        width[i] = to_string(patch_width[i]);
        height[i] = to_string(patch_height[i]);
    }

    cfg += config.ffmpeg_app;
    cfg.append(" -y ");
    for (int i = 0; i < PATCH_CNT; i++)
    {
        cfg.append(" -pix_fmt yuv420p -s ");
        cfg += width[i];
        cfg.append("*");
        cfg += height[i];
        cfg.append(" -i ");
        cfg += yuv_rec[i];
        cfg.append(".yuv ");
    }
    cfg.append(" -filter_complex ");
    cfg.append("\"");
    cfg.append(" [0:v]pad = ");
    cfg += config.cfg_info[2];
    cfg.append(":");
    cfg += config.cfg_info[3];
    cfg.append("[a]; [a] [1:v] overlay = ");
#if SPLIT_MODE != 0
    cfg += width[0];
#else
    cfg.append("0:");
    cfg += height[0];
#endif
#if PATCH_CNT == 4
    cfg.append("[b]; [b] [2:v] overlay = ");
    cfg.append("0:");
    cfg += height[0];
    cfg.append("[c]; [c] [3:v] overlay = ");
    cfg += width[0];
    cfg.append(":");
    cfg += height[0];
#endif
    cfg.append("\"");
    cfg.append(" -pix_fmt yuv420p test_rec.yuv");
    printf("%s", cfg.data());
    system(cfg.data());
    //system("ffmpeg.exe -y -pix_fmt yuv420p -s 192*240 -i Unit1-1_rec.yuv -pix_fmt yuv420p -s 224*240 -i Unit1-2_rec.yuv -filter_complex
 //       \"[0:v]pad = 416:240[a]; [a] [1:v] overlay = 192\" -pix_fmt yuv420p test_rec.yuv");
}

void yuv_split(CFG_INFO & config, int patch_width[PATCH_CNT], int patch_height[PATCH_CNT], int pic_width, int pic_height)
{
    string split_yuv_command[PATCH_CNT];


    int x_pos, y_pos = 0;
    for (int i = 0; i < PATCH_CNT; i++)
    {
#if SPLIT_MODE != 0
        x_pos = i & 1 ? patch_width[0] : 0;
        y_pos = i > 1 ? patch_height[0] : 0;
#else
        x_pos = 0;
        y_pos = i == 0 ? 0 : patch_height[0];
#endif
        split_yuv_command[i] = config.ffmpeg_app;
        split_yuv_command[i].append(" -y ");
        split_yuv_command[i].append(" -s ");
        split_yuv_command[i] += config.cfg_info[2];
        split_yuv_command[i].append("*");
        split_yuv_command[i] += config.cfg_info[3];
        split_yuv_command[i].append(" -i ");
        split_yuv_command[i] += config.cfg_info[1];
        split_yuv_command[i].append(" -filter:v crop=");
        split_yuv_command[i] += to_string(patch_width[i]);
        split_yuv_command[i].append(":");
        split_yuv_command[i] += to_string(patch_height[i]);
        split_yuv_command[i].append(":");
        split_yuv_command[i] += to_string(x_pos);
        split_yuv_command[i].append(":");
        split_yuv_command[i] += to_string(y_pos);
        split_yuv_command[i].append(" patch");
        split_yuv_command[i] += to_string(i);
        split_yuv_command[i].append(".yuv");
        system(split_yuv_command[i].data());
    }
}