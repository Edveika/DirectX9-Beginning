#include "Emitter.h"

Emitter::Emitter()
{

}

Emitter::Emitter(LPDIRECT3DDEVICE9 pDevice)
{
}

Emitter::~Emitter()
{

}

// Add texture to this emitter
void Emitter::AddTexture(std::string textureName)
{

}

// Set the number of particles and size the vector
void Emitter::SetNumParticles(int numParticles)
{

}

// Init particles and set the position of the emitter
void Emitter::InitializeParticles(D3DXVECTOR3 pos, D3DCOLOR color)
{
	// Create vertex buffer for this emitter and store it in vertexBuffer variable
	emitterDevice->CreateVertexBuffer
	(
		numParticles * sizeof(CUSTOMVERTEX),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS,
		D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT,
		&vertexBuffer,
		NULL
	);

	// Loop through the particles of this emitter and set the properties
	for (int i = 0; i < numParticles; ++i)
	{
		// The particle is alive
		particles[i].bAlive = true;
		// Set color value passed from particle manager
		particles[i].color = color;
		// Set position value from particle manager
		particles[i].curPos = pos;

		// Create random value for each part of dir/vel vector
		float vecX = ((float)rand() / RAND_MAX);
		float vecY = ((float)rand() / RAND_MAX);
		float vecZ = ((float)rand() / RAND_MAX);

		particles[i].curVel = D3DXVECTOR3(vecX, vecY, vecZ);
	}
}

// Update particles in this emitter
void Emitter::Update()
{
	for (int i = 0; i < numParticles; ++i)
	{
		particles[i].curPos += particles[i].curVel;
	}
}

// Render particles
void Emitter::Render()
{
	CUSTOMVERTEX* pointVertices;

	// Lock the vertex buffer and update the particles within it
	vertexBuffer->Lock
	(
		0,
		numParticles * sizeof(CUSTOMVERTEX),
		(void**)&pointVertices,
		D3DLOCK_DISCARD
	);

	// Go through the buffer and set the properties
	for (int i = 0; i < numParticles; ++i)
	{
		pointVertices->pos = particles[i].curPos;
		pointVertices->color = particles[i].color;
		++pointVertices;
	}

	// Unlock the buffer
	vertexBuffer->Unlock();

	// Set the texture for the particles
	emitterDevice->SetTexture(0, texture);

	// Set the vertex stream
	emitterDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(CUSTOMVERTEX));

	// Set vertex format
	emitterDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// Draw particles to the screen
	emitterDevice->DrawPrimitive(D3DPT_POINTLIST, 0, numParticles);
}

LPDIRECT3DVERTEXBUFFER9 Emitter::CreateVertexBuffer(int size, DWORD usage, DWORD fvf)
{
	LPDIRECT3DVERTEXBUFFER9 buffer;

	emitterDevice->CreateVertexBuffer
	(
		size,
		usage,
		fvf,
		D3DPOOL_DEFAULT,
		&buffer,
		NULL
	);

	if (buffer == NULL)
		return NULL;

	return buffer;
}