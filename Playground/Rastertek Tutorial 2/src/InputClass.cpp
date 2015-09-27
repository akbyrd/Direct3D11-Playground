#include "stdafx.h"
#include "InputClass.h"

bool InputClass::Initialize()
{
	//Initialize all the keys to being released and not pressed
	for ( size_t i = 0; i < 256; ++i )
		keys[i] = false;

	return true;
}

void InputClass::KeyDown(unsigned int keyIndex)
{
	keys[keyIndex] = true;
}

void InputClass::KeyUp(unsigned int keyIndex)
{
	keys[keyIndex] = false;
}

bool InputClass::IsKeyDown(unsigned int keyIndex)
{
	return keys[keyIndex];
}