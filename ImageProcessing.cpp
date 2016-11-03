#include "precomp.h"
#include "ImageProcessing.h"
#include "glm/glm.hpp"
#include <algorithm>

ImageProcessing::ImageProcessing() {
	gauss_blur[0] = 0.0947416;
	gauss_blur[1] = 0.118318;
	gauss_blur[2] = 0.0947416;
	gauss_blur[3] = 0.118318;
	gauss_blur[4] = 0.147761;
	gauss_blur[5] = 0.118318;
	gauss_blur[6] = 0.0947416;
	gauss_blur[7] = 0.118318;
	gauss_blur[8] = 0.0947416;

	circle[0] = glm::vec2(-2, 0);
	circle[1] = glm::vec2(-1, -1);
	circle[2] = glm::vec2(-1, 0);
	circle[3] = glm::vec2(-1, 1);
	circle[4] = glm::vec2(0, -2);
	circle[5] = glm::vec2(0, -1);
	circle[6] = glm::vec2(0, 0);
	circle[7] = glm::vec2(0, 1);
	circle[8] = glm::vec2(0, 2);
	circle[9] = glm::vec2(1, -1);
	circle[10] = glm::vec2(1, 0);
	circle[11] = glm::vec2(1, 1);
	circle[12] = glm::vec2(2, 0);

	render_count = 8;
	gray_threshold = 0.05;
	memset(vis, 0, sizeof(vis));

	dis_x[0] = 0;		dis_y[0] = 1;
	dis_x[1] = 0;		dis_y[1] = -1;
	dis_x[2] = 1;		dis_y[2] = 1;
	dis_x[3] = 1;		dis_y[3] = 0;
	dis_x[4] = 1;		dis_y[4] = -1;
	dis_x[5] = -1;		dis_y[5] = 1;
	dis_x[6] = -1;		dis_y[6] = 0;
	dis_x[7] = -1;		dis_y[7] = -1;
}

void ImageProcessing::median_filter(int width, int height,
	colorBuffer* buffer, colorBuffer* ipbuffer) {
	ipbuffer->one_more_pass();
#pragma omp parallel for
	for (int y = 1; y < height - 1; y++) {
#pragma omp parallel for
		for (int x = 1; x < width - 1; x++) {
			int k = 0;
			RGBColor mask[9];
			for (int yy = y - 1; yy <= y + 1; ++yy) {
				for (int xx = x - 1; xx <= x + 1; ++xx) {
					mask[k] = buffer->get_color(xx, yy);
					k++;
				}
			}
			RGBColor result;
			std::sort(&mask[0], &mask[8], [=](RGBColor a, RGBColor b) {
				return a.r < b.r; });
				result.r = mask[4].r;
				std::sort(&mask[0], &mask[8], [=](RGBColor a, RGBColor b) {
					return a.g < b.g; });
					result.g = mask[4].g;
					std::sort(&mask[0], &mask[8], [=](RGBColor a, RGBColor b) {
						return a.b < b.b; });
						result.b = mask[4].b;
						ipbuffer->add_to_buffer(result, x, y);
						//scene->Plot(x, y, result.out());
		}
	}
}

void ImageProcessing::brush_paint(int w, int h, colorBuffer* buffer,
	colorBuffer* ipbuffer) {

	width = w;
	height = h;
	ipbuffer->one_more_pass();

	//gauss blur
#pragma omp parallel for
	for (int y = 1; y < height - 1; y++) {
#pragma omp parallel for
		for (int x = 1; x < width - 1; x++) {
			RGBColor result;
			int k = 0;
			for (int yy = y - 1; yy <= y + 1; ++yy)
			for (int xx = x - 1; xx <= x + 1; ++xx) {
				result += buffer->get_color(xx, yy)*gauss_blur[k];
				k++;
			}
			b_image[y][x] = result;
			r_image[y][x] = result;
		}
	}

	int random_count = (int)(width / (1 + render_count) / 7 * height);
	int st_deep = (render_count + 1) * 10;
	int radius = (render_count + 1) * 1 + 1;

	/*int random_count = (int)(width / (1 + render_count)  * height);
	int st_deep = (render_count + 1) * 10;
	int radius = (render_count + 1) * 2 + 1;
	*/
	while (random_count--) {
		memset(vis, 0, sizeof(vis));
		int n_deep = st_deep;
		BrushPoint ori_point;
		ori_point.p_x = rand() % width;
		ori_point.p_y = rand() % height;
		ori_point.deep = 0;
		if (!check_pos(ori_point.p_x, ori_point.p_y)) continue;
		BrushPoint s_point;
		s_point.copy(ori_point);
		while (n_deep--) {
			for (int i = 0; i < 8; ++i) {
				int index_x = s_point.p_x + dis_y[i];
				int index_y = s_point.p_y + dis_x[i];
				if (!check_pos(index_x, index_y)) continue;
				if (!vis[index_x][index_y])
					vis[index_x][index_y] = true;
				else continue;
				if (check_gray(ori_point.p_x, ori_point.p_y, index_x, index_y)) {
					stroke(index_x, index_y, radius);
					s_point.p_x = index_x;
					s_point.p_y = index_y;
					s_point.deep = 0;
					break;
				}
			}
		}
		//draw_line(glm::vec2(ori_point.p_x,ori_point.p_y), glm::vec2(s_point.p_x, s_point.p_y), radius);
	}

#pragma omp parallel for
	for (int y = 1; y < height - 1; y++) {
#pragma omp parallel for
		for (int x = 1; x < width - 1; x++) {
			if (r_image[y][x] == 0.0) ipbuffer->add_to_buffer(b_image[y][x], x, y);
			else ipbuffer->add_to_buffer(r_image[y][x], x, y);
			//scene->Plot(x, y, r_image[y][x].out());
		}
	}
}

