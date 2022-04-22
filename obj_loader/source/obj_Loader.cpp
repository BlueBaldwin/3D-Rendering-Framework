//\------------------------------------------------------------------------------------------
//\ OBJ PARSER - Scan throught the contents of an OBJ Model and organise its contents to be passed into our Main application
//\------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "obj_loader.h"

void OBJModel::unload()
{
	m_meshes.clear();
}

bool OBJModel::load(const char* a_filename, float a_scale)
{
	std::cout << "Attempting to open file: " << a_filename << std::endl;
	// Get an fstream to read in the file data
	std::fstream file;
	file.open(a_filename, std::ios_base::in | std::ios_base::binary);
	// Test to see if the file has opened successfully
	if (file.is_open())
	{
		std::cout << "Successfully opened" << std::endl;
		//Get File path information 
		std::string filePath = a_filename;
		size_t path_end = filePath.find_last_of("\/\\");
		if (path_end != std::string::npos)
		{
			filePath = filePath.substr(0, path_end + 1);
		}
		else
		{
			filePath = "";
		}
		m_path = filePath;

		// Success file has been opened, verify contents of file -- i.e. check that the file is not zero length
		file.ignore(std::numeric_limits<std::streamsize>::max());	// attempts to read the highest number of bytes from the file
		std::streamsize fileSize = file.gcount();					// gCount will have reached an EOF marker, letting us know the byte size
		file.clear();												// clear EOF marker from being read
		file.seekg(0, std::ios_base::beg);							// seek back to the beginning of the file
		if (fileSize == 0)											// Close the file and return if it's empty
		{
			std::cout << "File contains no data, closing file" << std::endl;
			file.close();
		}
		std::cout << "File Size: " << fileSize / 1024 << " KB" << std::endl;

		OBJMesh* currentMesh = nullptr;
		std::string fileLine;
		std::vector<glm::vec4> vertexData;
		std::vector<glm::vec4> normalData;
		std::vector<glm::vec2> UVData;
		// Store our material in a string as face data is not generated prior to material assignment and may not have a mesh
		OBJMaterial* currentMtl = nullptr;
		//Set up reading in chunks of a file at a time
		while (!file.eof())
		{
			if (std::getline(file, fileLine))
			{
				if (fileLine.size() > 0)
				{
					std::string dataType = lineType(fileLine);
					// If datatype has a 0 length then skip all tests and continue to next line.
					if (dataType.length() == 0) { continue; } 
					std::string data = lineData(fileLine);

					if (dataType == "#")
					{
						std::cout << data << std::endl;
						continue;
					}
					if (dataType == "mtllib")
					{
						std::cout << "Material File: " << data << std::endl;
						// Load in Material file so that materials can be used as required
						LoadMaterialLibrary(data);
						continue;
					}
					if (dataType == "g" || dataType == "o")
					{
						std::cout << "OBJ Group Found: " << data << std::endl;
						// We can use group tags to split our model up into smaller mesh components
						if (currentMesh != nullptr)
						{
							m_meshes.push_back(currentMesh);
						}
						currentMesh = new OBJMesh();
						currentMesh->m_name = data;
						if (currentMtl != nullptr) // If we have a material name
						{
							currentMesh->m_material = currentMtl;
							currentMtl = nullptr;
						}
					}
					if (dataType == "v")
					{
						glm::vec4 vertex = processVectorString(data);
						vertex *= a_scale;				// HARD CODED multipy factor to the passed in vector to scale the model
						vertex.w = 1.f;					// As this is positional data ensure the w component is set to 1.0f
						vertexData.push_back(vertex);
						continue;
					}
					if (dataType == "vt")
					{
						glm::vec4 uvCoordv4 = processVectorString(data);
						glm::vec2 uvCoord = glm::vec2(uvCoordv4.x, uvCoordv4.y);
						UVData.push_back(uvCoord);
						continue;
					}
					if (dataType == "vn")
					{
						glm::vec4 normal = processVectorString(data);
						normal.w = 0.f;
						normalData.push_back(normal);
						continue;
					}
					if (dataType == "f")
					{
						if (currentMesh == nullptr) // We have entered processing faces without having hit a 'o' or 'g' tag
						{
							currentMesh = new OBJMesh();
							if (currentMtl !=  nullptr)	// We have a marterial name
							{
								currentMesh->m_material = currentMtl;
								currentMtl = nullptr;
							}
						}
						// Process face data
						// Face consists of 3 -> more vertices split at ' ' then at '/' characters
						std::vector<std::string> faceData = SplitStringAtCharacter(data, ' ');
						unsigned int ci = currentMesh->m_vertices.size();
						for (auto iter = faceData.begin(); iter != faceData.end(); ++iter)
						{
							// Process face triplet
							obj_face_triplet triplet = ProcessTriplet(*iter);
							// Triplet processed now set vertex data from position/normal/texture data
							OBJVertex currentVertex;
							currentVertex.position = vertexData[triplet.v - 1];
							if (triplet.vn != 0)
							{
								currentVertex.normal = normalData[triplet.vn - 1];
							}
							if (triplet.vt != 0)
							{
								currentVertex.uvcoord = UVData[triplet.vt - 1];
							}
							currentMesh->m_vertices.push_back(currentVertex);
						}
						// All face information for the tri/quad/fan have been collected
						// Time to index these into the current mesh
						// Test to see if OBJ file contains normal data if normalData is empty then there are no normals
						bool calcNormals = normalData.empty();
						for (unsigned int offset = 1; offset < (faceData.size() - 1); ++offset)
						{
							currentMesh->m_indices.push_back(ci);
							currentMesh->m_indices.push_back(ci + offset);
							currentMesh->m_indices.push_back(ci + 1 + offset);
							// If we need to calculate normals we can do that here
							if (calcNormals)
							{
								glm::vec4 normal = currentMesh->calculateFaceNormal(
									ci,
									ci + offset,
									ci + offset + 1);
								currentMesh->m_vertices[ci].normal = normal;
								currentMesh->m_vertices[ci + offset].normal = normal;
								currentMesh->m_vertices[ci + offset + 1].normal = normal;
							}
						}
					}
					if (dataType == "usemtl")
					{
						// We have a material to use on the current mesh
						OBJMaterial* mtl = getMaterialByName(data.c_str());
						if (mtl != nullptr)
						{
							currentMtl = mtl;
							if(currentMesh != nullptr)
							{
								currentMesh->m_material = currentMtl;
							}
						}
					}
				}
			}
		}
		
		auto [minVec, maxVec] = BoundingBox(vertexData);

		if (currentMesh != nullptr)
		{
			m_meshes.push_back(currentMesh);
		}
		file.close();
		return true;
	}
	return false;
}

