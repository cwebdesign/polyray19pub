#if !defined(__POLYRAY_SYMTAB_DEFS)
#define __POLYRAY_SYMTAB_DEFS


#include "display_backend.h"

#include <string>

#include "defs3.h"
#include "lex.yy.h"//for reflex
#include "unixcompat.h"//for Uint8




bool iequals(const std::string& a, const std::string& b);
int istrcmp(std::string_view str1, const std::string& str2);

extern void Initialize_Symtab(void);
extern void Deallocate_Symtab(int);

extern void Lookup_Definition(const char *, int *, void **);
extern std::string Lookup_String(const char *name);
extern void Insert_Definition(const char *, int, void *, int, int);

extern void reset_subst(SUBST_PTR);

extern ostackptr push_object(ostackptr, Object *);
extern Object *pop_object(ostackptr *);

extern int  LookupColorByName(char *, Vec);


extern void Initialize_BinTree(BinTree& root);
extern void Add_To_BinTree(BinTree& root, Object *obj);
extern void Delete_BinTree(BinTree* root);

// Parse command line/file args
void LoadPolyrayIni(void);

// Global object functions
extern int GenericInitialize(Object *);
extern void GenericCopy(Object *, Object *);
extern void GenericDelete(Object *);
extern void GenericNormal(Object *, Isect *, Vec, Vec, Vec);
extern void GenericRender(Viewpoint *, BinTree *, Object *);
extern void Delete_Object(Object *);
extern void Copy_Object(Object *, Object&);



// Declarations of a whole bunch of global variables. 



extern DrawNode *Draw_Commands;



/* Particle variables */
extern Particle *CurrentParticle;
extern Particle *Particles;
extern ParticleObject *ParticleObjects;



extern Poly_box pbox;
extern Window win;
extern Vec ViewVec;



#endif /* __POLYRAY_SYMTAB_DEFS */
