
#if !defined(__POLYRAY_WAVEFRONT_DEFS)
#define __POLYRAY_WAVEFRONT_DEFS


#include <fstream>      // std::ifstream
extern int Process_Obj_File(Object *, std::ifstream *);
#ifdef TESTING
void read_vertex(const std::string& token, long& v, long& vt, long& vn);
#endif


#if !defined(__POLYRAY_RAW_DEFS)
#include "raw.h"
#endif


#endif /* __POLYRAY_WAVEFRONT_DEFS */


