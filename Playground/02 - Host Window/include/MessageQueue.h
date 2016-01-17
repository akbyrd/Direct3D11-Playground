#pragma once

#include <queue>

enum class Message
{
	Quit,

	WindowResizingBegin,
	WindowResizingEnd,
	WindowSizeChanged,
	WindowMinimized,
	WindowUnminimized,
	WindowActive,
	WindowInactive,
	WindowClosed,

	MouseLeftDown,
	MouseLeftUp,
	MouseRightDown,
	MouseRightUp,
	MouseMiddleDown,
	MouseMiddleUp,
	MouseWheelDown,
	MouseWheelUp,
};

class MessageQueue final
{
public:
	struct Pusher
	{
	public:
		Pusher(MessageQueue*);
		void PushMessage(Message);

	private:
		MessageQueue* messageQueue;
	};

	void PushMessage(Message);
	bool PopMessage(Message&);
	Pusher* GetQueuePusher();

private:
	std::queue<Message> queue;
	Pusher queuePusher = Pusher(this);
};