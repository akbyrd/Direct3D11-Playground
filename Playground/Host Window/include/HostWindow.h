#include "stdafx.h"
#include "Window.h"
#include "GameTimer.h"

class HostWindow final : public Window
{
public:
	bool IsFocused() const;

	bool Initialize();
	void Update();
	void Shutdown();

	void Resize();

protected:
	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM);

private:
	void UpdateFrameStatistics();
	double averageFrameTime = 0;

	LPCWSTR   applicationName = nullptr;
	HINSTANCE hInstance       = nullptr;
	GameTimer gameTimer;

	bool isActive    = false;
	bool isMinimized = false;
	bool isMaximized = false;
	bool isResizing  = false;

	//TODO: Move this into a config struct
	int windowWidth  = 800;
	int windowHeight = 600;
};