// Cycle through the entire model and generate face normals.
// Getting the Normalised vertex direction from A - B (AB) & A - C (AC)
// Then performing the cross production function to get the surface normal of the face 
glm::vec4 OBJMesh::calculateFaceNormal(const unsigned int& a_indexA, const unsigned int& a_indexB, const unsigned int& a_indexC) const
{
	glm::vec3 a = m_vertices[a_indexA].position;
	glm::vec3 b = m_vertices[a_indexB].position;
	glm::vec3 c = m_vertices[a_indexC].position;

	glm::vec3 ab = glm::normalize(b - a);
	glm::vec3 ac = glm::normalize(c - a);

	return glm::vec4(glm::cross(ab, ac), 0.f);
}

void OBJMesh::calculateFaceNormals()
{
	// As our indexed triangle array contains a tri for each three points we can iterater through this vector and calculate a face normal
	for (int i = 0; i < m_indices.size(); i += 3)
	{
		glm::vec4 normal = calculateFaceNormal(i, i + 1, i + 2);
		// Set face normal to each vertex for the tri
		m_vertices[i].normal = m_vertices[i + 1].normal = m_vertices[i + 2].normal = normal;
	}
}

std::string OBJModel::lineType(const std::string& a_in)
{
	if (!a_in.empty())
	{
		size_t token_start = a_in.find_first_not_of(" \t");
		size_t token_end = a_in.find_first_of(" \t", token_start);
		// Test to see if the start token is valid, test to see if the end token is valid
		if (token_start != std::string::npos && token_end != std::string::npos)
		{
			return a_in.substr(token_start, token_end - token_start);
		}
		else if (token_start != std::string::npos)
		{
			return a_in.substr(token_start);
		}
	}
	return "";
}

