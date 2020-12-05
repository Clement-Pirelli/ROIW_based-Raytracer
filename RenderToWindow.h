#ifndef RENDER_TO_WINDOW_H_DEFINED
#define RENDER_TO_WINDOW_H_DEFINED
#include "color.h"
struct RenderTarget;


class RenderToWindow
{
public:
	RenderToWindow(color *image, size_t width, size_t height);
	~RenderToWindow();

	void handleMessagesBlocking();
	void updateImage(color *image);
private:
	size_t width, height;
	RenderTarget *rt;
};

#endif