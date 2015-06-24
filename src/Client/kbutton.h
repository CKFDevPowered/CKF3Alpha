#if !defined (KBUTTONH)
#define KBUTTONH
#pragma once

typedef struct kbutton_s
{
	int down[2];
	int state;
}
kbutton_t;

struct kbutton_s *KB_Find(const char *name);

#endif