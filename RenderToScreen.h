#ifndef RENDER_TO_SCREEN_H_DEFINED
#define RENDER_TO_SCREEN_H_DEFINED

union color;
struct RenderTarget;


class RenderToScreen
{
public:
	RenderToScreen(color *image, size_t width, size_t height);
	~RenderToScreen();

	void handleMessagesBlocking();
	void updateImage(color *image);
private:
	size_t width, height;
	RenderTarget *rt;
};

#endif