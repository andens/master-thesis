#include "obj-loader.h"

#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../mesh/mesh.h"

using namespace std;
using namespace DirectX;

OBJLoader::OBJLoader() :
mCurrentMaterial( 0 )
{
	Material defaultMaterial;
	defaultMaterial.Name = "";
	defaultMaterial.Indices.push_back( 0 ); // Start location
	mMaterials.push_back( defaultMaterial );
}

OBJLoader::~OBJLoader()
{

}

// TODO: If tex coords or normals are omitted, don't create defaults; just
// skip them. Take a look through the whole thing, there will need to be
// checks on stuff that has to do with texcoords and normals.
Mesh* OBJLoader::Load( const char *filename )
{
	this->~OBJLoader();
	new (this) OBJLoader();

	// Open file for read.
	ifstream fin( filename );

	if ( !fin )
		return nullptr;

	char curChar;
	while ( fin )
	{
		curChar = fin.get(); // Get next char.

		switch ( curChar )
		{
		case 'v':			ReadVertexDescriptions( fin ); break;
		case 'f':			ReadFace( fin ); break;
		//case 'u':			ReadUsemtl( fin ); break;
		case 'g':			ReadGroup( fin ); break;
		default:			if ( curChar != '\n' ) ReadComment( fin ); break;
		}
	}

	fin.close();

	// Fix the last material.
	mMaterials[mCurrentMaterial].Indices.push_back( mPositionIndices.size() );

	// Remove the default material if it's end location is 0 (no vertices).
	// This happens when a material is specified directly (as it should be).
	if ( mMaterials[0].Indices[1] == 0 )
		mMaterials.erase( mMaterials.begin() );

	// Remove empty groups
	for ( auto it = mMaterials.begin(); it != mMaterials.end(); ++it )
	{
		if ( it->Indices[0] == it->Indices[1] )
			it = mMaterials.erase( it );
	}

	Mesh *retVal = new Mesh();

	unsigned materialStart = 0;
	unsigned *arrangedPositions = new unsigned[mPositionIndices.size()];
	unsigned *arrangedTexCoords = new unsigned[mTexCoordIndices.size()];
	unsigned *arrangedNormals = new unsigned[mNormalIndices.size()];

	// The same material can occur several times in a file, each time adding
	// a chunk of indices. Sort the indices on material by moving all chunks
	// belonging to a certain material together.
	for ( unsigned material = 0; material < mMaterials.size(); ++material )
	{
		unsigned accumIndices = 0;
		std::vector<unsigned> &materialIndices = mMaterials[material].Indices;

		// Loop through every occurence of the material...
		for ( unsigned i = 0; i < materialIndices.size(); i += 2 )
		{
			unsigned chunkIndexCnt = materialIndices[i + 1] - materialIndices[i];

			// ...and copy indices to the correct location.
			memcpy( arrangedPositions + materialStart + accumIndices, &mPositionIndices[0] + materialIndices[i], sizeof( unsigned ) * chunkIndexCnt );
			memcpy( arrangedTexCoords + materialStart + accumIndices, &mTexCoordIndices[0] + materialIndices[i], sizeof( unsigned ) * chunkIndexCnt );
			
			if ( mNormals.size() )
				memcpy( arrangedNormals + materialStart + accumIndices, &mNormalIndices[0] + materialIndices[i], sizeof( unsigned ) * chunkIndexCnt );

			accumIndices += chunkIndexCnt;
		}

		retVal->AddBatch( materialStart, accumIndices );

		materialStart += accumIndices;
	}

	retVal->AddAttributeStream( Mesh::AttributeType::Position, mPositions.size(), (float*)&mPositions[0], mPositionIndices.size(), arrangedPositions );
	retVal->AddAttributeStream( Mesh::AttributeType::TexCoord, mTexCoords.size(), (float*)&mTexCoords[0], mTexCoordIndices.size(), arrangedTexCoords );
	if ( mNormals.size() )
		retVal->AddAttributeStream( Mesh::AttributeType::Normal, mNormals.size(), (float*)&mNormals[0], mNormalIndices.size(), arrangedNormals );

	delete[] arrangedPositions;
	delete[] arrangedTexCoords;
	delete[] arrangedNormals;

	return retVal;
}