void ImageProcessing::stroke(int x, int y, int rad) {
	float r = b_image[y][x].r;
	float g = b_image[y][x].g;
	float b = b_image[y][x].b;
	for (int i = (rad - rad / 2) - rad; i < (rad + rad / 2) - rad; ++i) {
		int rand_x = rand() % 5;
		if (rand_x == 0) break;
		for (int j = (rad - rad / 2) - rad; j < (rad + rad / 2) - rad; ++j) {
			int rand_y = rand() % 5;
			if (rand_y == 0) break;
			if (check_pos(x + i, y + j)) {
				r_image[y + j][x + i].r = r;
				r_image[y + j][x + i].g = g;
				r_image[y + j][x + i].b = b;
			}
		}
	}
}

void ImageProcessing::draw_line(glm::vec2 p1, glm::vec2 p2, int rad) {
	float r = b_image[(int)p1.y][(int)p1.x].r;
	float g = b_image[(int)p1.y][(int)p1.x].g;
	float b = b_image[(int)p1.y][(int)p1.x].b;
	glm::vec2 direction = p2 - p1;
	float t = std::sqrtf(glm::dot(direction, direction));
	direction = glm::normalize(direction);
	float k = 0;
	while (k <= t) {
		glm::vec2 centre = p1 + k*direction;
		for (int i = 0; i < 13; ++i) {
			glm::vec2 cur = centre + circle[i];
			if (check_pos((int)cur.x, (int)cur.y)) {
				r_image[(int)cur.y][(int)cur.x].r = r;
				r_image[(int)cur.y][(int)cur.x].g = g;
				r_image[(int)cur.y][(int)cur.x].b = b;
			}
		}
		k += 1;
	}
}

bool ImageProcessing::check_pos(int i, int j) {
	if (j >= 0 && i >= 0 && i <= width - 1 && j <= height - 1) return true;
	return false;
}

bool ImageProcessing::check_gray(int o_i, int o_j, int r_i, int r_j) {
	float o_gray = float(0.3*b_image[o_j][o_i].r + 0.59*b_image[o_j][o_i].g + 0.11*b_image[o_j][o_i].b);
	float r_gray = float(0.3*b_image[r_j][r_i].r + 0.59*b_image[r_j][r_i].g + 0.11*b_image[r_j][r_i].b);
	if (fabs(o_gray - r_gray) < gray_threshold) return true;
	return false;
}

void ImageProcessing::color_disperse(int width, int height, colorBuffer* buffer,
	colorBuffer* ipbuffer) {
	ipbuffer->one_more_pass();

	int r_width = width, r_height = height;
	int g_width = (int)(0.99*width), g_height = (int)(0.99*height);
	int b_width = (int)(0.985*width), b_height = (int)(0.985*height);

	for (int y = 0; y < g_height; ++y) {
		for (int x = 0; x < g_width; ++x) {
			r_pic[y][x] = buffer->get_color(x, y);
			r_pic[y][x].g = 0;
			r_pic[y][x].b = 0;
		}
	}

	float f_g_w = (float)r_width / g_width;
	float f_g_h = (float)r_height / g_height;
	int x0, y0;
	for (int y = 0; y < g_height; ++y) {
		y0 = (int)(y*f_g_h);
		for (int x = 0; x < g_width; ++x) {
			x0 = (int)(x*f_g_w);
			g_pic[y][x] = buffer->get_color(x0, y0);
			g_pic[y][x].r = 0;
			g_pic[y][x].b = 0;
		}
	}

	float f_b_w = (float)r_width / b_width;
	float f_b_h = (float)r_height / b_height;
	int x1, y1;
	for (int y = 0; y < b_height; ++y) {
		y1 = (int)(y*f_b_h);
		for (int x = 0; x < b_width; ++x) {
			x1 = (int)(x*f_b_w);
			b_pic[y][x] = buffer->get_color(x1, y1);
			b_pic[y][x].r = 0;
			b_pic[y][x].g = 0;
		}
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			RGBColor cur(r_pic[y][x].r, g_pic[y][x].g, b_pic[y][x].b);
			if (cur.g == 0.0 || cur.b == 0.0 || cur.r == 0.0)
				cur = buffer->get_color(x, y);
			ipbuffer->add_to_buffer(cur, x, y);
		}
	}
}

