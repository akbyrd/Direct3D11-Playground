#include "stdafx.h"

#include "MessageQueue.h"

MessageQueue::Pusher::Pusher(MessageQueue* messageQueue)
{
	MessageQueue::Pusher::messageQueue = messageQueue;
}

void MessageQueue::Pusher::PushMessage(Message message)
{
	messageQueue->PushMessage(message);
}

MessageQueue::Pusher*
MessageQueue::GetQueuePusher()
{
	return &queuePusher;
}

void MessageQueue::PushMessage(Message message)
{
	queue.push(message);
}

bool MessageQueue::PopMessage(Message &message)
{
	if ( !queue.empty() )
	{
		message = queue.front();
		queue.pop();

		return true;
	}

	return false;
}