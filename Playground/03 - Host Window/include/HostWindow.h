#pragma once

#undef IsMinimized

#include "Window.h"
#include "MessageQueue.h"
#include "Common.h"

class HostWindow final : public Window
{
public:
	struct Input;

	bool   IsActive()    const;
	bool   IsMinimized() const;
	bool   IsResizing()  const;
	HWND   GetHWND()     const;
	Input* GetInput();

	bool Initialize(LPCWSTR applicationName, int iCmdshow,
	                uint16f width, uint16f height,
	                MessageQueue::Pusher* messageQueue);
	void Teardown();

	struct ButtonState
	{
		bool   isDown;
		uint8f transitionCount;
	};

	struct Input
	{
		int16f mouseX;
		int16f mouseY;

		union
		{
			//TODO: Assert size
			ButtonState buttons[5];
			struct
			{
				ButtonState mouseLeft;
				ButtonState mouseRight;
				ButtonState mouseMiddle;
				ButtonState mouseWheelUp;
				ButtonState mouseWheelDown;
			};
		};
	};

protected:
	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM);

private:
	bool ClientSizeToWindowSizeClamped(uint16f&, uint16f&, uint16f, uint16f, DWORD);

	LPCWSTR               applicationName = nullptr;
	HINSTANCE             hInstance       = nullptr;
	MessageQueue::Pusher* messageQueue    = nullptr;

	uint16f minWidth  = 640;
	uint16f minHeight = 480;

	bool isActive    = false;
	bool isResizing  = false;
	bool isMinimized = false;
	bool isMaximized = false;

	Input  input                 = {};
	Input  previousInput         = {};
	SHORT  mouseWheelAccumulator = 0;
};