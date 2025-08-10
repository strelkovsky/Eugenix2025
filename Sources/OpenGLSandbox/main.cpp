// Tests headers
#include "Tests/SquareApp.h"

#include "Tests/SimpleCameraApp.h"
#include "Tests/TextureApp.h"
#include "Tests/LightingApp.h"

#include "Tests/LearnOpenGLApp.h"

int main()
{
	//Eugenix::SquareApp app;
	//Eugenix::SimpleCameraApp app;
	//Eugenix::TextureApp app;
	//Eugenix::LightingApp app;
	Eugenix::LearnOpenGLApp app;
	return app.Run();
}