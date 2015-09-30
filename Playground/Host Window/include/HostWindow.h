#include "stdafx.h"
#include "Window.h"
#include "GameTimer.h"

class HostWindow final : public Window
{
public:
	bool Initialize();
	void Update();
	void Shutdown();

protected:
	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM);

private:
	void UpdateFrameStatistics();
	double averageFrameTime = 0;

	LPCWSTR   applicationName = nullptr;
	HINSTANCE hInstance       = nullptr;
	GameTimer gameTimer;
};