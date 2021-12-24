#include "app_args.h"
#define PARSE(X,Y) \
for (int i = 1; i < argc; i++)\
{\
	if (!strcmp(argv[i], X))\
	{\
		if (i + 1 < argc)\
		{\
			config.Y = argv[i + 1];\
			break;\
		}\
	}\
}


void args_parse(int argc, const char* argv[], CFG_INFO & config)
{

	PARSE("--config", cfg_info[0])
	PARSE("-i", cfg_info[1])
	PARSE("-w", cfg_info[2])
	PARSE("-h", cfg_info[3])
	PARSE("-z", cfg_info[4])
	PARSE("-p", cfg_info[5])
	PARSE("-f", cfg_info[6])
	PARSE("-q", cfg_info[7])
	PARSE("-o", cfg_info[8])
		PARSE("-r", cfg_info[9])
		PARSE("-app", encode_app)
		PARSE("-head", head_info_app)
		PARSE("-ffmpeg", ffmpeg_app)

}