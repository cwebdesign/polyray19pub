#if !defined(__POLYRAY_PARTICLE_DEFS)
#define __POLYRAY_PARTICLE_DEFS

Particle *CreateParticle(void);
void DeleteParticle(Particle *particle);
Particle *CopyParticle(char *name);
void InsertParticle(Particle *particle);
void SetParticleP(Particle *particle, NODE_PTR P);
void SetParticleV(Particle *particle, NODE_PTR V);
void SetParticleA(Particle *particle, NODE_PTR A);
void SetParticleAvoid(Particle *particle, NODE_PTR Avoid);
void SetParticleFlock(Particle *particle, NODE_PTR Flock);
void SetParticleBirth(Particle *particle, NODE_PTR Birth);
void SetParticleDeath(Particle *particle, NODE_PTR Death);
void SetParticleCount(Particle *particle, NODE_PTR Count);
void SetParticleObjName(Particle *particle, NODE_PTR obj_name);
void InstantiateParticles(Viewpoint *Eye);
void FreeParticles(void);

#endif /* __POLYRAY_PARTICLE_DEFS */
