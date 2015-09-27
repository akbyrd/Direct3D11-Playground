#pragma once

class InputClass final
{
public:
	bool Initialize();

	bool IsKeyDown(unsigned int);
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

private:
	bool keys[256];
};