void OBJLoader::Save( const char *filename, const Mesh *mesh )
{
	int positionStream = mesh->FindStream( Mesh::AttributeType::Position );
	int texCoordStream = mesh->FindStream( Mesh::AttributeType::TexCoord );
	int normalStream = mesh->FindStream( Mesh::AttributeType::Normal );
	if ( positionStream == -1 )
		return;

	ofstream file( filename, ios::trunc );

	if ( file.is_open() )
	{
		file << fixed << setprecision( 4 );

		const unsigned *posIndices = mesh->AttributeIndices( positionStream );
		const unsigned *texIndices = nullptr;
		const unsigned *norIndices = nullptr;

		// Write positions
		{
			const vector<float>& data = mesh->AttributeData( positionStream );

			for ( unsigned i = 0; i < data.size(); i += 3 )
			{
				file << "v " << data[i] << " " << data[i + 1] << " " << data[i + 2] << endl;
			}
		}

		// Write tex coords if available
		if ( texCoordStream >= 0 )
		{
			texIndices = mesh->AttributeIndices( texCoordStream ); // For later

			const vector<float>& data = mesh->AttributeData( texCoordStream );

			for ( unsigned i = 0; i < data.size(); i += 2 )
			{
				file << "vt " << data[i] << " " << data[i + 1] << endl;
			}
		}

		// Write normals if available
		if ( normalStream >= 0 )
		{
			norIndices = mesh->AttributeIndices( normalStream ); // For later

			const vector<float>& data = mesh->AttributeData( normalStream );

			for ( unsigned i = 0; i < data.size(); i += 3 )
			{
				file << "vn " << data[i] << " " << data[i + 1] << " " << data[i + 2] << endl;
			}
		}

		file << endl;

		const vector<Mesh::Batch> batches = mesh->Batches();
		for ( unsigned b = 0; b < batches.size(); ++b )
		{
			file << "usemtl Mat" << b << endl;

			// Write the faces
			for ( unsigned i = 0; i < batches[b].IndexCount; i += 3 )
			{
				unsigned index = batches[b].StartIndex + i;
				file << "f";

				// Three vertices in a face
				for ( unsigned v = 0; v < 3; ++v )
				{
					file << " " << posIndices[index + v] + 1;

					if ( texIndices )
					{
						file << "/" << texIndices[index + v] + 1;
					}
					else if ( norIndices )
					{
						// No tex coords, but normals incoming; write the slash.
						// Now we will have 'f pos//norm'
						file << "/";
					}

					if ( norIndices )
					{
						file << "/" << norIndices[index + v] + 1;
					}
				}

				file << endl;
			}

			file << endl;
		}

		file.close();
	}
	else
	{
		throw std::runtime_error( "Failed to open/create OBJ file for saving." );
	}
}

// -----------------------------------------------------------------------------
// Reads a vertex attribute and stores the data into the appropriate stream.
// -----------------------------------------------------------------------------
void OBJLoader::ReadVertexDescriptions( std::ifstream& fin )
{
	float x, y, z;

	switch ( fin.get() )
	{
	case ' ': // v - position
		fin >> x >> y >> z;
		mPositions.push_back( XMFLOAT3( x, y, z ) );
		break;

	case 't': // vt - texture coordinate
		fin >> x >> y;
		mTexCoords.push_back( XMFLOAT2( x, y ) );
		break;

	case 'n': // vn - normal
		fin >> x >> y >> z;
		mNormals.push_back( XMFLOAT3( x, y, z ) );
		break;
	}

	fin.get(); // \n
}

// -----------------------------------------------------------------------------
// Reads an entire line defining a face, adding its vertices, and retriangulating
// if the polygon has more than three vertices.
// -----------------------------------------------------------------------------
void OBJLoader::ReadFace( std::ifstream& fin )
{
	char currChar = fin.get();

	// If the next character by any chance does not separate token and data
	// we stop right away.
	if ( currChar != ' ' )
		return;

	std::string face = ""; // Contains face vertices.
	unsigned vertexCount = 0; // Number of vertices in the face.

	// Read and store the face definition, keeping track of how many vertices it
	// is made up of.
	while ( currChar != '\n' )
	{
		face += currChar; // Add the char to our face string.
		currChar = fin.get(); // Get next character.
		if ( currChar == ' ' ) // If it's a space...
			vertexCount++; // ...increase our vertex count.
	}

	// If the face string is empty there's nothing to parse.
	if ( face.length() <= 0 )
		return;

	// Every vertex is separated by a space, but if the last character is not a
	// space the last vertex has not been taken into consideration, and so we
	// increment the vertex count.
	if ( face[face.length() - 1] != ' ' )
		vertexCount++;

	// Now that the line has been read, it's time to disect it into the individual
	// vertices. Each vertex string is parsed to find the indices of the elements
	// it consists of.
	istringstream ss( face );

	unsigned firstVIndex = mPositionIndices.size();

	// Loop through the vertices and parse their vertex definitions.
	for ( unsigned i = 0; i < vertexCount; ++i )
	{
		// If we are parsing the fourth or further vertex, we need to retriangulate
		// the face. For each vertex we create another triangle which reuses the
		// first vertex of the face and the last vertex of the previous triangle.
		// This method might not work for concave polygons.
		if ( i > 2 )
		{
			// Reuse the first vertex of the face (copy the indices of that vertex)
			mPositionIndices.push_back( mPositionIndices[firstVIndex] );
			mTexCoordIndices.push_back( mTexCoordIndices[firstVIndex] );
			mNormalIndices.push_back( mNormalIndices[firstVIndex] );

			// Reuse the last vertex of the previous triangle (size - 2 because
			// size - 1 is the one we just added for this triangle!)
			mPositionIndices.push_back( mPositionIndices[mPositionIndices.size() - 2] );
			mTexCoordIndices.push_back( mTexCoordIndices[mTexCoordIndices.size() - 2] );
			mNormalIndices.push_back( mNormalIndices[mNormalIndices.size() - 2] );
		}

		string vertDef;
		ss >> vertDef;
		unsigned positionIndex, texCoordIndex, normalIndex;
		ParseVertexDefinition( vertDef, positionIndex, texCoordIndex, normalIndex );

		mPositionIndices.push_back( positionIndex );
		mTexCoordIndices.push_back( texCoordIndex );
		mNormalIndices.push_back( normalIndex );
	}
}

