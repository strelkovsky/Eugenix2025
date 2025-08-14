// Tests headers
#include "Tests/0-TriangleApp.h"
#include "Tests/1-SquareApp.h"
#include "Tests/2-SimpleCameraApp.h"
#include "Tests/3-TextureApp.h"
#include "Tests/4.1-TinyObjLoader.h"
#include "Tests/4.2-AssimpLoader.h"

#include "Tests/LearnOpenGLApp.h"

int main()
{
	//Eugenix::TriangleApp app;
	//Eugenix::SquareApp app;
	//Eugenix::SimpleCameraApp app;
	Eugenix::TextureApp app;
	//Eugenix::TinyObjLoaderApp app;
	//Eugenix::AssimpLoaderApp app;
	
	//Eugenix::LightingApp app;
	//Eugenix::LearnOpenGLApp app;
	return app.Run();
}