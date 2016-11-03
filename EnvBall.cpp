#include "precomp.h"
#include "EnvBall.h"

// Construct from an image file
EnvBall::EnvBall(const char* filePath)
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filePath);
	FIBITMAP *bitmap = FreeImage_Load(fif, filePath);
	
	width = FreeImage_GetWidth(bitmap);
	height = FreeImage_GetHeight(bitmap);
	int bpp = FreeImage_GetBPP(bitmap);
	
	// The PNG file is itself 24 bit, i.e. 8 * 3, no alpha
	// now is 4 including alpha, which is interesting
	float inver = 1.0f / (float)pow(2, (bpp / 4));

	for (int y = 0; y < (int)height; y++)
	{
		for (int x = 0; x < (int)width; x++)
		{
			RGBQUAD color;
			FreeImage_GetPixelColor(bitmap, x, height - y, &color);

			float r, g, b;
			// Map color back to [0, 1] using image color depth info 

			r = (float)color.rgbRed * inver;
			g = (float)color.rgbGreen * inver;
			b = (float)color.rgbBlue * inver;

			texture[x][y] = RGBColor(r, g, b);
		}
	}

	FreeImage_Unload(bitmap);
}

// Destructor
EnvBall::~EnvBall()
{}