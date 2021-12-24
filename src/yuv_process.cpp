#include "yuv_process.h"

void yuv_joint()
{
	system("ffmpeg.exe -y -pix_fmt yuv420p -s 192*240 -i Unit1-1_rec.yuv -pix_fmt yuv420p -s 224*240 -i Unit1-2_rec.yuv -filter_complex \"[0:v]pad = 416:240[a]; [a] [1:v] overlay = 192\" -pix_fmt yuv420p test_rec.yuv");
}

void yuv_split(CFG_INFO & config, int patch_width[PATCH_CNT], int patch_height[PATCH_CNT], int pic_width, int pic_height)
{
    string split_yuv_command[PATCH_CNT];


    int x_pos, y_pos = 0;
    for (int i = 0; i < PATCH_CNT; i++)
    {
        x_pos = i & 1 ? patch_width[0] : 0;
        y_pos = i > 1 ? patch_height[0] : 0;
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