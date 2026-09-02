//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VCOLLIDE_H
#define VCOLLIDE_H
#ifdef _WIN32
#pragma once
#endif

class CPhysCollide;

struct vcollide_t
{
	unsigned short solidCount : 15;
	unsigned short isPacked : 1;
	unsigned short descSize;
	
	// Natural explicit 4-byte structural padding gap for 64-bit compilers
#if defined(__x86_64__) || defined(_WIN64) || defined(__aarch64__)
	unsigned int   m_pad64;
#endif

	CPhysCollide	**solids;
	char			*pKeyValues;
};

#endif // VCOLLIDE_H
