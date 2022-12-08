#pragma once
#include <vector>
#include <string>

#include <d3d9.h>
#include <d3dx9.h>

#include "Particle.h"

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// Class declarations
class Particle;

class Emitter
{
private:
	LPDIRECT3DDEVICE9 emitterDevice;

	// Current pos of this particle
	D3DXVECTOR3 curPos;
	// Current velocity of this particle
	D3DXVECTOR3 curVel;

	// Vertex buffer to hold point sprites
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	// Texture that is going to be applied to every particle
	LPDIRECT3DTEXTURE9 texture;
	// Pointer of type Particle; will be used to create particle array
	Particle* particles;
	// Num of particles in this emitter
	int numParticles;
	// Value to hold if the emitter is active
	bool bAlive;

	// Private functions create the vertex buffer to hold the particles
	LPDIRECT3DVERTEXBUFFER9 CreateVertexBuffer(int size, DWORD usage, DWORD fvf);

	struct CUSTOMVERTEX
	{
		D3DXVECTOR3 pos;
		D3DCOLOR color;
	};

public:
	Emitter();
	Emitter(LPDIRECT3DDEVICE9 pDevice);
	~Emitter();

	// Add texture to this emitter
	void AddTexture(std::string textureName);

	// Set the number of particles and size the vector
	void SetNumParticles(int numParticles);

	// Init particles and set the position of the emitter
	void InitializeParticles(D3DXVECTOR3 pos, D3DCOLOR color);

	// Update particles in this emitter
	void Update();

	// Render particles
	void Render();

	inline bool GetAlive() { return bAlive; }
	inline DWORD FloatToDword(float f) { return *((DWORD*)&f); }
};