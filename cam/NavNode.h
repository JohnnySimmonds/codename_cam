#pragma once

#include <vector>
#include "assimp\scene.h"

class NavNode
{
public:
	NavNode(float x, float y, const aiFace* aFace, const aiVector3D* verts);
	~NavNode();
	std::vector<NavNode*> neighbours;
	float centerX;
	float centerZ;
	const aiFace* mFace;
	const aiVector3D* mVerts;
	std::vector<uint32_t> vertIndices;

	void addNeighbour(NavNode* neighbour);
	bool isInside(float xPos, float zPos);
	float distance(NavNode* aNode);
private:

};
