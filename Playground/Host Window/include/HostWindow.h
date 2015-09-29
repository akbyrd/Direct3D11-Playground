#include "stdafx.h"
#include "Window.h"

class HostWindow final : public Window
{
public:
	bool Initialize();
	void Run();
	void Shutdown();

protected:
	virtual LRESULT MessageHandler(UINT, WPARAM, LPARAM);

private:
	LPCWSTR   applicationName = nullptr;
	HINSTANCE hInstance       = nullptr;
};