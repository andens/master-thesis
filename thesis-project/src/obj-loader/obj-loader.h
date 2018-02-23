#ifndef _OBJLOADER_H_
#define _OBJLOADER_H_

#include <DirectXMath.h>
#include <string>
#include <vector>

class Mesh;

class OBJLoader
{
public:
	OBJLoader();
	~OBJLoader();

	Mesh* Load( const char *filename );
	void Save( const char *filename, const Mesh *mesh );

private:
	struct Material
	{
		std::string Name;
		std::vector<unsigned> Indices; // Start- and end location in pairs.
	};

private:

	void ReadVertexDescriptions( std::ifstream& fin );
	void ReadFace( std::ifstream& fin );
	void ReadGroup( std::ifstream& fin );
	//void ReadUsemtl( std::ifstream& fin );
	void ReadComment( std::ifstream& fin );

	void ParseVertexDefinition( const std::string &vertDef, unsigned &positionIndex, unsigned &texCoordIndex, unsigned &normalIndex );
	int GetMaterial( const std::string &name );

private:
	std::vector<DirectX::XMFLOAT3> mPositions;
	std::vector<DirectX::XMFLOAT2> mTexCoords;
	std::vector<DirectX::XMFLOAT3> mNormals;
	std::vector<unsigned> mPositionIndices;
	std::vector<unsigned> mTexCoordIndices;
	std::vector<unsigned> mNormalIndices;

	std::vector<Material> mMaterials;
	unsigned mCurrentMaterial;
};

#endif // _OBJLOADER_H_