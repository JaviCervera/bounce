/*
* Copyright (c) 2016-2019 Irlan Robson https://irlanrobson.github.io
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <bounce/cloth/garment/garment_mesh.h>
#include <bounce/cloth/garment/garment.h>
#include <bounce/cloth/garment/sewing_pattern.h>

#define ANSI_DECLARATORS
#define REAL double
#define VOID void

extern "C"
{
	#include <triangle/triangle.h>
}

b3GarmentMesh::b3GarmentMesh()
{
	meshCount = 0;
	meshes = nullptr;
	sewingCount = 0;
	sewingLines = nullptr;
	garment = nullptr;
}

b3GarmentMesh::~b3GarmentMesh()
{
	for (u32 i = 0; i < meshCount; ++i)
	{
		b3Free(meshes[i].vertices);
		b3Free(meshes[i].triangles);
	}
	
	b3Free(meshes);
	b3Free(sewingLines);
}

// 
static void b3Set(b3SewingPatternMesh* mesh, scalar desiredArea, const b3SewingPattern* pattern)
{
	B3_ASSERT(desiredArea > B3_EPSILON);
	
	struct triangulateio in, mid, out;

	// Prepare the input structure
	in.pointlist = (REAL*)malloc(pattern->vertexCount * 2 * sizeof(REAL));
	const scalar* fp = (scalar*)pattern->vertices;
	for (u32 i = 0; i < 2 * pattern->vertexCount; ++i)
	{
		in.pointlist[i] = (REAL)fp[i];
	}
	in.pointattributelist = nullptr;
	in.pointmarkerlist = (int*)malloc(pattern->vertexCount * sizeof(int));
	for (u32 i = 0; i < pattern->vertexCount; ++i)
	{
		in.pointmarkerlist[i] = 10 + int(i);
	}

	in.numberofpoints = pattern->vertexCount;
	in.numberofpointattributes = 0;

	in.trianglelist = nullptr;
	in.triangleattributelist = nullptr;

	in.trianglearealist = nullptr;

	in.numberoftriangles = 0;
	in.numberofcorners = 0;
	in.numberoftriangleattributes = 0;

	in.segmentlist = nullptr;
	in.segmentmarkerlist = nullptr;
	in.numberofsegments = 0;

	in.holelist = nullptr;
	in.numberofholes = 0;

	in.regionlist = nullptr;
	in.numberofregions = 0;

	// Prepare the middle structure
	mid.pointlist = nullptr;
	mid.pointmarkerlist = nullptr;

	mid.trianglelist = nullptr;
	mid.triangleattributelist = nullptr;
	mid.trianglearealist = nullptr;
	mid.neighborlist = nullptr;

	mid.segmentlist = nullptr;
	mid.segmentmarkerlist = nullptr;

	// Run triangulation
	// Q - quiet
	// z - zero based indices
	// p - PSLG
	// c - preserve the convex hull
	triangulate("Qzpc", &in, &mid, nullptr);

	// Refine

	// Prepare middle structure
	mid.trianglearealist = (REAL*)malloc(mid.numberoftriangles * sizeof(REAL));
	for (int i = 0; i < mid.numberoftriangles; ++i)
	{
		mid.trianglearealist[i] = desiredArea;
	}

	// Prepare output structure
	out.pointlist = nullptr;
	out.pointmarkerlist = nullptr;

	out.trianglelist = nullptr;
	out.trianglearealist = nullptr;

	out.segmentlist = nullptr;
	out.segmentmarkerlist = nullptr;

	// Run triangulation
	// Q - quiet
	// z - zero based indices
	// p - PSLG
	// c - preserve the convex hull
	// r - read triangles
	triangulate("Qzpcra", &mid, &out, nullptr);

	// The first vertices of the output structure must be the vertices of the input structure.
	for (int i = 0; i < in.numberofpoints; ++i)
	{
		B3_ASSERT(in.pointmarkerlist[i] == out.pointmarkerlist[i]);
	}
	
	// Note: comparing doubles
	for (int i = 0; i < in.numberofpoints * 2; ++i)
	{
		B3_ASSERT(in.pointlist[i] == out.pointlist[i]);
	}

	// Convert the output structure 

	// The mesh must be empty
	mesh->vertices = (b3Vec2*)b3Alloc(out.numberofpoints * sizeof(b3Vec2));
	mesh->vertexCount = out.numberofpoints;
	for (int i = 0; i < out.numberofpoints; ++i)
	{
		mesh->vertices[i].x = (scalar)out.pointlist[2 * i + 0];
		mesh->vertices[i].y = (scalar)out.pointlist[2 * i + 1];
	}

	mesh->triangles = (b3SewingPatternMeshTriangle*)b3Alloc(out.numberoftriangles * sizeof(b3SewingPatternMeshTriangle));
	mesh->triangleCount = out.numberoftriangles;
	for (int i = 0; i < out.numberoftriangles; ++i)
	{
		B3_ASSERT(out.numberofcorners == 3);

		b3SewingPatternMeshTriangle triangle;
		triangle.v1 = out.trianglelist[3 * i + 0];
		triangle.v2 = out.trianglelist[3 * i + 1];
		triangle.v3 = out.trianglelist[3 * i + 2];

		mesh->triangles[i] = triangle;
	}

	// Free the input structure
	free(in.pointlist);
	free(in.pointmarkerlist);

	// Free the middle structure
	free(mid.pointlist);
	free(mid.pointmarkerlist);
	free(mid.trianglelist);
	free(mid.triangleattributelist);
	free(mid.trianglearealist);
	free(mid.segmentlist);
	free(mid.segmentmarkerlist);

	// Free the output structure
	free(out.pointlist);
	free(out.pointmarkerlist);
	free(out.trianglelist);
	free(out.segmentlist);
	free(out.segmentmarkerlist);
}

void b3GarmentMesh::Set(b3Garment* g, scalar desiredArea)
{
	garment = g;
	meshCount = garment->patternCount;
	meshes = (b3SewingPatternMesh*)b3Alloc(garment->patternCount * sizeof(b3SewingPatternMesh));
	for (u32 i = 0; i < garment->patternCount; ++i)
	{
		b3Set(meshes + i, desiredArea, garment->patterns[i]);
	}

	// It's okay to do run the following code because
	// the first vertices of a sewing pattern mesh are the vertices of its
	// corresponding sewing pattern. 
	sewingCount = garment->sewingCount;
	sewingLines = (b3GarmentMeshSewingLine*)b3Alloc(garment->sewingCount * sizeof(b3GarmentMeshSewingLine));
	for (u32 i = 0; i < garment->sewingCount; ++i)
	{
		b3SewingLine* sewingLine = garment->sewingLines + i;
		b3GarmentMeshSewingLine* line = sewingLines + i;

		line->s1 = sewingLine->p1;
		line->v1 = sewingLine->v1;
		
		line->s2 = sewingLine->p2;
		line->v2 = sewingLine->v2;
	}
}