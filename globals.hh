#pragma once
#include <unordered_map>

class c_globals {
public:
	bool active = true;
	char path[255] = "path";
	char file[255] = "file";
	char game[255] = "game";
};

inline c_globals globals;