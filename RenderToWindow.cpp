#include "RenderToWindow.h"
#include "Logger.h"


#pragma region WINDOW DISPLAY

//base code for window display provided Tommi Lipponen
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <stdio.h>

struct RenderTarget {
	HDC device;
	int width;
	int height;
	unsigned *data;
	BITMAPINFO info;

	RenderTarget(HDC givenDevice, int width, int height)
		: device(givenDevice), width(width), height(height), data(nullptr)
	{
		data = new unsigned[unsigned int(width * height)];
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biWidth = width;
		info.bmiHeader.biHeight = height;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 32;
		info.bmiHeader.biCompression = BI_RGB;
	}
	~RenderTarget() { delete[] data; }
	inline int  size() const {
		return width * height;
	}
	void clear(unsigned color) {
		const int count = size();
		for (int i = 0; i < count; i++) {
			data[i] = color;
		}
	}
	inline void pixel(int x, int y, unsigned color) {
		data[y * width + x] = color;
	}
	void present() {
		StretchDIBits(device,
			0, 0, width, height,
			0, 0, width, height,
			data, &info,
			DIB_RGB_COLORS, SRCCOPY);
	}
};

static unsigned
makeColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	unsigned result = 0;
	if (alpha > 0)
	{
		result |= ((unsigned)red << 16);
		result |= ((unsigned)green << 8);
		result |= ((unsigned)blue << 0);
		result |= ((unsigned)alpha << 24);
	}
	return result;
}

static LRESULT CALLBACK
Win32DefaultProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_CLOSE: { PostQuitMessage(0); } break;
	default: { return DefWindowProcW(window, message, wparam, lparam); } break;
	}
	return 0;
}

#pragma endregion


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
}

RenderToWindow::RenderToWindow(color *image, size_t width, size_t height) : width(width), height(height), rt(nullptr)
{
	const wchar_t *const myclass = L"minimalWindowClass";

		WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Win32DefaultProc;
	wc.hInstance = GetModuleHandle(0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = myclass;

	if (RegisterClassExW(&wc))
	{
		DWORD window_style = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
		RECT rc = { 0, 0, width, height };
		if (!AdjustWindowRect(&rc, window_style, FALSE)) 
		{
			Logger::LogError("Couldn't show the image : window rect adjustment failed!"); 
			return; 
		}
		HWND windowHandle = CreateWindowExW(0, wc.lpszClassName,
			L"Output image", window_style, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, GetModuleHandle(NULL), NULL);
		if (!windowHandle) { Logger::LogError("Couldn't show the image : window handle creation was unsuccessful!"); return; }
		ShowWindow(windowHandle, SW_SHOW);

		if (windowHandle)
		{
			HDC device = GetDC(windowHandle);
			rt = new RenderTarget(device, width, height);

			rt->clear(makeColor(0x00,0x00,0x00,0xff));			
		} else 
		{
			Logger::LogError("Couldn't show the image : Couldn't create the window!");
		}
	} else 
	{
		Logger::LogError("Couldn't show the image : Could not register window class!");
	}
}

RenderToWindow::~RenderToWindow()
{
	delete rt;
}

void RenderToWindow::handleMessagesBlocking()
{
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0))
	{
		if (msg.message == WM_QUIT) { return; }
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		rt->present();
	}
}

void RenderToWindow::updateImage(color *image)
{
	if (rt == nullptr) return;


	for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width; x++)
		{
			color &currentColor = image[y * width + x];
			rt->pixel(x, y, makeColor(currentColor.r, currentColor.g, currentColor.b, currentColor.a));
		}

	rt->present();
}
