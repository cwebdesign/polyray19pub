#if !defined(__POLYRAY_SYMTAB_DEFS)
#define __POLYRAY_SYMTAB_DEFS

extern void Initialize_Symtab(void);
extern void Deallocate_Symtab(int);

extern void Lookup_Definition(char *, int *, void **);
extern char *Lookup_String(char *);
extern void Insert_Definition(char *, int, void *, int, int);

extern void reset_subst(SUBST_PTR);
extern void Initialize_Bean_Counters(void);
extern ostackptr push_object(ostackptr, Object *);
extern Object *pop_object(ostackptr *);

extern int  LookupColorByName(char *, Vec);
extern int  istrcmp(char *str1, char *str2);

extern void Initialize_BinTree(BinTree *root);
extern void Add_To_BinTree(BinTree *root, Object *obj);
extern void Delete_BinTree(BinTree *root);

/* Parse command line/file args */
void Read_Initialization_Data(void);

/* Global object functions */
extern int GenericInitialize(Object *);
extern void GenericCopy(Object *, Object *);
extern void GenericDelete(Object *);
extern void GenericNormal(Object *, Isect *, Vec, Vec, Vec);
extern void GenericRender(Viewpoint *, BinTree *, Object *);
extern void Delete_Object(Object *);
extern void Copy_Object(Object *, Object *);

/* Global image variables */
extern int             buffer_update;
extern unsigned long   buffer_size;

/* Declarations of a whole bunch of global variables. */
extern int             Rendering_Method;
extern unsigned short  Global_Shade_Flag;
extern Viewpoint       Eye;
extern Flt             Global_Haze;
extern Flt             Global_Haze_Start;
extern Vec             Global_Haze_Color;
extern NODE_PTR        Background;
extern Vec             BackgroundColor;
extern Surface         DefaultSurface;
extern BinTree         Root;
extern Flt             rayeps;
extern int             tickflag;
extern int             antialias;
extern char            outfilebase[128];
extern int             filebaseflag;
extern int             File_Generation_Flag;
extern long            MaxBufferRAM;
extern Flt             csg_leg_tolerance;
extern Flt             csg_subdivision_depth;

/* Animation variables */
extern int start_frame;
extern int end_frame;
extern int total_frames;
extern int current_frame;
extern Flt frame_time;

extern DrawNode *Draw_Commands;

/*CM added ifndef IN_SYMTABC */
#ifndef IN_SYMTABC
extern char *POLYRAY_PATH_STRING; // was extern char *POLYRAY_PATH_STRING;
#endif

extern unsigned long nChecked;
extern unsigned long nRays;
extern unsigned long nShadows;
extern unsigned long nReflected;
extern unsigned long nRefracted;
extern unsigned long nTIR;
extern unsigned long nJittered;

/* Rendering quality variables */
extern int pixelsize;
extern int pixel_encoding;
extern int DepthRender;
extern Flt minweight;
extern int maxlevel;
extern int maxsamples;
extern Flt antialias_threshold;
extern int Display_Flag;
extern int Optimizer;

/* Runtime status variables */
extern int Parsed_Flag;
extern int Shadow_Test;
extern int Particle_Test;

/* Particle variables */
extern Particle *CurrentParticle;
extern Particle *Particles;
extern ParticleObject *ParticleObjects;

extern int     Check_Abort_Flag;
extern int     Abort_Flag;
extern jmp_buf abort_environ;

extern Poly_box box;
extern Window win;
extern Vec ViewVec;

extern Light *Lights;
extern int   nLights;

extern unsigned long totalShadows, totalShadowCaches;
extern unsigned long maxQueueSize;
extern unsigned long totalQueues;
extern unsigned long totalQueueResets;
extern unsigned long nEnqueued;

extern int clustersize;

/* Debug/ runtime screen variables */
extern int current_row;
extern int current_col;
extern int recursion_depth;

#endif /* __POLYRAY_SYMTAB_DEFS */

