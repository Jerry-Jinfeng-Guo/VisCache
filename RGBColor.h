#pragma once

class RGBColor
{
public:
	union { struct{ float r, g, b; }; float cell[3]; };
public:
	// Constructors
	// Defult Constructor, gives a black color
	inline RGBColor::RGBColor(){ r = 0.0f; g = 0.0f; b = 0.0f; }
	// Construct from one float value
	inline RGBColor::RGBColor(float value){ r = value; g = value; b = value; }
	// Construct from three float values
	inline RGBColor::RGBColor(float r_value, float g_value, float b_value) { r = r_value; g = g_value; b = b_value; }
	// Copy Constructor
	inline RGBColor::RGBColor(const RGBColor& color){ r = color.r; g = color.g; b = color.b;}
	// Destructor
	~RGBColor();

	// Assignment
	RGBColor& operator=  (const RGBColor& color);
	// Addition
	RGBColor  operator+  (const RGBColor& color) const;
	// Compound Addition
	RGBColor& operator+= (const RGBColor& color);
	// Multiplication by a float on the right
	RGBColor  operator*  (const float value) const;
	// Compund Multiplication
	RGBColor& operator*= (const float value);
	// Division by a float
	RGBColor  operator/  (const float value) const;
	// Compound Division
	RGBColor& operator/= (const float value);
	// (R1*R2, G1*G2, B1*B2) by two RGBColor
	RGBColor  operator*  (const RGBColor& color) const;
	// Returns if two RGBColor are equal
	bool operator== (const RGBColor& color) const;
	// If black/white
	inline bool is_black(){ return this->r <= 0.0f || this->g <= 0.0f || this->b <= 0.0f; };
	inline bool is_white(){ return this->r >= 1.0f || this->g >= 1.0f || this->b >= 1.0f; };
	// Max/min color value
	inline float max_channel(){ return this->r > this->g && this->r > this->b ? this->r : this->g > this->b ? this->g : this->b; }
	//inline float min_channel(){}
	// The power of the color
	inline float power(){ return this->r * this->r + this->g * this->g + this->b * this->b; }
	// Raise R,G,B to a power
	RGBColor powColor(float power) const;
	// Two simple Gamma correction methods, for explicit gamma value, use powColor()
	void correct_gamma();
	void un_correct_gamma();
	// Return the average color value
	float average() const;
	// Output a Pixel color to SDL
	long out() const;
};

// Non-member functions
// Multiplication by a float on the left
RGBColor operator* (const float value, const RGBColor& color);

class colorBuffer
{
private:
	int counter;
	RGBColor buffer[SCRWIDTH][SCRHEIGHT];

public:
	colorBuffer();
	~colorBuffer(){}
	void one_more_pass(){ counter++; }
	void add_to_buffer(RGBColor color, int x, int y);
	RGBColor get_color(int x, int y);
	void reset_buffer();
	int get_count(){ return this->counter; }
};