#version 410

layout(location = 0) in vec2 texture_coord;

uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;

// 0 - original
// 1 - grayscale
// 2 - blur
uniform int outputMode = 2;

// Flip texture horizontally when
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y);

layout(location = 0) out vec4 out_color;

vec4 grayscale()
{
	vec4 color = texture(textureImage, textureCoord);
	float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b; 
	return vec4(gray, gray, gray,  0);
}

vec4 blur(int blurRadius)
{
	vec2 texelSize = 1.0f / screenSize;
	vec4 sum = vec4(0);
	for(int i = -blurRadius; i <= blurRadius; i++)
	{
		for(int j = -blurRadius; j <= blurRadius; j++)
		{
			sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
		}
	}
		
	float samples = pow((2 * blurRadius + 1), 2);
	return sum / samples;
}

float sobel(int mycolor)
{
	mat3 mGx = mat3(
		-1,0,1,-2,0,2,-1,0,1
	);

	mat3 mGy = mat3(
		-1, -2, -1, 0, 0, 0, 1, 2, 1
	);

	vec2 texelSize = 1.0f / screenSize;

	float Gx = 0, Gy = 0;

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			if (mycolor == 0)
			{
				Gx += texture(textureImage, textureCoord + vec2(i, j) * texelSize).r * mGx[i + 1][j + 1];
				Gy += texture(textureImage, textureCoord + vec2(i, j) * texelSize).r * mGx[i + 1][j + 1];
			}

			if (mycolor == 1)
			{
				Gx += texture(textureImage, textureCoord + vec2(i, j) * texelSize).g * mGx[i + 1][j + 1];
				Gy += texture(textureImage, textureCoord + vec2(i, j) * texelSize).g * mGx[i + 1][j + 1];
			}

			if (mycolor == 2)
			{
				Gx += texture(textureImage, textureCoord + vec2(i, j) * texelSize).b * mGx[i + 1][j + 1];
				Gy += texture(textureImage, textureCoord + vec2(i, j) * texelSize).b * mGx[i + 1][j + 1];
			}
		}
	}

	return sqrt(Gx * Gx + Gy * Gy);
}

void main()
{
	switch (outputMode)
	{
		case 1:
		{
			out_color = grayscale();
			break;
		}

		case 2:
		{
			out_color = blur(3);
			break;
		}

		case 3:
		{
			float r = sobel(0);
			float g = sobel(1);
			float b = sobel(2);

			out_color = vec4(r, g, b, 1);
			break;
		}

		default:
			out_color = texture(textureImage, textureCoord);
			break;
	}
}