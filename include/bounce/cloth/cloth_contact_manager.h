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

#ifndef B3_CLOTH_CONTACT_MANAGER_H
#define B3_CLOTH_CONTACT_MANAGER_H

#include <bounce/cloth/contacts/cloth_sphere_triangle_contact.h>
#include <bounce/cloth/contacts/cloth_sphere_shape_contact.h>
#include <bounce/cloth/contacts/cloth_capsule_capsule_contact.h>
#include <bounce/collision/broad_phase.h>
#include <bounce/common/memory/block_pool.h>
#include <bounce/common/template/list.h>

class b3Cloth;

// Contact delegator for b3Cloth.
class b3ClothContactManager
{
public:
	b3ClothContactManager();

	void FindNewContacts();
	void AddPair(void* data1, void* data2);
	void UpdateContacts();

	b3ClothSphereAndTriangleContact* CreateSphereAndTriangleContact();
	void Destroy(b3ClothSphereAndTriangleContact* c);

	b3ClothSphereAndShapeContact* CreateSphereAndShapeContact();
	void Destroy(b3ClothSphereAndShapeContact* c);

	b3ClothCapsuleAndCapsuleContact* CreateCapsuleAndCapsuleContact();
	void Destroy(b3ClothCapsuleAndCapsuleContact* c);
	
	b3BlockPool m_sphereAndTriangleContactBlocks;
	b3BlockPool m_sphereAndShapeContactBlocks;
	b3BlockPool m_capsuleAndCapsuleContactBlocks;

	b3Cloth* m_cloth;
	b3BroadPhase m_broadPhase;
	b3List2<b3ClothSphereAndTriangleContact> m_sphereAndTriangleContactList;
	b3List2<b3ClothSphereAndShapeContact> m_sphereAndShapeContactList;
	b3List2<b3ClothCapsuleAndCapsuleContact> m_capsuleAndCapsuleContactList;
};

#endif