void OBJModel::LoadMaterialLibrary(std::string a_mtllib)
{
	std::string matFile = m_path + a_mtllib;
	std::cout << "Attempting to load material file: " << matFile << std::endl;
	// get an fstream to read in the file data 
	std::fstream file;
	file.open(matFile, std::ios_base::in | std::ios_base::binary);
	// Test to see if the file has opened successfully
	if (file.is_open())
	{
		std::cout << "Material Library Successfully Opened" << std::endl;
		// Success the file has been opened, verify the contents of file -- i.e. cchet the file is not zero in length
		file.ignore(std::numeric_limits<std::streamsize>::max());	// attempt to read the highest number of bytes from the file
		std::streamsize fileSize = file.gcount();					// gCount will have reached EOF marker, letting us know number of bytes
		file.clear();												// Clear EOF marker from being read
		file.seekg(0, std::ios_base::beg);							// Seek back to the beginning of the file
		if (fileSize == 0)
		{
			std::cout << "File contains no data, closing file" << std::endl;
			file.close();
		}
		std::cout << "Material File Size: " << fileSize / 1024 << "KB" << std::endl;
		
		// Variable to store file data as it is read line by line
		std::string fileLine;
		OBJMaterial* currentMaterial = nullptr;
		while (!file.eof())
		{
			if (std::getline(file, fileLine))
			{
				if (fileLine.size() > 0)
				{
					std::string dataType = lineType(fileLine);
					// If datatype has a 0 length then skip all tests and continue to next line
					if (dataType.length() == 0) { continue; }
					std::string data = lineData(fileLine);

					if (dataType == "#") // Comment line
					{
						std::cout << data << std::endl;
						continue;
					}
					if (dataType == "newmtl")		//Newmtl – this keyword indicates that there is a new material in the file and the remaining data section of the line is the name of the material
					{
						std::cout << "New Material Found: " << data << std::endl;
						if (currentMaterial != nullptr)
						{
							m_materials.push_back(currentMaterial);
						}
						currentMaterial = new OBJMaterial();
						currentMaterial->Set_name(data);
						continue;
					}
					if (dataType == "Ns")	//Ns – this is a floating point value that is the specular power that we use in the calculation of the specular term in our fragment shader
					{						//e.g specularTerm = pow(max(0.f, dot(E,ReflectedL), Ns)
						if (currentMaterial != nullptr)
						{
							// NS is guaranteed to be a single float value
							glm::vec4 kS = currentMaterial->Get_kS();
							kS.a = std::stof(data);
							currentMaterial->Set_kS(kS);
						}
						continue;
					}
					if (dataType == "Ka")	// Ka – this is the RGB colour of the ambient light for this mesh in the example it is white.
					{
						if (currentMaterial != nullptr)
						{
							// Process kA as a vector string
							glm::vec4 kA = currentMaterial->Get_kA();	// Store alpha channel as may contain refractive index
							glm::vec4 kA_a = processVectorString(data);
							kA_a.a = kA.a;
							currentMaterial->Set_kA(kA_a);
						}
						continue;
					}
					if (dataType == "Kd")	// Kd – this is the colour of the diffuse channel/light for the mesh this material is applied to
					{
						if (currentMaterial != nullptr)
						{
							// Process kD as a vector string
							glm::vec4 kD = currentMaterial->Get_kD();	// Store alpha channel as may contain refractive index
							glm::vec4 kD_a = processVectorString(data);
							kD_a.a = kD.a;
							currentMaterial->Set_kD(kD_a);
						}
						continue;
					}

					if (dataType == "Ks")	// Ks – this is the specular highlight colour rgb value
					{
						if (currentMaterial != nullptr)
						{
							// Process Ks as a vector string
							glm::vec4 Ks = currentMaterial->Get_kS();	// Store alpha channel as may contain refractive index
							glm::vec4 Ks_a = processVectorString(data);
							Ks_a.a = Ks.a;
							currentMaterial->Set_kS(Ks_a);
						}
						continue;
					}
					if (dataType == "Ke")	// Ke – this is the emissive colour of the mesh
					{
						// KE is for emissive properties
						// We will not need to support this for our purposes
						continue;
					}

					if (dataType == "Ni")	// Ni – this is the floating point value for the refractive index of the material.
					{
						if (currentMaterial != nullptr)
						{
							// This is the refractive index of the mesh (how light bends as it passes through the material)
							// We will store this in the alpha componenet of the ambient light values (kA)
							glm::vec4 kA = currentMaterial->Get_kA();
							kA.a = std::stof(data);
							currentMaterial->Set_kA(kA);
						}
					}
					if (dataType == "d" || dataType == "Tr")	// Tr - Transparancy/Opacity Tr - 1 - d
					{											// d – this is the dissolve value or transparency for the material 1 is solid, anything less than one allows light to pass through the material
						if (currentMaterial != nullptr)
						{
							// This is disolve or alpha value of the material we will store in the kD alpha channel
							glm::vec4 kD = currentMaterial->Get_kD();
							kD.a = std::stof(data);
							if (dataType == "Tr")
							{
								kD.a = 1.f - kD.a;
							}
						}
						continue;
					}
					if (dataType == "Illum")	// Illum – this is an integer value used to describe the lighting model to be applied to the material. We will ignore this for now.
					{
						// Illum describes the illumination model used to light the model
						// Ignore this for now as we will light the scene our own way
						continue;
					}
					if (dataType == "map_Kd") //diffuse texture
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::DiffuseTexture] =
							m_path + mapData[mapData.size() - 1]; // We are only interested in the file name
						//a nd other data is garbage as far as our loader is concerned
						continue;
					}
					if (dataType == "map_Ks") // Specular texture
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::SpecularTexture] =
							m_path + mapData[mapData.size() - 1]; // We are only interested in the file name
						//and other data is hot garbage as far as our loader is concerned
						continue;
					}
					// Annoyingly again OBJ can use bump or map_bump for normal map textures
					if (dataType == "map_bump" || dataType == "bump") //normal map texture
					{
						std::vector<std::string> mapData = SplitStringAtCharacter(data, ' ');
						currentMaterial->textureFileNames[OBJMaterial::TextureTypes::NormalTexture] =
							m_path + mapData[mapData.size() - 1]; // We are only interested in the file name
						continue;
					}
				}
			}
		}
		if (currentMaterial != nullptr)
		{
			m_materials.push_back(currentMaterial);
		}
		file.close();
	}
	
}

