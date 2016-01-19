#pragma once

#undef IsMinimized

#include "Window.h"
#include "MessageQueue.h"
#include "Common.h"

class HostWindow final : public Window
{
public:
	bool   IsActive()      const;
	bool   IsMinimized()   const;
	bool   IsResizing()    const;
	HWND   GetHWND()       const;
	POINTS MousePosition() const;

	bool Initialize(LPCWSTR applicationName, int iCmdshow, int width, int height, MessageQueue::Pusher* messageQueue);
	void Teardown();

protected:
	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM);

private:
	bool ClientSizeToWindowSize(int&, int&, int, int, DWORD);

	LPCWSTR               applicationName = nullptr;
	HINSTANCE             hInstance       = nullptr;
	MessageQueue::Pusher* messageQueue    = nullptr;

	int minWidth  = 640;
	int minHeight = 480;

	bool isActive    = false;
	bool isResizing  = false;
	bool isMinimized = false;
	bool isMaximized = false;

	POINTS mousePosition;
};