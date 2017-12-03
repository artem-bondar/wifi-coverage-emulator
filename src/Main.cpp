#include "Tracer.h"
#include "stdio.h"

int main(int argc, char** argv)
{
  CTracer tracer;
  CScene scene;
  char pathToModel[128];
  glm::vec3 router;
  FILE * model;

  int xRes, yRes, openMP, rays;
  float power;


  if(argc == 2) // There is input file in parameters
  {
    FILE* file = fopen(argv[1], "r");
    if(file)
    {
	  if (fscanf(file, "Model: %s\n", pathToModel) == 1)
	  {
		  model = fopen(pathToModel, "r");
		  if (model == NULL) {
			  printf("Impossible to open the model file!\r\n");
			  return 4;
		  }
	  }
	  else
	  {
		  printf("Invalid config format! No model file.\r\n");
		  return 3;
	  }

	  if (fscanf(file, "Router position: %f %f %f\n", &router.x, &router.y, &router.z) != 3)
	  {
		  printf("Invalid router position! Using default parameters.\r\n");
		  glm::vec3 router = glm::vec3(14000.0f, 0.0f, 1750.0f);
	  }
	  if (fscanf(file, "Router power: %f\n", &power) != 1)
	  {
		  printf("Invalid power! Using default parameters.\r\n");
		  power = 100.0f;
	  }
	  if (fscanf(file, "Maximum rays: %d\n", &rays) != 1)
	  {
		  printf("Invalid maximum rays! Using default parameters.\r\n");
		  rays = 30000;
	  }
	  if (fscanf(file, "Camera position: %f %f %f\n", &tracer.m_camera.m_pos.x, &tracer.m_camera.m_pos.y, &tracer.m_camera.m_pos.z) != 3)
	  {
		  printf("Invalid camera position! Using default parameters.\r\n");
		  tracer.m_camera.m_pos = glm::vec3(12500.0f, 1000.0f, 10000.0f);
	  }
	  if (fscanf(file, "Camera view direction: %f %f %f\n", &tracer.m_camera.m_forward.x, &tracer.m_camera.m_forward.y, &tracer.m_camera.m_forward.z) != 3)
	  {
		  printf("Invalid camera view direction! Using default parameters.\r\n");
		  tracer.m_camera.m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
	  }
	  if (fscanf(file, "Camera up direction: %f %f %f\n", &tracer.m_camera.m_up.x, &tracer.m_camera.m_up.y, &tracer.m_camera.m_up.z) != 3)
	  {
		  printf("Invalid camera view direction! Using default parameters.\r\n");
		  tracer.m_camera.m_up = glm::vec3(0.0, 1.0, 0.0);
	  }
	  if (fscanf(file, "Camera right direction: %f %f %f\n", &tracer.m_camera.m_right.x, &tracer.m_camera.m_right.y, &tracer.m_camera.m_right.z) != 3)
	  {
		  printf("Invalid camera view direction! Using default parameters.\r\n");
		  tracer.m_camera.m_right = glm::vec3(1.0, 0.0, 0.0);
	  }
	  if (fscanf(file, "Camera FOV angles: %f %f\n", &tracer.m_camera.m_FOV.x, &tracer.m_camera.m_FOV.y) != 2)
	  {
		  printf("Invalid camera FOV! Using default parameters.\r\n");
		  tracer.m_camera.m_FOV = glm::vec2(90, 80);
	  }
      if(fscanf(file, "Resolution: %d %d\n", &xRes, &yRes) != 2)
      {
		  printf("Invalid resolution format! Using default parameters.\r\n");
		  int xRes = 1024;
		  int yRes = 768;
      }   
	  if (fscanf(file, "OpenMP: %d", &openMP) != 1)
	  {
		  printf("Invalid OpenMP format! Using default parameters.\r\n");
		  openMP = false;
	  }
      fclose(file);
    }
	else
	{
		printf("Invalid config path!\r\n");
		return 2;
	}
  }
  else
  {
	  printf("No config!\r\n");
	  return 1;
  }

  printf("Model: %s\nRouter position: (%.2f, %.2f, %.2f)\nRouter power: %.2f\nMaximum rays: %d\nCamera position: (%.2f, %.2f, %.2f)\n"
	     "Camera view direction: (%.2f, %.2f, %.2f)\nCamera up direction: (%.2f, %.2f, %.2f)\nCamera right direction: (%.2f, %.2f, %.2f)\n"
	     "Camera FOV angles: (%.2f, %.2f)\nResolution: %dx%d\nOpenMP: %s\n\n",
	  pathToModel, router.x, router.y, router.z, power, rays, tracer.m_camera.m_pos.x, tracer.m_camera.m_pos.y, tracer.m_camera.m_pos.z,
	  tracer.m_camera.m_forward.x, tracer.m_camera.m_forward.y, tracer.m_camera.m_forward.z, tracer.m_camera.m_up.x, tracer.m_camera.m_up.y, tracer.m_camera.m_up.z,
	  tracer.m_camera.m_right.x, tracer.m_camera.m_right.y, tracer.m_camera.m_right.z, tracer.m_camera.m_FOV.x, tracer.m_camera.m_FOV.y,
	  xRes, yRes, openMP ? "true" : "false");

  if (openMP)
  {
#ifndef _OPENMP
	  printf("Required OpenMP to run, check runtime parametres.\n");
	  return 5;
#endif
  }

  if (!scene.load(model))
	  return 6;
  scene.preprocess();
  scene.processSignalMap(router, power, rays);

  tracer.m_scene = &scene;
  tracer.RenderImage(xRes, yRes, openMP);
  tracer.SaveImageToFile("Result.png");

  return 0;
}