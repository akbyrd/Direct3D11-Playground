#pragma once

class GraphicsClass final
{
public:
	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();
};