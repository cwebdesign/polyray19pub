#pragma once

#if !defined(__POLYRAY_IO_DEFS)
#define __POLYRAY_IO_DEFS

#include <stdio.h>
#include <stdarg.h>

const int MAX_FILE_DEPTH = (8);

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



typedef struct file_table {
   FILE *file;       /* Handle for this file */
   std::string name;       /* Name of this file */
   int   line;       /* Current line in this file */
   } file_table;
extern std::string CurrentFileName;
extern file_table File_List[MAX_FILE_DEPTH];
extern int File_Name_Depth;

/* Message and Error reporting */
void message(char *fmt, ...);  /* Simply display the message      */
void status(char *fmt, ...);   /* Display message to stderr       */
void  warning(char *fmt, ...);  /* Display "WARNING: ..."          */
[[noreturn]] void error(char *fmt, ...);    /* Display "ERROR: ...", then exit */
[[noreturn]] void fatal(char *fmt, ...);    /* Display "FATAL: ...", then exit */

void SetInputFile(char *);


//extern int yywrap(void);
extern FILE *message_log; // = stderr; /* File to write all messages */



#endif /* __POLYRAY_IO_DEFS */

