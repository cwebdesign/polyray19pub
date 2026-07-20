

#if !defined(__POLYRAY_IO_DEFS)
#define __POLYRAY_IO_DEFS

#include <stdio.h>
#include <stdarg.h>

const int MAX_FILE_DEPTH = (8);

#ifdef __cplusplus
#include <string>

FILE *PathFileOpencpp(const std::string environn, std::string sstr, const std::string coptions);
std::ifstream *PathFileOpencppifstr(const std::string environn, std::string sstr, const std::string coptions);
extern void SetMessageLog(std::string);
[[noreturn]] void serror(std::string fmt, ...);
void sstatus(std::string fmt, ...);
void smessage(std::string fmt, ...);
void swarning(std::string fmt, ...);
[[noreturn]] void sfatal(std::string fmt, ...);

FILE* PathFileOpen(char*, std::string, std::string);

//extern "C" {
#endif
/* Central file open routine - this will automatically search
   the directories in the environment variable for places that the
   file might reside */
#ifndef IN_IOC
//extern FILE *PathFileOpen(char *nviron, char *str, char *options);

#endif

#if defined( VISUALC )
#define RTCDECL __cdecl
#else
#define RTCDECL
#endif

typedef struct file_table {
   FILE *file;       /* Handle for this file */
   std::string name;       /* Name of this file */
   int   line;       /* Current line in this file */
   } file_table;
extern std::string CurrentFileName;
extern file_table File_List[MAX_FILE_DEPTH];
extern int File_Name_Depth;

/* Message and Error reporting */
void RTCDECL message(char *fmt, ...);  /* Simply display the message      */
void RTCDECL status(char *fmt, ...);   /* Display message to stderr       */
void RTCDECL warning(char *fmt, ...);  /* Display "WARNING: ..."          */
[[noreturn]] void RTCDECL error(char *fmt, ...);    /* Display "ERROR: ...", then exit */
[[noreturn]] void RTCDECL fatal(char *fmt, ...);    /* Display "FATAL: ...", then exit */

extern void SetInputFile(char *);


//extern int yywrap(void);
extern int status_flag;   /* Set to 0 to eliminate status output */
extern int warnings_flag; /* Set to 0 to eliminate warning output */
extern int errors_flag;   /* Set to 0 to eliminate error output */
extern FILE *message_log; // = stderr; /* File to write all messages */


#ifdef __cplusplus
//}
#endif



#endif /* __POLYRAY_IO_DEFS */

