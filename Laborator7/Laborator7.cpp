#include "Laborator7.h"

#include <vector>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

// Order of function calling can be seen in "Source/Core/World.cpp::LoopUpdate()"
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/World.cpp

Laborator7::Laborator7()
{
	outputMode = 0;
	gpuProcessing = false;
	saveScreenToImage = false;
	window->SetSize(600, 600);
}

Laborator7::~Laborator7()
{
}

FrameBuffer *processed;

void Laborator7::Init()
{
	// Load default texture fore imagine processing 
	originalImage = TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Cube/posx.png", nullptr, "image", true, true);
	processedImage = TextureManager::LoadTexture(RESOURCE_PATH::TEXTURES + "Cube/posx.png", nullptr, "newImage", true, true);

	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "quad.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	static std::string shaderPath = "Source/Laboratoare/Laborator7/Shaders/";

	// Create a shader program for particle system
	{
		Shader *shader = new Shader("ImageProcessing");
		shader->AddShader((shaderPath + "VertexShader.glsl").c_str(), GL_VERTEX_SHADER);
		shader->AddShader((shaderPath + "FragmentShader.glsl").c_str(), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

void Laborator7::FrameStart()
{
}

void Laborator7::Update(float deltaTimeSeconds)
{
	ClearScreen();

	auto shader = shaders["ImageProcessing"];
	shader->Use();

	if (saveScreenToImage)
	{
		window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
	}

	int flip_loc = shader->GetUniformLocation("flipVertical");
	glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

	int screenSize_loc = shader->GetUniformLocation("screenSize");
	glm::ivec2 resolution = window->GetResolution();
	glUniform2i(screenSize_loc, resolution.x, resolution.y);

	int outputMode_loc = shader->GetUniformLocation("outputMode");
	glUniform1i(outputMode_loc, outputMode);

	int gpuProcessing_loc = shader->GetUniformLocation("outputMode");
	glUniform1i(outputMode_loc, outputMode);

	int locTexture = shader->GetUniformLocation("textureImage");
	glUniform1i(locTexture, 0);
	auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
	textureImage->BindToTextureUnit(GL_TEXTURE0);

	RenderMesh(meshes["quad"], shader, glm::mat4(1));

	if (saveScreenToImage)
	{
		saveScreenToImage = false;
		GLenum format = GL_RGB;
		if (originalImage->GetNrChannels() == 4)
		{
			format = GL_RGBA;
		}
		glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
		processedImage->UploadNewData(processedImage->GetImageData());
		SaveImage("shader_processing_" + std::to_string(outputMode));

		float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
		window->SetSize(static_cast<int>(600 * aspectRatio), 600);
	}
}

void Laborator7::FrameEnd()
{
	DrawCoordinatSystem();
}

void Laborator7::OnFileSelected(std::string fileName)
{
	if (fileName.size())
	{
		std::cout << fileName << endl;
		originalImage = TextureManager::LoadTexture(fileName.c_str(), nullptr, "image", true, true);
		processedImage = TextureManager::LoadTexture(fileName.c_str(), nullptr, "newImage", true, true);

		float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
		window->SetSize(static_cast<int>(600 * aspectRatio), 600);
	}
}

void Laborator7::GrayScale()
{
	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();

	if (channels < 3)
		return;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

	for (int i = 0; i < imageSize.y; i++)
	{
		for (int j = 0; j < imageSize.x; j++)
		{
			int offset = channels * (i * imageSize.x + j);

			// Reset save image data
			char value = static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f + data[offset + 2] * 0.07);
			memset(&newData[offset], value, 3);
		}
	}

	processedImage->UploadNewData(newData);
}
double deg2rad(double degrees) {
	return degrees * 4.0 * atan(1.0) / 180.0;
}
void Laborator7::HoughFilter()
{
	GrayScale();

	unsigned int channels = originalImage->GetNrChannels();
	unsigned char* data = originalImage->GetImageData();
	unsigned char* newData = processedImage->GetImageData();
	int threshold = 175;

	glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

	unsigned int* _accu = (unsigned int*)calloc(imageSize.x * imageSize.y, sizeof(unsigned int));

	double hough_h = ((sqrt(2.0) * (double)(imageSize.y > imageSize.x ? imageSize.y : imageSize.x)) / 2.0);
	double _accu_h = hough_h * 2.0; // -r -> +r  
	double _accu_w = 180;
	double center_x = imageSize.x / 2;
	double center_y = imageSize.y / 2;



	for (int i = 0; i < imageSize.y; i++)
	{
		for (int j = 0; j < imageSize.x; j++)
		{
			int offset = channels * (i * imageSize.x + j);

			// Reset save image data
			char value = static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f + data[offset + 2] * 0.07);
			memset(&newData[offset], value, 3);
			if (data[offset] > 250)
			{
				for (int t = 0; t < 180; t++)
				{
					double r = (((double)i - center_x) * cos(deg2rad(t))) + (((double)j - center_y) * sin(deg2rad(t)));
					_accu[(int)((round(r + hough_h) * 180.0)) + t]++;
				}
			}
		}
	}

	std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > lines;

	int _img_w = imageSize.x;
	int _img_h = imageSize.y;

	for (int r = 0; r < _accu_h; r++)
	{
		for (int t = 0; t < _accu_w; t++)
		{
			if ((int)_accu[(int) (r*_accu_w) + t] >= threshold)
			{
				//Is this point a local maxima (9x9)  
				int max = _accu[(int) (r*_accu_w) + t];
				for (int ly = -4; ly <= 4; ly++)
				{
					for (int lx = -4; lx <= 4; lx++)
					{
						if ((ly + r >= 0 && ly + r < _accu_h) && (lx + t >= 0 && lx + t < _accu_w))
						{
							if ((int)_accu[(int) ((r + ly)*_accu_w) + (t + lx)] > max)
							{
								max = _accu[(int) ((r + ly)*_accu_w) + (t + lx)];
								ly = lx = 5;
							}
						}
					}
				}
				if (max > (int)_accu[(int) (r*_accu_w) + t])
					continue;


				int x1, y1, x2, y2;
				x1 = y1 = x2 = y2 = 0;

				if (t >= 45 && t <= 135)
				{
					//y = (r - x cos(t)) / sin(t)  
					x1 = 0;
					y1 = ((double)(r - (_accu_h / 2)) - ((x1 - (_img_w / 2)) * cos(deg2rad(t)))) / sin(deg2rad(t)) + (_img_h / 2);
					x2 = _img_w - 0;
					y2 = ((double)(r - (_accu_h / 2)) - ((x2 - (_img_w / 2)) * cos(deg2rad(t)))) / sin(deg2rad(t)) + (_img_h / 2);
				}
				else
				{
					//x = (r - y sin(t)) / cos(t);  
					y1 = 0;
					x1 = ((double)(r - (_accu_h / 2)) - ((y1 - (_img_h / 2)) * sin(deg2rad(t)))) / cos(deg2rad(t)) + (_img_w / 2);
					y2 = _img_h - 0;
					x2 = ((double)(r - (_accu_h / 2)) - ((y2 - (_img_h / 2)) * sin(deg2rad(t)))) / cos(deg2rad(t)) + (_img_w / 2);
				}

				lines.push_back(std::pair< std::pair<int, int>, std::pair<int, int> >(std::pair<int, int>(x1, y1), std::pair<int, int>(x2, y2)));

			}
		}
	}

	std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > >::iterator it;
	for (it = lines.begin(); it != lines.end(); it++) {
		std::cout << it->first.first << ", " << it->first.second << "\n";
	}
	//Draw the results  
	//std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > >::iterator it;
	//for (it = lines.begin(); it != lines.end(); it++)
	//{
	//	Line(img_res, cv::Point(it->first.first, it->first.second), cv::Point(it->second.first, it->second.second), cv::Scalar(0, 0, 255), 2, 8);
	//}

	//Visualize all  
	int aw, ah, maxa;
	aw = ah = maxa = 0;
	for (int p = 0; p < (ah*aw); p++)
	{
		if (_accu[p] > maxa)
			maxa = _accu[p];
	}
	double contrast = 1.0;
	double coef = 255.0 / (double)maxa * contrast;

	for (int p = 0; p < (ah*aw); p++)
	{
		unsigned char c = (double)_accu[p] * coef < 255.0 ? (double)_accu[p] * coef : 255.0;
		newData[(p * 3) + 0] = 255;
		newData[(p * 3) + 1] = 255 - c;
		newData[(p * 3) + 2] = 255 - c;
	}


	processedImage->UploadNewData(newData);
}


void Laborator7::SaveImage(std::string fileName)
{
	cout << "Saving image! ";
	processedImage->SaveToFile((fileName + ".png").c_str());
	cout << "[Done]" << endl;
}

// Read the documentation of the following functions in: "Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/SPG-Framework/blob/master/Source/Core/Window/InputController.h

void Laborator7::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void Laborator7::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
	{
		OpenDialogue();
	}

	if (key == GLFW_KEY_E)
	{
		gpuProcessing = !gpuProcessing;
		if (gpuProcessing == false)
		{
			outputMode = 0;
		}
		cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
	}

	if (key - GLFW_KEY_0 >= 0 && key < GLFW_KEY_4)
	{
		outputMode = key - GLFW_KEY_0;

		if (gpuProcessing == false)
		{
			outputMode = 0;
			HoughFilter();
		}
	}

	if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
	{
		if (!gpuProcessing)
		{
			SaveImage("processCPU_" + std::to_string(outputMode));
		}
		else
		{
			saveScreenToImage = true;
		}
	}
};

void Laborator7::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void Laborator7::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void Laborator7::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void Laborator7::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator7::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void Laborator7::OnWindowResize(int width, int height)
{
	// treat window resize event
}
