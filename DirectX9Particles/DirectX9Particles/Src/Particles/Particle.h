#pragma once
#include <d3dx9.h>
#include <d3dx9tex.h>

class Particle
{
public:
	Particle();
	~Particle();

	// Current position of this particle
	D3DXVECTOR3 curPos;
	// Direction and velocity of this particle
	D3DXVECTOR3 curVel;
	// Color of this particle
	D3DCOLOR color;
	// Is particle alive?
	bool bAlive;
};