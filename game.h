#pragma once
#include "Scene.h"

class Scene;
class RGBColor;
class colorBuffer;
class ImageProcessing;

namespace Tmpl8 {

class Surface;

class Game
{
public:
	void SetTarget( Surface* _Surface ) { screen = _Surface; }
	void SetScene( Scene* _Scene ) { scene = _Scene; }
	void Init();
	void BuildScene();
	void Shutdown(){}
	void Tick( float _DT );
	void MouseUp( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int _Button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int _X, int _Y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp(int _Key);
	void KeyDown(int _Key);

	// The Morton Encoder, given pixel coordinate (x, y), return index in array
	uint32_t morton_encode(uint16_t xPos, uint16_t yPos);
	// The Morton Decoder, given array index, return pixel coordinate (x, y)
	void morton_decode(const uint32_t& index, uint32_t& x_out, uint32_t& y_out);
	
private:
	Surface* screen;
	Scene* scene;
	colorBuffer* buffer;
	colorBuffer* ipbuffer;
	ImageProcessing* imageprocessing;
	int ipindex;
};

}; // namespace Tmpl8