#pragma once
#include <vector>
#include <string>

#include <d3dx9.h>

#include "Emitter.h"
#include "Particle.h"

// Class declarations
class Particle;
class Emitter;

class ParticleSystemManager
{
private:
	std::vector<Emitter*>emitters;
public:
	ParticleSystemManager();
	~ParticleSystemManager();

	// Init particle system manager
	bool Initialize();

	// Shutdown particle system manager
	void Shutdown();

	// Creates new emitter in the scene
	void CreateEmitter(LPDIRECT3DDEVICE9 pDevice, int numParticles, std::string textureName, D3DXVECTOR3 pos, D3DCOLOR color);

	// Removes emitter from the scene based on its index in the emitter vector
	void RemoveEmitter(int emitterNum);

	// Removes emitter from the scene based on its pointer to the emitter
	void RemoveEmitter(Emitter* emitter);

	// Updates the position of the emitter and particles it contains
	void Update();

	// Renders particles within an emitter
	void Render(LPDIRECT3DDEVICE9 pDevice);
};