// -----------------------------------------------------------------------------
// Indicates a new submesh
// -----------------------------------------------------------------------------
void OBJLoader::ReadGroup( std::ifstream& fin )
{
	fin.unget();

	// Now we want to switch to using another material. Before we do that however,
	// we want to tell the current material what index (not including) it ends at.
	mMaterials[mCurrentMaterial].Indices.push_back( mPositionIndices.size() );

	string line;
	getline( fin, line );
	string materialName = line.substr( 2 ); // Skip 'g '

	int matIndex = GetMaterial( materialName );

	// If the material does not already exist, we add it.
	if ( matIndex == -1 )
	{
		Material mat;
		mat.Name = materialName;

		mMaterials.push_back( mat );

		matIndex = mMaterials.size() - 1;
	}

	mCurrentMaterial = matIndex;
	mMaterials[mCurrentMaterial].Indices.push_back( mPositionIndices.size() ); // Start location
}

// -----------------------------------------------------------------------------
// Reads what material to use.
// -----------------------------------------------------------------------------
//void OBJLoader::ReadUsemtl( std::ifstream &fin )
//{
//	fin.unget();
//
//	// Now we want to switch to using another material. Before we do that however,
//	// we want to tell the current material what index (not including) it ends at.
//	mMaterials[mCurrentMaterial].Indices.push_back( mPositionIndices.size() );
//
//	string line;
//	getline( fin, line );
//	string materialName = line.substr( 7 ); // Skip 'usemtl '
//
//	int matIndex = GetMaterial( materialName );
//
//	// If the material does not already exist, we add it.
//	if ( matIndex == -1 )
//	{
//		Material mat;
//		mat.Name = materialName;
//
//		mMaterials.push_back( mat );
//
//		matIndex = mMaterials.size() - 1;
//	}
//
//	mCurrentMaterial = matIndex;
//	mMaterials[mCurrentMaterial].Indices.push_back( mPositionIndices.size() ); // Start location
//}

// -----------------------------------------------------------------------------
// Ignores a line by reading chars until end of line.
// -----------------------------------------------------------------------------
void OBJLoader::ReadComment( std::ifstream& fin )
{
	std::string dump;
	getline( fin, dump );
}

// -----------------------------------------------------------------------------
// Parses a vertex definition, returning the indices of the position, tex coord,
// and normal it consists of. If an attribute is unavailable, index 0 is used.
// -----------------------------------------------------------------------------
void OBJLoader::ParseVertexDefinition( const std::string &vertDef, unsigned &positionIndex, unsigned &texCoordIndex, unsigned &normalIndex )
{
	// Read the indices of the vertex definition.
	int ptn[] = { 0, 0, 0 };
	int n = 0;
	const char *loc = vertDef.c_str();
	for ( int i = 0; i < 3; ++i ) // Three indices in a vertdef
	{
		// We only store in one variable (number of characters read n does not
		// increase assignment count), therefore the return value can be used
		// as a success value, indicating a value was read or not.
		int success = sscanf_s( loc, "%i%n", &ptn[i], &n );
		loc += success * n + 1; // Increase pointer by number of read chars + 1 for slash.
		if ( loc - vertDef.c_str() >= strlen( vertDef.c_str() ) ) // Parsed entire string? Early break.
			break;
	}

	// Position index
	if ( ptn[0] > 0 )
		positionIndex = ptn[0] - 1;
	else
		positionIndex = mPositions.size() + ptn[0];

	// Tex coord index
	if ( ptn[1] > 0 )
		texCoordIndex = ptn[1] - 1;
	else if ( ptn[1] < 0 )
		texCoordIndex = mTexCoords.size() + ptn[1];

	// Normal index (if zero, we keep it like that)
	if ( ptn[2] > 0 )
		normalIndex = ptn[2] - 1;
	else if ( ptn[2] < 0 )
		normalIndex = mNormals.size() + ptn[2];
}

// --------------------------------------------------------------------------
// Returns the index to the materials array for a certain material, or -1 if
// the material name wasn't found.
// --------------------------------------------------------------------------
int OBJLoader::GetMaterial( const string &name )
{
	for ( int i = 0; i < mMaterials.size(); ++i )
	{
		if ( name == mMaterials[i].Name )
			return i;
	}

	return -1;
}