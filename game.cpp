#include "precomp.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

extern glm::mat4 GLOBAL_transform = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 1.0));
// -----------------------------------------------------------
// Key control
// -----------------------------------------------------------
/* Key Instructions
	A: rotate left
	D: rotate right
	W: rotate up
	S: rotate down
	arrow-up: move front
	arrow-down: move back
	arrow-left: move left
	arrow-right: move right
	K: Focus further
	L: Focus closer (>0.0f)
	N: Aperture up (<5.0f)
	M: Aperture down (>0.0f)
	Anyother Key: reset render
*/
void Game::KeyUp(int _Key)
{
	if (ipindex == 0)	buffer->reset_buffer();
}
void Game::KeyDown(int _Key)
{
	/// Comment this out if you want to see some motion blur during camera transformation, a little trick :)
	if (_Key == SDL_SCANCODE_A)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::rotate(GLOBAL_transform, (GLfloat)-0.02745, glm::vec3(0.0, 1.0, 0.0));
	}
	else if (_Key == SDL_SCANCODE_D)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::rotate(GLOBAL_transform, (GLfloat)0.02745, glm::vec3(0.0, 1.0, 0.0));
	}
	else if (_Key == SDL_SCANCODE_W)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::rotate(GLOBAL_transform, (GLfloat)-0.02745, glm::vec3(1.0, 0.0, 0.0));
	}
	else if (_Key == SDL_SCANCODE_S)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::rotate(GLOBAL_transform, (GLfloat)0.02745, glm::vec3(1.0, 0.0, 0.0));
	}
	else if (_Key == SDL_SCANCODE_LEFT)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::translate(GLOBAL_transform, glm::vec3(-0.5f, 0.0f, 0.0f));
	}
	else if (_Key == SDL_SCANCODE_RIGHT)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::translate(GLOBAL_transform, glm::vec3(0.5f, 0.0f, 0.0f));
	}
	else if (_Key == SDL_SCANCODE_UP)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::translate(GLOBAL_transform, glm::vec3(0.0f, 0.0f, -0.5f));
	}
	else if (_Key == SDL_SCANCODE_DOWN)
	{
		buffer->reset_buffer();
		GLOBAL_transform = glm::translate(GLOBAL_transform, glm::vec3(0.0f, 0.0f, 0.5f));
	}
	else if (_Key == SDL_SCANCODE_K)
	{
		buffer->reset_buffer();
		scene->get_cam()->set_focal(scene->get_cam()->get_d() + 2.0f);
	}
	else if (_Key == SDL_SCANCODE_L)
	{
		buffer->reset_buffer();
		if (scene->get_cam()->get_d() > 4.0f) {
			scene->get_cam()->set_focal(scene->get_cam()->get_d() - 2.0f);
		}
	}
	else if (_Key == SDL_SCANCODE_N)
	{
		buffer->reset_buffer();
		if (scene->get_cam()->get_aperture() < 5.0f) {
			scene->get_cam()->set_aperture(scene->get_cam()->get_aperture() + 0.1f);
		}
	}
	else if (_Key == SDL_SCANCODE_M)
	{
		buffer->reset_buffer();
		if (scene->get_cam()->get_aperture() > 0.0f) {
			scene->get_cam()->set_aperture(scene->get_cam()->get_aperture() - 0.1f);
		}
	}
	else if (_Key == SDL_SCANCODE_1)
	{
		ipindex = 1;
	}
	else if (_Key == SDL_SCANCODE_2)
	{
		ipindex = 2;
	}
	else if (_Key == SDL_SCANCODE_3)
	{
		ipindex = 3;
	}
	else if (_Key == SDL_SCANCODE_4)
	{
		ipindex = 4;
	}
	else if (_Key == SDL_SCANCODE_5)
	{
		ipindex = 5;
	}
	else if (_Key == SDL_SCANCODE_6)
	{
		ipindex = 6;
	}
	else if (_Key == SDL_SCANCODE_9)
	{
		ipindex = 7;
	}
	else if (_Key == SDL_SCANCODE_0)
	{
		ipindex = 8;
	}
	else
	{
		buffer->reset_buffer();
	}
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
	// Clear the screen and build the scene
	screen->Clear(0);
	BuildScene();
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float _DT )
{
	/// Time counter
	clock_t startTime = clock();
	screen->Clear(0);
	/// Camera transformation - the Matrix
	for (int ti = 0; ti < 4; ++ti)
		for (int tj = 0; tj < 4; ++tj)
			scene->get_cam()->transform.m[ti][tj] = GLOBAL_transform[tj][ti];
	/// Screen display information
	char info[128];
	
	/// Into the Tracer, choose between RayTracing (single/packet) and PathTracing
#if 0						// <<<------- Switch between Ray tracing and Path Tracing ------<<-
	/// Ray Tracing
	/// Single Ray VS Packet Traversal
#if 1							// <<<--------- Switch between single ray and ray packet -------<<-
	/// Loop through all pixels, hit the Scene with Rays, Single Ray Mode
#pragma omp parallel for
	for (int y = 0; y < scene->get_cam()->get_h(); y++)
	{
#pragma omp parallel for
		for (int x = 0; x < scene->get_cam()->get_w(); x++)
		{													//	512*512		256*256
			/// Shot a Ray given the pixel coordinate		//		2ms		1ms
			Ray ray = scene->get_cam()->ray(x, y);			//		20ms	5ms
			Tracer tracer = Tracer(scene, ray);				//		25ms	7ms
			RGBColor color = tracer.trace();				//		440ms	120ms
			screen->Plot(x, y, color.out());				//		450ms	120ms
		}
	}
	sprintf(info, "single ray mode");
	screen->Print(info, (SCRWIDTH / 2) - 50, 20, 0xffff00);

#else
	/// Loop through all packets, hit the Scene with Packets, Ray Packet Mode
	uint32_t	packetCount = scene->get_cam()->get_w() * scene->get_cam()->get_h() / RAY_PACKET_SIZE;
	uint32_t	frustum01 = 0, frustum02 = morton_encode(RAY_PACKET_SQRT - 1, 0),
				frustum03 = morton_encode(0, RAY_PACKET_SQRT - 1), frustum04 = RAY_PACKET_SIZE - 1;

#pragma omp parallel for
	for (int p = 0; p < (int)packetCount; ++p)		// omp loves int...
	{
		RayPacket rayPacket;
		rayPacket.firstActive = 0;
		/// Can't use parallel here because order is crucial
		for (uint32_t r = 0; r < RAY_PACKET_SIZE; ++r)
		{
			/// Shot RAY_PACKET_SIZE Ray's according to						512*512	256*256
			/// the pixel coordinate decoded from index					//		1ms		1ms
			uint32_t u, v;												//		12ms	1ms
			uint32_t index = (uint32_t)p * RAY_PACKET_SIZE + r;			//		12ms	1ms
			morton_decode(index, u, v);									//		15ms	2ms
			int x = (int)u, y = (int)v;									//		15ms	2ms
			Ray ray = scene->get_cam()->ray(x, y);						//		35ms	5ms
			rayPacket.rays[r] = ray;									//		35ms	6ms
			RGBColor color = scene->get_env_color(ray);					//		50ms	10ms
			rayPacket.colors[r] = color;								//		50ms	10ms
			rayPacket.hitPoints[r] = HitPoint();
		}
		
		/// Store the four rays determine the frustrum
		rayPacket.frustum[0] = frustum01;	rayPacket.frustum[1] = frustum02;
		rayPacket.frustum[2] = frustum03;	rayPacket.frustum[3] = frustum04;
		
		/// Now traverse use the RayPacket
		Tracer tracer = Tracer(scene);
		tracer.trace_packet(rayPacket);

		/// Now plot results to screen
#pragma omp parallel for
		for (int i = 0; i < RAY_PACKET_SIZE; ++i)	// omp loves int...	//			10ms
		{
			uint32_t u, v;												//			10ms
			uint32_t index = (uint32_t)p * RAY_PACKET_SIZE + i;			//			10ms
			morton_decode(index, u, v);									//			11ms
			int x = (int)u, y = (int)v;									//			11ms
			screen->Plot(x, y, (rayPacket.colors[i].out()));			//			11ms
		}
	}
	sprintf(info, "ray packet mode");
	screen->Print(info, (SCRWIDTH / 2) - 50, 10, 0xffff00);
	sprintf(info, " %d Rays/Packet", RAY_PACKET_SIZE);
	screen->Print(info, 20, 40, 0x00ff00);
#endif

#else						// <<<------- Switch between ray tracing and path tracing -----<<-
	/// Path Tracing
	if (ipindex == 0)	buffer->one_more_pass();
	int spp = SAMPLEPERPIXEL;
	int pass = buffer->get_count() % spp;

	// Before there is Image Processing, we render...
	if (ipindex == 0)
	{
#pragma omp parallel for
		for (int y = 0; y < scene->get_cam()->get_h(); y++)
		{
#pragma omp parallel for
			for (int x = 0; x < scene->get_cam()->get_w(); x++)
			{
				Ray ray = scene->get_cam()->ray(x, y, spp, pass);
				Sampler sampler = Sampler(scene);
				RGBColor color = sampler.sample(ray, 0);
				color.correct_gamma();
				buffer->add_to_buffer(color, x, y);
				screen->Plot(x, y, (buffer->get_color(x, y).out()));
			}
		}
	}

	// Image Processing
#if 1
	if (ipindex == 1)
	{
		ipbuffer->reset_buffer();
		imageprocessing->median_filter(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 2)
	{
		ipbuffer->reset_buffer();
		imageprocessing->brush_paint(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 3)
	{
		ipbuffer->reset_buffer();
		imageprocessing->color_disperse(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 4)
	{
		ipbuffer->reset_buffer();
		imageprocessing->barrel_distortion(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 5)
	{
		ipbuffer->reset_buffer();
		imageprocessing->hight_light(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 6)
	{
		ipbuffer->reset_buffer();
		imageprocessing->lomo(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 7)
	{
		ipbuffer->reset_buffer();
		imageprocessing->original_image(scene->get_cam()->get_w(), scene->get_cam()->get_h(), buffer, ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == 8)
	{
		imageprocessing->save_image(scene->get_cam()->get_w(), scene->get_cam()->get_h(), ipbuffer);
		ipindex = -1;
	}
	else if (ipindex == -1)
	{
#pragma omp parallel for
		for (int y = 0; y < scene->get_cam()->get_h(); y++)
#pragma omp parallel for
		for (int x = 0; x < scene->get_cam()->get_w(); x++)
			screen->Plot(x, y, (ipbuffer->get_color(x, y).out()));
	}
#endif
#ifdef VIS_CACHE
	sprintf(info, "Path Tracing with VisCache");
	screen->Print(info, (SCRWIDTH / 2) - 80, 10, 0xffff00);
	sprintf(info, " %d SPP", buffer->get_count());
	screen->Print(info, 20, 40, 0x00ff00);
#else
	sprintf(info, "Path Tracing");
	screen->Print(info, (SCRWIDTH / 2) - 50, 10, 0xffff00);
	sprintf(info, " %d SPP", buffer->get_count());
	screen->Print(info, 20, 40, 0x00ff00);
#endif
	
#endif

	/// Time counter and screen display
	clock_t finishTime = clock();
	float duration =  float(finishTime - startTime);
	float fps = 1000.0f / duration;
	
	sprintf(info, " %.4f ms", duration);
	screen->Print(info, 20, 20, 0x00ff00);
	sprintf(info, " %.2f FPS", fps);
	screen->Print(info, 20, 30, 0x00ff00);
	/*
	sprintf(info, " %d object(s)", scene->object_count());
	screen->Print(info, 20, SCRHEIGHT - 40, 0xff0000);
	sprintf(info, " %d BVH node(s)", scene->BVH_node_count());
	screen->Print(info, 20, SCRHEIGHT - 30, 0xff0000);
	sprintf(info, " %d light(s)", scene->light_count());
	screen->Print(info, 20, SCRHEIGHT - 20, 0xff0000);
	//getchar();	// Save the laptop from burning...
	*/
}

// -----------------------------------------------------------
// Build the scene
// -----------------------------------------------------------
void Game::BuildScene()
{
	/// Build the scene, here, a simple sphere and a defult camera
	scene = new Scene();
	Camera* cam = new Camera();
	cam->set_focal(28.0f);
	cam->set_aperture(0.5f);
	buffer = new colorBuffer();
	scene->set_cam(cam);
	/// Load image using FreeImage, and set it to the Scene::EnvBall as a SkySphere
	EnvBall* sky = new EnvBall("./img/sky.png");
	scene->set_env_ball(sky);

	/// Initialize Textures
	ConstTex* solid_blk = new ConstTex(RGBColor(0.0f));
	ConstTex* solid_red = new ConstTex(RGBColor(1.0f));
	ConstTex* solid_gnd = new ConstTex(RGBColor(0.1f, 0.4f, 0.1f));
	ConstTex* solid_gry = new ConstTex(RGBColor(0.5f));
	ConstTex* solid_grn = new ConstTex(RGBColor(1.0f, 0.85f, 0.0f));
	ConstTex* solid_wit = new ConstTex(RGBColor(0.8f));
	CheckerTex* checkerBoard = new CheckerTex(0.4f, RGBColor(1.0f), RGBColor(0.15f, 0.3f, 0.15f));
	CheckerTex* smallChecker = new CheckerTex(1.5f, RGBColor(0.1f, 0.4f, 0.1f), RGBColor(0.8f));
	MarbleTex*  marble		 = new MarbleTex(0.05f, RGBColor(0.75f));
	ImageTex*   earth_map	 = new ImageTex("./img/earth.jpg");
	// Add all textures to scene.textures
	scene->add_texture(solid_blk);			scene->add_texture(solid_wit);
	scene->add_texture(solid_red);			scene->add_texture(checkerBoard);
	scene->add_texture(solid_gnd);			scene->add_texture(smallChecker);
	scene->add_texture(solid_gry);			scene->add_texture(marble);
	scene->add_texture(solid_grn);			scene->add_texture(earth_map);

	/// Initialize materials: lambert, blinn, etc. All materials need to have a texture!
	/// Material test
	Lambert* lambert_test = new Lambert(RGBColor(0.5f));		lambert_test->set_texture(solid_red);		scene->add_material(lambert_test);
	Lambert* lambert_test_r = new Lambert(RGBColor(0.5f));		lambert_test_r->set_texture(solid_red);		scene->add_material(lambert_test_r);
	Lambert* lambert_test_grd = new Lambert(RGBColor(0.5f));	lambert_test_grd->set_texture(checkerBoard);	scene->add_material(lambert_test_grd);
	Lambert* light_W_test = new Lambert(RGBColor(1.0f), RGBColor(10.0f)); light_W_test->set_texture(solid_red); scene->add_material(light_W_test);
	Lambert* light_R_test = new Lambert(RGBColor(0.0f), RGBColor(20.0f, 0.0f, 0.0f)); light_R_test->set_texture(solid_blk); scene->add_material(light_R_test);
	Matte* matte_test = new Matte(RGBColor(1.0f, 0.0f, 0.0f));	matte_test->set_texture(solid_grn);			scene->add_material(matte_test);
	Mirror* mirror_test = new Mirror(RGBColor(0.8f));			mirror_test->set_texture(solid_wit);		scene->add_material(mirror_test);
	Dielectric* glass_test = new Dielectric(RGBColor(1.0f));	glass_test->set_texture(new ConstTex(RGBColor(1.0f)));	scene->add_material(glass_test);
	Metal* metal_test_01 = new Metal(RGBColor(0.3f), 0.5f);		metal_test_01->set_texture(solid_wit);		scene->add_material(metal_test_01);
	Metal* metal_test_02 = new Metal(RGBColor(0.3f), 0.5f);		metal_test_02->set_texture(smallChecker);	scene->add_material(metal_test_02);
	Metal* metal_test_03 = new Metal(RGBColor(0.3f), 0.4f);		metal_test_03->set_texture(new ConstTex(RGBColor(0.8f, 0.4f, 0.0f)));		scene->add_material(metal_test_03);
	Shiny* specular_test = new Shiny(RGBColor(1.0f), RGBColor(0.0f), 0.25f);	specular_test->set_texture(marble);		scene->add_material(specular_test);
	Phong* phong_test = new Phong(RGBColor(0.75f), 0.4f, 0.05f, 300.0f);			phong_test->set_texture(earth_map);		scene->add_material(phong_test);
	Phong* phong_test_e = new Phong(RGBColor(0.75f), 0.65f, 0.34f, 2.0f);		phong_test_e->set_texture(earth_map);	scene->add_material(phong_test_e);
	Microfacet * microfacet_test01 = new Microfacet(RGBColor(0.5f), 1.0f, 0.9f);		microfacet_test01->set_texture(solid_wit);	scene->add_material(microfacet_test01);
	Microfacet * microfacet_test02 = new Microfacet(RGBColor(0.5f), 1000.0f, 0.009f);		microfacet_test02->set_texture(solid_grn);	scene->add_material(microfacet_test02);
	Microfacet * microfacet_test03 = new Microfacet(RGBColor(0.5f), 1000.0f, 0.009f);		microfacet_test03->set_texture(solid_wit);	scene->add_material(microfacet_test03);

	/// Create objects, assign materials, push primitives into scene
	/// Three balls, one specular, one mirror, one glass
	Sphere* ball01 = new Sphere(Point3D( 0.0f,   0.0f,  0.0f), 10.0f, RGBColor(0.5f));
	ball01->set_material(mirror_test);	// lambert_test	microfacet_test
	Sphere* ball02 = new Sphere(Point3D( 0.0f, -17.0f, 20.0f),  3.0f, RGBColor(0.5f));
	ball02->set_material(phong_test);
	Sphere* ball03 = new Sphere(Point3D(-30.0f,  8.0f, 0.0f),	6.0f, RGBColor(0.5f));
	ball03->set_material(phong_test);
	
	/// Two new balls, one as a lambert basement, the other as a shiny thingy
	Sphere* ball04 = new Sphere(Point3D(-20.0f, -25.0f, 10.0f),  7.0f, RGBColor(0.5f));
	ball04->set_material(lambert_test_grd);
	Sphere* ball05 = new Sphere(Point3D(-20.0f, -13.0f, 10.0f),  5.0f, RGBColor(0.5f));
	ball05->set_material(glass_test);
	
	/// The ground and the back wall, lambert is too bright for the backwall, thus use a new 'wall' material*
	Rect* ground = new Rect(Point3D(-50.0f, -20.01f, -30.0f), Point3D(-50.0f, -20.0f, 30.0f), Point3D(50.0f, -20.01f, -30.0f));
	ground->set_material(lambert_test_grd);


	
	// The light(s)!
#ifdef BIG_LIGHT
	Rect* areaLight01 = new Rect(Point3D(30.0f, 40.0f, 15.0f), Point3D( 0.0f, 40.1f, 15.0f), Point3D(30.0f, 39.9f, 0.0f));
#else
	Rect* areaLight01 = new Rect(Point3D(50.0f, 40.0f, 15.0f), Point3D(30.0f, 40.1f, 15.0f), Point3D(50.0f, 39.9f, 5.0f));
#endif
	areaLight01->set_material(light_W_test);	// 
	scene->add_geo(areaLight01);
	//Rect* areaLight02 = new Rect(Point3D(-30.0f, 40.0f, 15.0f), Point3D(-50.0f, 39.9f, 15.0f), Point3D(-30.0f, 40.1f, 5.0f));
	//areaLight02->set_material(light_R_test);	// light_W_test
	//scene->add_geo(areaLight02);
	//Rect* areaLight03 = new Rect(Point3D(50.0f, 5.0f, -20.0f), Point3D(40.0f, 5.1f, -20.0f), Point3D(50.0f, 4.9f, -30.0f));
	//areaLight03->set_material(light_W_test);	// 
	//scene->add_geo(areaLight03);

#if 1
	//// Load *.obj files
	scene->load_obj("./Obj/bunny.obj", microfacet_test03, Point3D(21.5f, -20.0f, 10.0f));
	scene->load_obj("./Obj/helix_high.obj", microfacet_test03, Point3D(0.0f, -20.0f, 3.0f));
	scene->load_obj("./Obj/bracelet.obj", microfacet_test02, Point3D(-1.5f, -20.0f, 15.5f));
#endif
#ifdef INDOOR
	// Add left, right, back, front and top of the indoor box
	scene->load_obj("./Obj/indoor.obj", lambert_test, Point3D(0.0f, 0.0f, 0.0f));
#endif

#if 1
	//scene->add_geo(ball01);
	//scene->add_geo(ball02);
	//scene->add_geo(ball03);
	scene->add_geo(ball04);
	scene->add_geo(ball05);
#endif

#if 1
	scene->add_geo(ground);
	//scene->add_geo(sideWall);
	
#endif

	// Initialize Image Processing Parts
	ipindex = 0;
	imageprocessing = new ImageProcessing();
	ipbuffer = new colorBuffer();

	// Initialize Scene's BVH acceleration structure
	scene->buildBVH();

#ifdef VIS_CACHE
	clock_t cache_begin = clock();
	scene->buildVisCache();
	clock_t cache_end = clock();
	float duration = (float)(cache_end - cache_begin);
	cout << "Visibility Cache:	" << scene->get_cache_count() << " caches\n"
		<< "Using:			" << float(scene->get_cache_count() * sizeof(VisCacheData)) / (1024.f*1024.f)
		 << " MB space\n"
		 << "Taking:			" << duration << " ms.\n" << std::endl;
#endif

	/// Print scene info
	cout << "Total: " << scene->object_count() << " object(s), "
		 << scene->texture_count() << " texture(s), "
		 << scene->material_count() << " material(s), "
		 << scene->light_mat_count() << " light emmiting material(s), "
		 << scene->light_count() << " light(s).\n" << endl << endl;
	
	/// Print helper info
	cout << "Key Instructions:" << endl;
	cout << "	Esc: quit" << endl;
	cout << "	A: rotate left" << endl;
	cout << "	D: rotate right" << endl;
	cout << "	W: rotate up" << endl;
	cout << "	S: rotate down" << endl;
	cout << "	Arrow-up: move front" << endl;
	cout << "	Arrow-down: move back" << endl;
	cout << "	Arrow-left: move left" << endl;
	cout << "	Arrow-right: move right" << endl;
	cout << "	K: Focus further" << endl;
	cout << "	L: Focus closer (>0.0f)" << endl;
	cout << "	N: Aperture up (<5.0f)" << endl;
	cout << "	M: Aperture down (>0.0f)" << endl;
	cout << "	#1: Median Filter" << endl;
	cout << "	#2: Painting Style" << endl;
	cout << "	#3: Color Disperse" << endl;
	cout << "	#4: Barrel Distortion" << endl;
	cout << "	#5: Glow" << endl;
	cout << "	#6: Lomo" << endl;
	cout << "	#9: Original Image" << endl;
	cout << "	#0: Save Image" << endl;
	cout << "	Any other Key: restart render" << endl;
}

// The Morton Encoder, given pixel coordinate (x, y), return index in array
uint32_t Game::morton_encode(uint16_t xPos, uint16_t yPos)
{
	static const uint32_t MASKS[] = { 0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF };
	static const uint32_t SHIFTS[] = { 1, 2, 4, 8 };

	uint32_t x = xPos;
	uint32_t y = yPos;

	x = (x | (x << SHIFTS[3])) & MASKS[3];
	x = (x | (x << SHIFTS[2])) & MASKS[2];
	x = (x | (x << SHIFTS[1])) & MASKS[1];
	x = (x | (x << SHIFTS[0])) & MASKS[0];

	y = (y | (y << SHIFTS[3])) & MASKS[3];
	y = (y | (y << SHIFTS[2])) & MASKS[2];
	y = (y | (y << SHIFTS[1])) & MASKS[1];
	y = (y | (y << SHIFTS[0])) & MASKS[0];

	const uint32_t result = x | (y << 1);
	return result;
}

// The Morton Decoder, given array index, return pixel coordinate (x, y)
void Game::morton_decode(const uint32_t& index, uint32_t& x_out, uint32_t& y_out)
{
	uint32_t x = index;
	uint32_t y = index;
	y = y >> 1;

	x &= 0x55555555;
	x = (x ^ (x >> 1)) & 0x33333333;
	x = (x ^ (x >> 2)) & 0x0f0f0f0f;
	x = (x ^ (x >> 4)) & 0x00ff00ff;
	x = (x ^ (x >> 8)) & 0x0000ffff;
	y &= 0x55555555;
	y = (y ^ (y >> 1)) & 0x33333333;
	y = (y ^ (y >> 2)) & 0x0f0f0f0f;
	y = (y ^ (y >> 4)) & 0x00ff00ff;
	y = (y ^ (y >> 8)) & 0x0000ffff;

	x_out = x;
	y_out = y;
}