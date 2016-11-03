#pragma once

class Point3D;
class Vector3D;
class Matrix;

class Camera
{
public:
	// Constructors
	// Defult Consturctor
	Camera();
	// Construct from eye point, loot-at point, up vector, focal and resolution
	Camera(	const Point3D& eye,
			const Point3D& look,
			const Vector3D& upVec,
			const float aperture_size,
			const float distance,
			const int width,
			const int height,
			const float vp_size);
	// Copy Constructor
	Camera(const Camera& cam);
	// Destructor
	~Camera();
	// Assignment operator
	Camera& operator= (const Camera& cam);
	// Eye point Setter
	void set_eye(const Point3D& p3d);
	// LootAt point setter
	void set_lookat(const Point3D& p3d);
	// Up vector setter
	void set_up(const Vector3D& upVec);
	// Set aperture size
	void set_aperture(const float apertureInput);
	// Set focal length
	void set_focal(const float focalInput);
	// Set resolution
	void set_resolution(const int width, const int height);
	// Set view plane size
	void set_vp_size(const float vp_size);
	// Reset everything to a defult camera
	void reset();
	// Get camera resolution, w
	int get_w(){ return w; }
	// Get camera resolution, h
	int get_h(){ return h; }
	// Get camera pixel count, pixel_count
	int get_p_count(){ return pixel_count; }
	// Get eye point
	Point3D get_e(){ return e; }
	// Get view direction
	Vector3D get_viewDir();
	// Get up direction
	Vector3D get_up(){ return up; }
	// Get view plane size, which only corresponds to width
	float get_vp_size(){ return viewplane_size; };
	// Get focal length
	float get_d(){ return d; }
	// Get pixel size
	float get_pixelSize();
	// Get aperature
	float get_aperture() { return aperture; }
	
	// Initialize camera parameters
	void init_cam();
	// Emit a primary Ray according to pixel coordinate and camera parameters
	Ray ray(int& x, int& y);
	// Emit a Sample Ray, with built-in random sample generation
	// Notice that the parameter spp_root is a corrected sqrt of sample_per_pixel
	// i.e. spp_root = ceil(sqrtf(sample_per_pixel))
	Ray ray(int& x, int& y, int& spp_root, int& pass);
	// camera transformation matrix
	Matrix transform;

protected:
	// Eye Point3D
	Point3D e;
	// Look-At Point3D
	Point3D lookat;
	// Up Vector3D
	Vector3D up;
	// Focal length
	float d;
	// Lens aperture size
	float aperture;
	// Screen resolution
	int w, h;
	// Total pixel count
	int pixel_count;
	// View plane size
	float viewplane_size;
	// Parameters for Ray generation
	Point3D viewplane_center;
	Point3D left_up_corner;
	float pixel_size;
	float strata_size;
};