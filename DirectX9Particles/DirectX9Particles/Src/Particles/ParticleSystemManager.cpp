#include "ParticleSystemManager.h"

ParticleSystemManager::ParticleSystemManager()
{

}
ParticleSystemManager::~ParticleSystemManager()
{

}

// Init particle system manager
bool ParticleSystemManager::Initialize()
{

}

// Shutdown particle system manager
void ParticleSystemManager::Shutdown()
{

}

// Creates new emitter in the scene
void ParticleSystemManager::CreateEmitter(LPDIRECT3DDEVICE9 pDevice, int numParticles, std::string textureName, D3DXVECTOR3 pos, D3DCOLOR color)
{
	// Create new emitter
	Emitter* tempEmitter = new Emitter(pDevice);

	// Load the texture
	tempEmitter->AddTexture(textureName);

	// Set the number of particles
	tempEmitter->SetNumParticles(numParticles);

	// Initialize the particles
	tempEmitter->InitializeParticles(pos, color);

	// Add this created emitter to the emitters vector(dynamic array of emitters)
	emitters.push_back(tempEmitter);
}

// Removes emitter from the scene based on its index in the emitter vector
void ParticleSystemManager::RemoveEmitter(int emitterNum)
{

}

// Removes emitter from the scene based on its pointer to the emitter
void ParticleSystemManager::RemoveEmitter(Emitter* emitter)
{

}

// Updates the position of the emitter and particles it contains
void ParticleSystemManager::Update()
{
	for (int i = 0; i < emitters.size(); ++i)
	{
		// Check if emitter is active
		if (emitters[i]->GetAlive())
			emitters[i]->Update();
	}
}

// Renders particles within an emitter
void ParticleSystemManager::Render(LPDIRECT3DDEVICE9 pDevice)
{
	for (int i = 0; i < emitters.size(); ++i)
	{
		if (emitters[i]->GetAlive())
			emitters[i]->Render();
	}
}