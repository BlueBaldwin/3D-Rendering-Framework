#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

// A basic class for an OBJ file, supports vertex position, vertex normal, vertex uv Coord
class OBJVertex
{
public:
	enum VertexAttributeFlags
	{
		POSITION	= (1 << 0),		// The Position of the Vertex
		NORMAL		= (1 << 1),		// The Normal for the vertex
		UVCOORD		= (1 << 2),		// The UV Coordinates for the Vertex
	};

	enum Offsets
	{
		PositionOffset	= 0,
		NormalOffset	= PositionOffset + sizeof(glm::vec4),
		UVCoordOffset	= NormalOffset + sizeof(glm::vec4),
	};

	OBJVertex();
	~OBJVertex();

	// Currently public variables ++++++++ TURN INTO GETTER AND SETTER FUNCTIONS ++++++++++++++
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec2 uvcoord;

	bool operator == (const OBJVertex& a_rhs) const;
	bool operator < (const OBJVertex& a_rhs) const;

};
// Inline constructor destructor for OBJVertex clas
inline OBJVertex::OBJVertex() : position(0, 0, 0, 1), normal(0, 0, 0, 0), uvcoord(0, 0) {}
inline OBJVertex::~OBJVertex() {}
// Inline comparitor methods for OBJVertex
inline bool OBJVertex::operator == (const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(const OBJVertex)) == 0;
}
inline bool OBJVertex::operator < (const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(const OBJVertex)) < 0;
}

/// <summary>
/// An OBJ Material - all Materials have properties such as lights, textures, roughness
/// </summary>

class OBJMaterial
{
public:
	OBJMaterial() : name(), kA(0.f), kD(0.f), kS(0.f) {};
	~OBJMaterial();

	
//************************************************************
	// Getters and setters
	const std::string Get_name() const { return name; };
	void Set_name(std::string_view  set_name) { name = set_name; }
	
	const glm::vec4& Get_kA() const { return kA; };
	void Set_kA(const glm::vec4& set_kA) { kA = set_kA; }
	
	const glm::vec4& Get_kD() const { return kD; };
	void Set_kD(const glm::vec4& set_kD) { kD = set_kD; }
	
	const glm::vec4& Get_kS() const { return kS; };
	void Set_kS(const glm::vec4& set_kS) { kS = set_kS; }

	
//************************************************************

//enum for the texture our OBJ model will support
	enum TextureTypes
	{
		DiffuseTexture = 0,
		SpecularTexture,
		NormalTexture,

		TextureTypes_Count
	};
	//texture will have filenames for loading, once loaded ID's stored in ID array
	std::string textureFileNames[TextureTypes_Count];
	unsigned int textureIDs[TextureTypes_Count];

private:
	std::string		name;
	// Colour and illumination variables 
	glm::vec4		kA;		// Ambient light colour - alpha component stores Optical Density (Ni)(Refraction Index 0.001 - 10)
	glm::vec4		kD;		// Diffuse light colour - alpha component stores dissolve (d)(0-1)
	glm::vec4		kS;		// Specular Light colour (exponent stored in alpha)
};

// An OBJ model can be composed of many meshes. Much like any 3D model
// Lets use a class to store individual mesh data

class OBJMesh
{
public:
	OBJMesh();
	~OBJMesh();

	glm::vec4 calculateFaceNormal(const unsigned int& a_indexA, const unsigned int& a_indexB, const unsigned int& a_indexC) const;
	void calculateFaceNormals();

	std::string					m_name;
	std::vector<OBJVertex>		m_vertices;
	std::vector<unsigned int>	m_indices;
	OBJMaterial*				m_material{};
};

inline OBJMesh::OBJMesh() {}
inline OBJMesh::~OBJMesh() {};

class OBJModel
{
public:
	OBJModel() : m_worldMatrix(glm::mat4(1.f)), m_path(), m_meshes() {};
	~OBJModel()
	{
		unload();	//function to unload any data loaded in from file
	};
	// Load from file function
	bool				load(const char* a_filename, float a_scale = 0.05f);		
	// Function to unload and free memory
	void				unload();
	// Function to retrieve path, number of meshed and world matrix of model
	const char*			getPath()			const { return m_path.c_str(); }
	unsigned int		getMeshCount()		const { return m_meshes.size(); }
	unsigned int		getMaterialCount()	const { return m_materials.size(); }
	const glm::mat4&	getWorldMatrix()	const { return m_worldMatrix; }
	// Functions to retrieve mesh by name or index for models that contain multiple meshes
	OBJMesh*			getMeshByName(const char* a_name);
	OBJMesh*			getMeshByIndex(unsigned int a_index);
	OBJMaterial*		getMaterialByName(const char* a_name);
	OBJMaterial*		getMaterialByIndex(unsigned a_index); 

private:
	// fucntion to process line data read in from file
	std::string lineType(const std::string& a_in);
	std::string lineData(const std::string& a_in);
	glm::vec4 processVectorString(const std::string a_data);
	std::vector<std::string> SplitStringAtCharacter(std::string data, char a_character);
	void LoadMaterialLibrary(std::string a_mtllib);
	std::pair<glm::vec4, glm::vec4> BoundingBox(const std::vector<glm::vec4>& vertexData);

	// OBJ face triplet struct;
	typedef struct obj_face_triplet
	{
		unsigned int v;
		unsigned int vt;
		unsigned int vn;
	} obj_face_triplet;

	// function to extract triplet data from OBJ file
	obj_face_triplet ProcessTriplet(std::string a_triplet);

	// Vector to store mesh data 
	std::vector<OBJMesh*> m_meshes;
	// Path to model data - useful for things like texture lookups
	std::string m_path;
	// Root Mat4 (world matrix)
	glm::mat4 m_worldMatrix;
	// reading data from a current material pointer into OBJMaterial object
	std::vector<OBJMaterial*> m_materials;
};
