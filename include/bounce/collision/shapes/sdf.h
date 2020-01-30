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

#ifndef B3_SDF_H
#define B3_SDF_H

#include <bounce/collision/shapes/aabb.h>

struct b3MultiIndex
{
	unsigned int& operator[](unsigned int i) 
	{
		return v[i];
	}
	
	const unsigned int& operator[](unsigned int i) const
	{
		return v[i];
	}

	unsigned int v[3];
};

struct b3Cell32
{
	unsigned int v[32];
};

// This class stores a discretized signed distance function (SDF) 
// generated by Discregrid.
// Discregrid is available at https://github.com/InteractiveComputerGraphics/Discregrid
// Inside Discregrid, there is a tool called GenerateSDF that can 
// generate an SDF of a triangle mesh stored in .obj file format.
// You may call this tool from a command line. 
// For example, the following command will generate an SDF for a given .obj mesh.
// GenerateSDF -r "32 32 32" -d "-5 -5 -5 5 5 5" teapot.obj
// The parameters are:
// 1. r - resolution
// 2. d - domain (an AABB)
// 3. input filename
// You will need to set a reasonable large domain depending on the radius of 
// the vertices that can collide against the SDF because the SDF 
// can only return valid output values for points that are inside the domain.
// Therefore, its a good idea to set the domain to the AABB containing the 
// associated object extended by twice the largest vertex radius that can collide against this SDF.
// Generally, the greater the SDF resolution the more accurate is the result of the signed distance function.
class b3SDF
{
public:
	// Construct this SDF
	b3SDF();

	// Destruct this SDF
	~b3SDF();

	// Read this SDF from a .cdf (binary) file given the filename.
	// Returns true if this SDF was loaded sucessfuly and false otherwise.
	bool Load(const char* filename);

	// Return the domain (AABB) of this SDF.
	const b3AABB& GetDomain() const
	{
		return m_domain;
	}

	// Evaluate the signed distance function for a point if the point is inside the domain of this SDF.
	// Optionally output the normal at the SDF boundary.
	// Return true if the output values are valid and false otherwise.
	bool Evaluate(const b3Vec3& point, double& distance, b3Vec3* normal = nullptr) const
	{
		return interpolate(0, distance, point, normal);
	}
private:
	bool interpolate(unsigned int field_id, double& dist, b3Vec3 const& x, b3Vec3* gradient = nullptr) const;
	
	b3MultiIndex singleToMultiIndex(unsigned int i) const;
	unsigned int multiToSingleIndex(b3MultiIndex const& ijk) const;

	b3AABB subdomain(b3MultiIndex const& ijk) const;
	b3AABB subdomain(unsigned int l) const;

	b3AABB m_domain;
	unsigned int m_resolution[3];
	b3Vec3 m_cell_size;
	b3Vec3 m_inv_cell_size;
	std::size_t m_n_cells;
	std::size_t m_n_fields;

	struct b3SDFNodeArray
	{
		double& operator[](u32 i)
		{
			B3_ASSERT(i < count);
			return values[i];
		}
		
		const double& operator[](u32 i) const
		{
			B3_ASSERT(i < count);
			return values[i];
		}
		
		u32 count;
		double* values;
	};

	struct b3SDFCellArray
	{
		b3Cell32& operator[](u32 i)
		{
			B3_ASSERT(i < count);
			return values[i];
		}
		
		const b3Cell32& operator[](u32 i) const
		{
			B3_ASSERT(i < count);
			return values[i];
		}
		
		u32 count;
		b3Cell32* values;
	};
	
	struct b3SDFCellMapArray
	{
		unsigned int& operator[](u32 i)
		{
			B3_ASSERT(i < count);
			return values[i];
		}

		const unsigned int& operator[](u32 i) const
		{
			B3_ASSERT(i < count);
			return values[i];
		}
		
		u32 count;
		unsigned int* values;
	};
	
	u32 m_nodeCount;
	b3SDFNodeArray* m_nodes;
	
	u32 m_cellCount;
	b3SDFCellArray* m_cells;
	
	u32 m_cellMapCount;
	b3SDFCellMapArray* m_cell_map;
};

#endif