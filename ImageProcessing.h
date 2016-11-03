#pragma once
#include "glm/glm.hpp"

class Scene;
class colorBuffer;
class RGBColor;

struct BrushPoint
{
public:
	BrushPoint(int a, int b, int c) {
		p_x = a;
		p_y = b;
		deep = c;
	}

	BrushPoint() {
		p_x = 0;
		p_y = 0;
		deep = 0;
	}
	void copy(BrushPoint a) {
		this->p_x = a.p_x;
		this->p_y = a.p_y;
		this->deep = a.deep;
	}
	BrushPoint operator+(const BrushPoint& p) const {
		return BrushPoint(this->p_x + p.p_x, this->p_y + p.p_y, this->deep + p.deep);
	}

	int p_x;
	int p_y;
	int deep;

};

class ImageProcessing {
public:
	ImageProcessing();

	/*   median filter  */
	void median_filter(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/*   brush paint   */
	void brush_paint(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);
	bool check_gray(int o_i, int o_j, int r_i, int r_j);
	bool check_pos(int i, int j);
	void stroke(int x, int y, int rad);
	void draw_line(glm::vec2 a, glm::vec2 b, int rad);

	/*  color disperse  */
	void color_disperse(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/*  barrel distortion  */
	void barrel_distortion(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/*  origin image  */
	void original_image(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/*  high light  */
	void hight_light(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/*  lomo  */
	void lomo(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer);

	/* save image */
	bool save_image(int width, int height, colorBuffer* ipbuffer);


private:

	/*   brush paint  */
	int render_count;
	float gauss_blur[9];
	RGBColor b_image[4096][4096];
	RGBColor r_image[4096][4096];
	bool vis[4096][4096];
	float gray_threshold;
	int dis_x[8];
	int dis_y[8];
	glm::vec2 circle[13];
	int width, height;

	/*  color disperse  */
	RGBColor r_pic[4096][4096];
	RGBColor g_pic[4096][4096];
	RGBColor b_pic[4096][4096];

	/*  barrel distortion  */
	RGBColor d_pic[4096][4096];

	/*  high light  */
	RGBColor blur_pic[4096][4096];
	RGBColor high_pic[4096][4096];
	RGBColor res_pic[4096][4096];

	/*  lomo  */
	RGBColor lomo_pic[4096][4096];
};