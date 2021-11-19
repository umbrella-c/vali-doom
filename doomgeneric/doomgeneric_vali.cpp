#include "doomkeys.h"

extern "C" {
	#include "doomgeneric.h"
}
#include "doomgeneric_vali_wnd.hpp"

#include <os/spinlock.h>
#include <stdio.h>
#include <threads.h>
#include <time.h>

#define KEYQUEUE_SIZE 16

static unsigned short              s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int                s_KeyQueueWriteIndex = 0;
static unsigned int                s_KeyQueueReadIndex = 0;
static spinlock_t                  g_queueLock = _SPN_INITIALIZER_NP(spinlock_plain);
static thrd_t                      g_appThread;
static std::shared_ptr<DoomWindow> g_window;

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
	case VK_LCONTROL:
	case VK_RCONTROL:
		key = KEY_FIRE;
		break;
	case VK_SPACE:
		key = KEY_USE;
		break;
	case VK_RSHIFT:
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
	unsigned char key = convertToDoomKey(keyCode, translated);

	unsigned short keyData = (pressed << 8) | key;

	spinlock_acquire(&g_queueLock);
	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
	spinlock_release(&g_queueLock);
}

//static int runApplication(void* unused)
//{
//	return Asgaard::APP.Execute();
//}

extern "C" void DG_Init()
{
    std::string appGuid = "2f8d1d4f-1809-49fa-967e-44d93cc8026d";

	memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned short));
	
    Asgaard::APP.SetSettingString(Asgaard::Application::Settings::APPLICATION_GUID, appGuid);
    Asgaard::APP.Initialize();
    
    Asgaard::Rectangle initialSize(-1, -1, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
    g_window = Asgaard::APP.GetScreen()->CreateWindow<DoomWindow>(initialSize);
	Asgaard::APP.PumpMessages();
	//thrd_create(&g_appThread, runApplication, NULL);
}

extern "C" void DG_DrawFrame()
{
	Asgaard::APP.PumpMessages();
	if (g_window) {
		g_window->UpdateBuffer(DG_ScreenBuffer);
	}
}

extern "C" void DG_SleepMs(uint32_t ms)
{
	thrd_sleepex(ms);
	Asgaard::APP.PumpMessages();
}

extern "C" uint32_t DG_GetTicksMs()
{
	struct timespec now;
	timespec_get(&now, TIME_PROCESS);
	return (uint32_t)((now.tv_sec * 1000) + (now.tv_nsec / NSEC_PER_MSEC));
}

extern "C" int DG_GetKey(int* pressed, unsigned char* doomKey)
{
	Asgaard::APP.PumpMessages();
	
	spinlock_acquire(&g_queueLock);
	if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
		spinlock_release(&g_queueLock);
		//key queue is empty
		return 0;
	}
	else {
		unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
		
		s_KeyQueueReadIndex++;
		s_KeyQueueReadIndex %= KEYQUEUE_SIZE;
		spinlock_release(&g_queueLock);

		*pressed = keyData >> 8;
		*doomKey = keyData & 0xFF;
		return 1;
	}
}

extern "C" void DG_SetWindowTitle(const char* title)
{
	if (g_window) {
		g_window->UpdateTitle(title);
	}
}
