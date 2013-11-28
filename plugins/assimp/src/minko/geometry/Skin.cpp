/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Skin.hpp"

#include <minko/scene/Node.hpp>
#include <minko/math/Matrix4x4.hpp>
#include <minko/geometry/Bone.hpp>

using namespace minko;
using namespace minko::scene;
using namespace minko::math;
using namespace minko::geometry;

Skin::Skin(unsigned int numBones, unsigned int numFrames):
	_bones(numBones, nullptr),
	_numBones(numBones),
	_duration(0.0f),
	_timeFactor(0.0f),
	_boneMatricesPerFrame(numFrames, std::vector<float>(numBones << 4, 0.0f)),
	_maxNumVertexBones(0),
	_numVertexBones(),
	_vertexBones(),
	_vertexBoneWeights()
{

}

void
Skin::clear()
{
	_bones.clear();
	_boneMatricesPerFrame.clear();
	_duration	= 0.0f;
	_timeFactor	= 0.0f;
	_maxNumVertexBones	= 0;
	_numVertexBones.clear();
	_vertexBones.clear();
	_vertexBoneWeights.clear();
}

void
Skin::duration(float value)
{
	if (value < 1e-6f)
		throw std::invalid_argument("value");

	_duration	= value;
	_timeFactor	= numFrames() / _duration; 
}

void
Skin::matrix(unsigned int	frameId, 
			 unsigned int	boneId, 
			 Matrix4x4::Ptr	value)
{
#ifdef DEBUG_SKINNING
	assert(frameId < numFrames() && boneId < numBones());
#endif // DEBUG_SKINNING

	memcpy(
		&(_boneMatricesPerFrame[frameId][boneId << 4]), 
		&(value->data()[0]), 
		sizeof(float)*16
	);

	//_boneMatricesPerFrame[frameId][boneId] = value;
}

Skin::Ptr
Skin::reorganizeByVertices()
{
	_numVertexBones.clear();
	_vertexBones.clear();
	_vertexBoneWeights.clear();

	const unsigned int lastId		= lastVertexId();
	const unsigned int numVertices	= lastId + 1;
	const unsigned int numBones		= _bones.size();

	_numVertexBones		.resize(numVertices,			0);
	_vertexBones		.resize(numVertices * numBones, 0);
	_vertexBoneWeights	.resize(numVertices * numBones, 0.0f);

	for (unsigned int boneId = 0; boneId < numBones; ++boneId)
	{
		auto bone = _bones[boneId];

		const std::vector<unsigned short>&	vertexIds		= bone->vertexIds();
		const std::vector<float>&			vertexWeights	= bone->vertexWeights();

		for (unsigned int i = 0; i < vertexIds.size(); ++i)
			if (vertexWeights[i] > 0.0f)
			{
				const unsigned short	vId		= vertexIds[i];
#ifdef DEBUG_SKINNING
				assert(vId < numVertices);
#endif // DEBUG_SKINNING

				const unsigned int		j		= _numVertexBones[vId];
	
				++_numVertexBones[vId];
	
				const unsigned int		index	= vertexArraysIndex(vId, j);
				
				_vertexBones[index]				= boneId;
				_vertexBoneWeights[index]		= vertexWeights[i];
			}
	}

	_maxNumVertexBones = 0;
	for (unsigned int vId = 0; vId < numVertices; ++vId)
		_maxNumVertexBones = std::max(_maxNumVertexBones, _numVertexBones[vId]);

	return shared_from_this();
}

unsigned short
Skin::lastVertexId() const
{
	unsigned short lastId = 0;

	for (unsigned int boneId = 0; boneId < _bones.size(); ++boneId)
	{
		const std::vector<unsigned short>& vertexId = _bones[boneId]->vertexIds();

		for (unsigned int i = 0; i < vertexId.size(); ++i)
			lastId = std::max(lastId, vertexId[i]);
	}

	return lastId;
}

unsigned int
Skin::getFrameId(float time) const
{
	if (_duration < 1e-6f)
		return 0;
	
	const float	t = fmod(time, _duration);
	
	return (unsigned int)floorf(t * _timeFactor) % numFrames();
}

void
Skin::vertexBoneData(unsigned int	vertexId, 
					 unsigned int	j, 
					 unsigned int&	boneId, 
					 float&			boneWeight) const
{
	const unsigned int index = vertexArraysIndex(vertexId, j);

	boneId		= _vertexBones[index];
	boneWeight	= _vertexBoneWeights[index];
}

unsigned int
Skin::vertexBoneId(unsigned int vertexId, unsigned int j) const
{
	return _vertexBones[vertexArraysIndex(vertexId, j)];
}

float 
Skin::vertexBoneWeight(unsigned int vertexId, unsigned int j) const
{
	return _vertexBoneWeights[vertexArraysIndex(vertexId, j)];
}

Skin::Ptr
Skin::disposeBones()
{
	_bones.clear();
	_bones.shrink_to_fit();

	return shared_from_this();
}