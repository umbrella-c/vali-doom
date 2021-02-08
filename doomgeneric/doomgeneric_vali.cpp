#include "doomkeys.h"

extern "C" {
	#include "doomgeneric.h"
}
#include "doomgeneric_vali_wnd.hpp"
#include <ddk/utils.h>

#include <stdio.h>
#include <threads.h>
#include <time.h>

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(unsigned char key, char translated)
{
	switch (key)
	{
	case VK_ENTER:
		key = KEY_ENTER;
		break;
	case VK_ESCAPE:
		key = KEY_ESCAPE;
		break;
	case VK_LEFT:
		key = KEY_LEFTARROW;
		break;
	case VK_RIGHT:
		key = KEY_RIGHTARROW;
		break;
	case VK_UP:
		key = KEY_UPARROW;
		break;
	case VK_DOWN:
		key = KEY_DOWNARROW;
		break;
	case VK_CONTROL:
		key = KEY_FIRE;
		break;
	case VK_SPACE:
		key = KEY_USE;
		break;
	case VK_SHIFT:
		key = KEY_RSHIFT;
		break;
	default:
		key = tolower(translated);
		break;
	}

	return key;
}

void addKeyToQueue(int pressed, unsigned char keyCode, char translated)
{
	WARNING("addKeyToQueue(pressed=%i, keyCode=0x%x, translated=%c)", pressed, keyCode, translated);
	unsigned char key = convertToDoomKey(keyCode, translated);

	unsigned short keyData = (pressed << 8) | key;

	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

extern "C" void DG_Init()
{
	memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned short));
	
    Asgaard::APP.Initialize();
    
    Asgaard::Rectangle initialSize(0, 0, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
    Asgaard::APP.CreateWindow<DoomWindow>(initialSize);
    Asgaard::APP.PumpMessages();
}

extern "C" void DG_DrawFrame()
{
	WARNING("DG_DrawFrame()");
	Asgaard::APP.PumpMessages();

	auto window = Asgaard::APP.Window();
	if (window) {
		auto doomWindow = std::dynamic_pointer_cast<DoomWindow>(window);
		doomWindow->UpdateBuffer(DG_ScreenBuffer);
	}
}

extern "C" void DG_SleepMs(uint32_t ms)
{
	thrd_sleepex(ms);
}

extern "C" uint32_t DG_GetTicksMs()
{
	struct timespec now;
	timespec_get(&now, TIME_PROCESS);
	return (uint32_t)(now.tv_nsec / 1000);
}

extern "C" int DG_GetKey(int* pressed, unsigned char* doomKey)
{
	Asgaard::APP.PumpMessages();

	if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
		//key queue is empty
		return 0;
	}
	else {
		unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
		s_KeyQueueReadIndex++;
		s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

		*pressed = keyData >> 8;
		*doomKey = keyData & 0xFF;

		WARNING("DG_GetKey() returned %i, %c", *pressed, *doomKey);
		return 1;
	}
}

extern "C" void DG_SetWindowTitle(const char* title)
{
	auto window = Asgaard::APP.Window();
	if (window)
	{
		auto doomWindow = std::dynamic_pointer_cast<DoomWindow>(window);
		doomWindow->UpdateTitle(title);
	}
}