void ImageProcessing::barrel_distortion(int width, int height, colorBuffer* buffer,
	colorBuffer* ipbuffer) {

	ipbuffer->one_more_pass();

	float alpha = 1.5;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float yy = (float)y / height;
			float xx = (float)x / width;
			glm::vec2 r = glm::vec2(xx - 0.5, yy - 0.5);
			float r_judge = glm::dot(r, r);
			glm::vec2 nr = r*(1 + alpha*glm::dot(r, r));
			if (nr.x <= 0.5 && nr.y <= 0.5) {
				int x2 = (int)((nr.x + 0.5) *width);
				int y2 = (int)((nr.y + 0.5) *height);
				if (x2 >= 0 && x2 <= width && y2 >= 0 && y2 <= height) {
					d_pic[y][x] = buffer->get_color(x2, y2);
					ipbuffer->add_to_buffer(d_pic[y][x], x, y);
				}
			}
		}
	}

}

bool ImageProcessing::save_image(int width, int height, colorBuffer* ipbuffer) {
	//FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(width, height, 32);
	float inver = float(std::pow(2, 8));
	int index = 0;
	RGBQUAD value;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			RGBColor cur = ipbuffer->get_color(x, y);
			float q_r = BYTE(cur.r * 255);
			float q_g = BYTE(cur.g * 255);
			float q_b = BYTE(cur.b * 255);
			value.rgbRed = BYTE(q_r);
			value.rgbGreen = BYTE(q_g);
			value.rgbBlue = BYTE(q_b);
			FreeImage_SetPixelColor(bitmap, x, SCRHEIGHT - y, &value);
		}
	}
	FreeImage_Save(FIF_PNG, bitmap, "image.png", 0);
	//FreeImage_DeInitialise();
	return true;
}

/*  original image , for saving  */
void ImageProcessing::original_image(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer) {
	ipbuffer->one_more_pass();
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			ipbuffer->add_to_buffer(buffer->get_color(x, y), x, y);
		}
	}
}

/*  high light  */
void ImageProcessing::hight_light(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer) {
	ipbuffer->one_more_pass();
	// high light part
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			RGBColor color = buffer->get_color(x, y);
			color.r = color.r < 1.0f ? 0.0f : color.r - 1.0f;
			color.g = color.g < 1.0f ? 0.0f : color.g - 1.0f;
			color.b = color.b < 1.0f ? 0.0f : color.b - 1.0f;
			high_pic[y][x] = color;
		}
	}

	//average blur
	for (int y = 3; y < height - 3; y++) {
		for (int x = 3; x < width - 3; x++) {
			RGBColor result;
			int k = 0;
			for (int yy = y - 3; yy <= y + 3; ++yy)
			for (int xx = x - 3; xx <= x + 3; ++xx) {
				result += high_pic[yy][xx] * 0.0204;
				k++;
			}
			blur_pic[y][x] = result;
		}
	}

	// add gauss blur to high light part
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			res_pic[y][x] = blur_pic[y][x] + buffer->get_color(x, y);
			ipbuffer->add_to_buffer(res_pic[y][x], x, y);
		}
	}
}

// lomo, make the margin darker
void ImageProcessing::lomo(int width, int height, colorBuffer* buffer, colorBuffer* ipbuffer) {
	ipbuffer->one_more_pass();
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			float yy = float(2.0f * ((float)y / height - 0.5f));
			float xx = float(2.0f * ((float)x / width - 0.5f));
			float distance = float(sqrtf(xx*xx + yy*yy));
			RGBColor color = buffer->get_color(x, y);
			color.r = (color.r >= 1.0f) ? 0.9999f : color.r;
			color.g = (color.g >= 1.0f) ? 0.9999f : color.g;
			color.b = (color.b >= 1.0f) ? 0.9999f : color.b;
			color.r = (color.r < 0.0f) ? 0.0f : color.r;
			color.g = (color.g < 0.0f) ? 0.0f : color.g;
			color.b = (color.b < 0.0f) ? 0.0f : color.b;
			if (distance > 0.5f) {
				lomo_pic[y][x] = (1.5f - distance)*color;
			}
			else {
				lomo_pic[y][x] = buffer->get_color(x, y);
			}
			ipbuffer->add_to_buffer(lomo_pic[y][x], x, y);
		}
	}
}