std::string OBJModel::lineData(const std::string& a_in)
{
	// Get the token part of the line
	size_t token_start = a_in.find_first_not_of(" \t");
	size_t token_end = a_in.find_first_of(" \t", token_start);
	// FInd the data part of the current line
	size_t data_start = a_in.find_first_not_of(" \t", token_end);
	size_t data_end = a_in.find_last_not_of(" \t\n\r");

	if (data_start != std::string::npos && data_end != std::string::npos)
	{
		return a_in.substr(data_start, data_end - data_start + 1);
	}
	else if (data_start != std::string::npos)
	{
		return a_in.substr(data_start);
	}
	return "";
}

glm::vec4 OBJModel::processVectorString(const std::string a_data)
{
	//split the line data at eash sapce character and store this as a float value within a glm::vec4
	std::stringstream iss(a_data);
	glm::vec4 vecData = glm::vec4(0.f);
	int i = 0;
	// For loop to loop until iss cannot stream data into val
	for (std::string val; iss >> val; ++i)
	{
		// use std::string to float function
		float fVal = std::stof(val);
		// cast vec4 to float* to allow iteration through elements of vec4 - casting to get the correct index
		vecData[i] = fVal;
	}
	return vecData;
}

// A function that splits a string at a given character into an array e.g. splitting faces into their face triplets with the space
// character and to split those triplets into seperate indicies with the '/' character
std::vector<std::string> OBJModel::SplitStringAtCharacter(std::string data, char a_character)
{
	std::vector<std::string> lineData;
	// Split the line data at each space character and store this as a float value within a glm::vec4
	std::stringstream iss(data);
	std::string lineSegment;
	// provinding a character to the getline  function splits the line at occurances of that character
	while (std::getline(iss, lineSegment, a_character))
	{
		// Push each line segment into a vector
		lineData.push_back(lineSegment);
	}
	return lineData;
}

