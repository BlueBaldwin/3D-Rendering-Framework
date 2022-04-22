#pragma once
#include <string>
#include <vector>

// A class to store texture data
// A texture is a data buffer that contains values which relate to pixel colours

class Texture
{
public:
	Texture();
	~Texture();

	// Function to load a texture from file
	bool Load(std::string a_filename);
	void unload();
	// Get file name
	const std::string& GetFileName() const { return m_filename; }
	unsigned int GetTextureID() const { return m_textureID; }
	void GetDimensions(unsigned int& a_w, unsigned int& a_h) const;
	unsigned int LoadCubeMap(std::vector<std::string> a_filenames, unsigned int* cubemap_face_id);

private:
	std::string m_filename;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_textureID;
};

inline void Texture::GetDimensions(unsigned int& a_w, unsigned int& a_h) const
{
	a_w = m_width; a_h = m_height;
}

