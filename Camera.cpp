#include "precomp.h"
#include "Camera.h"

// Defult Consturctor
Camera::Camera()
{
	reset();
}
// Construct from eye point, loot-at point, up vector, focal and resolution
Camera::Camera(	const Point3D& eye,
				const Point3D& look,
				const Vector3D& upVec,
				const float aperture_size,
				const float distance,
				const int width,
				const int height,
				const float vp_size)
: e(eye), lookat(look), up(upVec), aperture(aperture_size), d(distance), w(width), h(height), viewplane_size(vp_size)
{
	init_cam();
}
// Copy Constructor
Camera::Camera(const Camera& cam)
:	e(cam.e), lookat(cam.lookat), up(cam.up), aperture(cam.aperture),
	d(cam.d), w(cam.w), h(cam.h), viewplane_size(cam.viewplane_size),
	pixel_count(cam.pixel_count)
{
}
// Destructor
Camera::~Camera()
{
}
// Assignment operator
Camera& Camera::operator= (const Camera& cam)
{
	if (this == &cam)
		return *this;
	e = cam.e;
	lookat = cam.lookat;
	up = cam.up;
	d = cam.d;
	aperture = cam.aperture;
	w = cam.w;
	h = cam.h;
	viewplane_size = cam.viewplane_size;
	pixel_count = cam.pixel_count;
	return *this;
}
// Eye point Setter
void Camera::set_eye(const Point3D& p3d)
{
	e = p3d;
}
// LootAt point setter
void Camera::set_lookat(const Point3D& p3d)
{
	lookat = p3d;
}
// Up vector setter
void Camera::set_up(const Vector3D& upVec)
{
	up = upVec;
}
// Set aperture size
void Camera::set_aperture(const float apertureInput)
{
	aperture = apertureInput;
}
// Set focal length, notice the view plane size need to change as the focal length changes
// Otherwise there will be a zoom in/out effect!
void Camera::set_focal(const float focalInput)
{
	viewplane_size = ((float)viewplane_size * focalInput / d);
	d = focalInput;
	init_cam();
}
// Set resolution
void Camera::set_resolution(const int width, const int height)
{
	w = width;
	h = height;
	pixel_count = w * h;
}
// Set view plane size
void Camera::set_vp_size(const float vp_size)
{
	viewplane_size = vp_size;
	init_cam();
}
// Reset everything to a defult camera
void Camera::reset()
{
	// Set eye point to (0,0,40)
	e = Point3D(0.0f, 0.0f, 40.0f);
	// Set look at point to -Z
	lookat = Point3D(0.0f, 0.0f, 0.0f);
	// Set up-vector to +Y
	up = Vector3D(0.0f, 1.0f, 0.0f);
	// Set aperture size
	aperture = 5.0f;
	// Set focal length
	d = 40.0f;
	// Set resolution
	w = SCRWIDTH; h = SCRHEIGHT;
	pixel_count = w * h;
	// Set view plane size
	viewplane_size = 100;
	// Calculate camera parameters for Shotting Rays
	init_cam();
}
// Get view direction
Vector3D Camera::get_viewDir()
{
	Vector3D dir = lookat - e;
	dir.normalize();
	return dir;
}
// Get pixel size
float Camera::get_pixelSize()
{
	return (float)(viewplane_size / (float)w);
}

// Initialize camera parameters
void Camera::init_cam()
{
	// Camera parameter calculation
	this->viewplane_center = this->e + (float)this->d * this->get_viewDir();
	this->pixel_size = (float)(viewplane_size / (float)w);
	this->strata_size = this->pixel_size / (float)SAMPLEPERPIXEL;
	this->left_up_corner = Point3D(	(float)this->w * this->pixel_size * -0.5f,
									(float)this->h * this->pixel_size * 0.5f,
									(float)this->viewplane_center.z);
}

// Emit a primary Ray according to pixel coordinate and camera parameters
Ray Camera::ray(int& x, int& y)
{
	// Get the Ray start-end points
	Point3D pixelPoint = this->left_up_corner +
						 Vector3D((float)x * this->pixel_size, 0.0f, 0.0f) +
						 Vector3D(0.0f, (float)y * -1.0f * this->pixel_size, 0.0f);
	Vector3D dir = transform * (pixelPoint - this->e);
	Ray ray(transform*this->e, dir);
	
	return ray;
}

// Emit a Sample Ray, with built-in random sample generation
Ray Camera::ray(int& x, int& y, int& spp_root, int& pass)
{
	// Stratification is handled here
	// pass (always) <= spp_root*spp_root;
	int u = pass % spp_root, v = pass / spp_root;
	// Get the sample Ray start-end points, pixel location + randomized strata-size = sample location
	// Sample the pixel
	Point3D pixelPoint = this->left_up_corner +
						 Vector3D((float)x * this->pixel_size, 0.0f, 0.0f) +
						 Vector3D(0.0f, (float)y * -1.0f * this->pixel_size, 0.0f) +
						 Vector3D(((float)v + drand48()) * this->strata_size, 0.0f, 0.0f) +
						 Vector3D(0.0f, ((float)u + drand48()) * this->strata_size, 0.0f);
	// Sample the lens
	Point2D lensSample = random_in_unit_disc() * this->aperture;
	Point3D lensPoint = this->e + lensSample.x * up + lensSample.y * (up^get_viewDir());
	Vector3D dir = transform*(pixelPoint - lensPoint);
	// Apply Camera transformation
	Ray ray(transform*lensPoint, dir);

	return ray;
}