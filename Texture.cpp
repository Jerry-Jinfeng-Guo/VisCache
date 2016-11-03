#include <math.h>
#include "precomp.h"
#include "Texture.h"

RGBColor CheckerTex::value(const float u, const float v, const Point3D& p3d)
{
	float sineSign = sinf(checkerSize*p3d.x) * sinf(checkerSize*p3d.y) * sinf(checkerSize*p3d.z);
	if (sineSign < 0)
	{
		return this->light;
	}
	else
	{
		return this->dark;
	}
}

static Vector3D* perlin_generate()
{
	Vector3D *p = new Vector3D[256];
	for (int i = 0; i < 256; ++i)
	{
		Vector3D vec = Vector3D(2.0f*drand48() - 1.0f, 2.0f*drand48() - 1.0f, 2.0f*drand48() - 1.0f);
		vec.normalize();
		p[i] = vec;
	}
	return p;
}

void permute(int *p, int n)
{
	for (int i = n - 1; i > 0; i--)
	{
		int target = int(drand48()*(i + 1));
		int tmp = p[i];
		p[i] = p[target];
		p[target] = tmp;
	}
}

static int* perlin_generate_perm()
{
	int *p = new int[256];
	for (int i = 0; i < 256; i++)
	{
		p[i] = i;
	}
	permute(p, 256);
	return p;
}

Perlin::Perlin()
{
	this->ranVec = perlin_generate();
	this->perm_x = perlin_generate_perm();
	this->perm_y = perlin_generate_perm();
	this->perm_z = perlin_generate_perm();
}

float Perlin::turb(const Point3D& p3d) const
{
	float accum = 0.0f;
	Point3D temp_vec = p3d;
	float weight = 1.0f;
	for (int i = 0; i < 7; i++)
	{
		accum += weight * noise(temp_vec);
		weight *= 0.5f;
		temp_vec = 2.0f * temp_vec;
	}
	return fabs(accum);
}

float trilinear_initerp(float c[2][2][2], float u, float v, float w)
{
	float accum = 0.0f;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				accum   +=	(i*u + (1 - i)*(1 - u))*
							(j*v + (1 - j)*(1 - v))*
							(k*w + (1 - k)*(1 - w))*
							c[i][j][k];
			}
		}
	}
	return accum;
}

float perlin_initerp(Vector3D c[2][2][2], float u, float v, float w)
{
	float uu = u*u*(3.0f - 2.0f*u);
	float vv = v*v*(3.0f - 2.0f*v);
	float ww = w*w*(3.0f - 2.0f*w);
	float accum = 0.0f;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector3D weight_v(u - i, v - j, w - k);
				accum += (i*uu + (1 - i)*(1 - uu))*
						 (j*vv + (1 - j)*(1 - vv))*
						 (k*ww + (1 - k)*(1 - ww))*
						 (c[i][j][k] * weight_v);
			}
		}
	}
	return accum;
}

float Perlin::noise(const Point3D& p3d) const
{
	float u = p3d.x - floor(p3d.x);	
	float v = p3d.y - floor(p3d.y);	
	float w = p3d.z - floor(p3d.z);
	int i = (int)floor(p3d.x);
	int j = (int)floor(p3d.y);
	int k = (int)floor(p3d.z);
	Vector3D c[2][2][2];
	for (int di = 0; di < 2; di++)
	{
		for (int dj = 0; dj < 2; dj++)
		{
			for (int dk = 0; dk < 2; dk++)
			{
				c[di][dj][dk] = ranVec[	perm_x[(i + di) & 255] ^
										perm_y[(j + dj) & 255] ^
										perm_z[(k + dk) & 255] ];
			}
		}
	}
	return perlin_initerp(c, u, v, w);
}

// Construct from an image file
ImageTex::ImageTex(const char* filePath)
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
			RGBColor pixel(r, g, b);
			pixel.un_correct_gamma();
			image[x][y] = pixel;
		}
	}

	FreeImage_Unload(bitmap);
}

RGBColor ImageTex::value(const float u, const float v, const Point3D& p3d)
{
	int pixel_u = int(width*u);
	int pixel_v = int(height*v);
	return image[pixel_u][pixel_v];
}