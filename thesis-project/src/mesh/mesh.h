#ifndef _MESH_H_
#define _MESH_H_

#include <vector>

class Mesh
{
public:
	//  unused  unique  components  byte size
	// |--------|--------|--------|--------|
	enum class AttributeType : unsigned
	{
		Position = (1u << 16) | (3u << 8) | 12u,
		TexCoord = (2u << 16) | (2u << 8) | 8u,
		Normal = (3u << 16) | (3u << 8) | 12u,
		Binormal = (4u << 16) | (3u << 8) | 12u,
		Tangent = (5u << 16) | (3u << 8) | 12u
	};

	struct AttributeStream
	{
		AttributeType Type;
		std::vector<float> Attributes;
		unsigned *Indices;
	};

	struct Batch
	{
		unsigned StartIndex;
		unsigned IndexCount;
	};

public:
	Mesh( void );
	~Mesh();

	unsigned IndexCount() const { return _IndexCount; }
	unsigned BatchCount() const { return _Batches.size(); }

	// Assumes that the indices are arranged to match batches.
	void AddAttributeStream( AttributeType type, unsigned attributeCount, float *attributes, unsigned indexCount, unsigned *indices );
	void AddBatch( unsigned startIndex, unsigned indexCount );

	int FindStream( AttributeType type ) const;
	const std::vector<float>& AttributeData( int stream ) const { return _AttributeStreams[stream].Attributes; }
	const unsigned* AttributeIndices( int stream ) const { return _AttributeStreams[stream].Indices; }
	const std::vector<Batch>& Batches( void ) const { return _Batches; }

	unsigned FixTJunctions( void );
	void FlipPositionZ( void );
	void FlipNormals( void );
	void InvertV( void );
	// Calculate tangent space (should be able to do using positions and uv) http://www.terathon.com/code/tangent.html

	inline static unsigned Components( AttributeType type ) { return ((static_cast<unsigned>(type) >> 8) & 0xFF); }
	inline static unsigned ByteSize( AttributeType type ) { return (static_cast<unsigned>(type) & 0xFF); }

private:
	Mesh( const Mesh& rhs );
	Mesh& operator=(const Mesh& rhs);

private:
	unsigned _IndexCount { 0 };
	std::vector<AttributeStream> _AttributeStreams;
	std::vector<Batch> _Batches;
};

#endif // _MESH_H_