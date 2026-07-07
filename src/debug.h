#ifndef DEBUG_H
#define DEBUG_H

#include <cstdarg>
#include "raylib.h"

class Debug {
	static const int MAX_LOGS = 20;
	static const int MSG_LEN = 128;

	char messages[MAX_LOGS][MSG_LEN];
	int count;

public:
	Debug();

	void Log(const char *fmt, ...);
	void Clear();
	void Draw(Font font);
};

#endif
