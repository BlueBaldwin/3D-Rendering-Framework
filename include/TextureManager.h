#pragma once
#include <map>
#include <string>

class Texture; //Forward declare Texture as we only need to keep a pointer here and this avoids cyclic dependency.

class TextureManager
{
public:
	// Manager Class will act as a Singleton object for easy access

	static TextureManager* CreateInstance();
	static TextureManager* GetInstance();
	static void DestroyInstance();
	
	bool TextureExists(const char* a_pName);
	
	//load a texture from file --> calls Texture::load()
	unsigned int	LoadTexture(const char* a_pfilename);
	unsigned int	GetTexture(const char* a_filename);
	void			ReleaseTexture(unsigned int a_texture);

private:

	static TextureManager* m_instance;
	// A small structure to reference count a texture
	// references count indicates how many pointers are
	// currently pointing to this texture -> only unload at @ refs
	typedef struct TextureRef
	{
	Texture* pTexure;
	unsigned int refCount;
	} TextureRef;
	
	std::map<std::string, TextureRef> m_pTextureMap;
	
	TextureManager();
	~TextureManager();
};