OBJModel::obj_face_triplet OBJModel::ProcessTriplet(std::string a_triplet)
{
	std::vector<std::string> vertexIndices = SplitStringAtCharacter(a_triplet, '\/');
	obj_face_triplet ft;
	ft.v = 0; ft.vn = 0; ft.vt = 0;
	ft.v = std::stoi(vertexIndices[0]);
	if (vertexIndices.size() >= 2) {
		if (vertexIndices[1].size() > 0)
		{
			ft.vt = std::stoi(vertexIndices[1]);
		}
		if (vertexIndices.size() >= 3)
		{
			ft.vn = std::stoi(vertexIndices[2]);
		}
	}
	return ft;
}

OBJMesh* OBJModel::getMeshByIndex(unsigned int a_index)
{
	unsigned int meshCount = m_meshes.size();
	if (meshCount > 0 && a_index < meshCount)
	{
		return m_meshes[a_index];
	}
	return nullptr;
}

OBJMaterial* OBJModel::getMaterialByName(const char* a_name)
{
	for (auto iter = m_materials.begin(); iter != m_materials.end(); ++iter)
	{
		OBJMaterial* mat = (*iter);
		std::string matName = mat->Get_name();
		if(matName == a_name)
		{
			return mat;
		}
	}
	return nullptr;
}

OBJMaterial* OBJModel::getMaterialByIndex(unsigned int a_index)
{
	unsigned int materialCount = m_materials.size();
	if (materialCount > 0 && a_index < materialCount)
	{
		return m_materials[a_index];
	}
	return nullptr;
}

//\------------------------------------------------------------------------------------------
// Bounding box Min / Max Vector
//\------------------------------------------------------------------------------------------
std::pair<glm::vec4, glm::vec4> OBJModel::BoundingBox(const std::vector<glm::vec4>& vertexData)
{
	// Finding the minimum and maximum vectors within vectorData to create the two points of a bounding box
	glm::vec4 minVec = { std::numeric_limits<short>::max(),std::numeric_limits<short>::max(),std::numeric_limits<short>::max(),1.f };
	glm::vec4 maxVec = { -std::numeric_limits<short>::max(),-std::numeric_limits<short>::max(),-std::numeric_limits<short>::max(),1.f };
	for (auto& vertex : vertexData)
	{
		if (minVec.x > vertex.x) { minVec.x = vertex.x; }
		if (minVec.y > vertex.y) { minVec.y = vertex.y; }
		if (minVec.z > vertex.z) { minVec.z = vertex.z; }

		if (maxVec.x < vertex.x) { maxVec.x = vertex.x; }
		if (maxVec.y < vertex.y) { maxVec.y = vertex.y; }
		if (maxVec.z < vertex.z) { maxVec.z = vertex.z; }
	}
	return{ minVec, maxVec };
}
