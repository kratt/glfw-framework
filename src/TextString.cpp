#include "TextString.h"

#include <iostream>
#include <algorithm>
#include <glm/gtx/norm.hpp>

TextString::TextString(std::string text, glm::vec2 screenPos)
	: m_text(text),
	m_pos(screenPos)
{
	initTexture();
}

TextString::~TextString()
{
}

GLuint TextString::texId() const
{
	return m_texId;
}

glm::vec2 TextString::dims() const
{
	return m_dims;
}

glm::vec2 TextString::pos() const
{
	return m_pos;
}

void TextString::initTexture()
{
	FT_Library ft;
	FT_Face face;


	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init freetype library\n");
	}

	/* Load a font */
	if (FT_New_Face(ft, "C:/Windows/Fonts/Calibri.TTF", 0, &face)) {
		fprintf(stderr, "Could not open font %s\n", "Calibri");
	}

	FT_Set_Pixel_Sizes(face, 0, 224);

	const char *p;
	FT_GlyphSlot g = face->glyph;


	// determine min and max dims
	

	//for (p = m_text.c_str(); *p; p++)
	//{
	//	if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
	//		continue;

	//	//totalWidth += g->bitmap.width;
	//	totalWidth += g->advance.x >> 6;
	//	maxHeight = std::max(maxHeight, g->bitmap.rows);
	//}

	int totalWidth = 0.0f;
	int maxHeight = 0.0f;

	int pen_x = 0;
	int pen_y = 0;

	bool firstRun = true;
	float firstDist = 0.0f;

	for (p = m_text.c_str(); *p; p++)
	{
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO))
			continue;

		float w = g->bitmap.width;
		float h = g->bitmap.rows;

		float pos_x = pen_x + g->bitmap_left;
		float pos_y = pen_y + g->bitmap_top - h;

		// update position based on beginning of first character
		if (firstRun) {
			m_pos.x = m_pos.x + g->bitmap_left;
			m_pos.y = m_pos.y + g->bitmap_top - h;

			firstDist = g->bitmap_left;
			firstRun = false;
		}
	
		totalWidth = pen_x + w;

		pen_x += g->advance.x >> 6;
		pen_y += g->advance.y >> 6;
	
		maxHeight = std::max(maxHeight, int(h));
	}


	m_dims = glm::vec2(totalWidth, maxHeight);

	std::cout << "Text dims: " << m_dims.x << " " << m_dims.y << std::endl;

	/* Create an empty texture that will be used to hold the entire text */
	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, totalWidth, maxHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);


	pen_x = 0;
	pen_y = 0;

	/* Loop through all characters */
	for (p = m_text.c_str(); *p; p++)
	{
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO))
			continue;


		float w = g->bitmap.width;
		float h = g->bitmap.rows;

		float pos_x = pen_x + g->bitmap_left;
		float pos_y = maxHeight-pen_y;// +g->bitmap_top;


		unsigned char* sdfData;
		distanceField(&g->bitmap, &sdfData);

		glTexSubImage2D(GL_TEXTURE_2D, 0, pen_x, maxHeight-h, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, sdfData);

		pen_x += g->advance.x >> 6;
		pen_y += g->advance.y >> 6;
	}


	///* Loop through all characters */
	//for (p = text; *p; p++)
	//{
	//	float w = g->bitmap.width;
	//	float h = g->bitmap.rows;

	//	float pos_x = pen_x + g->bitmap_left;
	//	float pos_y = pen_y + g->bitmap_top - h;

	//	glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(pos_x, pos_y, 0.0f));
	//	glm::mat4 model = glm::scale(trans, glm::vec3(float(w), float(h), 1.0f));

	//	pen_x += g->advance.x >> 6;
	//	pen_y += g->advance.y >> 6;
	//}



	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextString::distanceField(FT_Bitmap* bitmap, unsigned char** outData)
{
	// first unpack monochromatic bitmap
	unsigned char* bitMapData;
	unpackMonoBitmap(bitmap, &bitMapData);

	int width = bitmap->width;
	int height = bitmap->rows;

	float* sdf = new float[width * height];

	float minDist = std::numeric_limits<float>::max();
	float maxDist = std::numeric_limits<float>::lowest();

	//compute signed distance field
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			glm::vec2 pos = glm::vec2(x, y);
			auto val = bitMapData[y*width + x];

			float sign = val > 0 ? 1.0f : -1.0f;

			// found nearest opposite texel
			float dist = std::numeric_limits<float>::max();

			for (int s = 0; s < width; ++s)
			{
				for (int t = 0; t < height; ++t)
				{
					glm::vec2 curPos = glm::vec2(s, t);
					auto curVal = bitMapData[t*width + s];

					if (curVal == val)
						continue;

					float curDist = glm::length(curPos - pos);
					dist = std::min(dist, curDist);
				}
			}

			float newVal = sign * dist;
			sdf[y*width + x] = newVal;

			minDist = std::min(minDist, newVal);
			maxDist = std::max(maxDist, newVal);
		}
	}

	std::cout << "min: " << minDist << " , max:" << maxDist << std::endl;

	// normalize data
	unsigned char* finalData = new unsigned char[width * height];
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			int idx = y*width + x;
			finalData[idx] =  (sdf[idx] - minDist) / (maxDist - minDist) * 255;
		}
	}

	delete[] sdf;
	delete[] bitMapData;
	*outData = finalData;
}


void TextString::unpackMonoBitmap(FT_Bitmap* bitmap, unsigned char** outData)
{	
	int width = bitmap->width;
	int height = bitmap->rows;

	GLubyte *data = new unsigned char[width * height];

	for (int y = 0; y < bitmap->rows; ++y)
	{
		for (int byteIndex = 0; byteIndex < bitmap->pitch; ++byteIndex)
		{
			auto byte_value = bitmap->buffer[y * bitmap->pitch + byteIndex];
			int num_bits_done = byteIndex * 8;
			int row_start = y * bitmap->width + byteIndex * 8;

			for (int bitIndex = 0; bitIndex < std::min(8, bitmap->width - num_bits_done); ++bitIndex)
			{
				auto bit = byte_value & (1 << (7 - bitIndex));

				if(bit)
					data[row_start + bitIndex] = 255;
				else
					data[row_start + bitIndex] = 0;
			}
		}
	}

	*outData = data;
}
