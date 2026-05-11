// io.c
/*
  Polyray � MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the �Software�), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdlib.h>
#include <string.h>

/* CM*/
#define IN_IOC
/* end CM */
#include "io.h"
#include "memory.h"


int status_flag = 1;    /* By default, print all status messages */
int warnings_flag = 1;  /* By default, print all warnings */
int errors_flag = 1;    /* By default, print all errors */
#define EXIT_NOW(val) { SetMessageLog(NULL); free_all_memory(); exit(val); }

#ifdef _WIN32
#define PATH_SEPARATORS ";"
#else
#define PATH_SEPARATORS ":;"
#endif

//static
        FILE *message_log = NULL; // = stderr; /* File to write all messages */


char *CurrentFileName = NULL;
file_table File_List[MAX_FILE_DEPTH];
int File_Name_Depth = 0;

/* To work with LEX & YACC, we need to be able to manipulate "yyin" and
   "yylineno" */
extern FILE *yyin;
extern int yylineno;

FILE *PathFileOpen(const char *environn,const char *str, const char *options)
{
   char *ename, *path; 
   char *envcopy; 
   char filename[512];
   FILE *file;

   if (str == NULL || options == NULL)
        return NULL;

   /* If we don't have an environment string, then
      just use the file name as given. */
   if (environn == NULL)
      return fopen(str, options);
   
   /* If it's in the local directory, then we use that */
   if ((file = fopen(str, options)) != NULL)
      return file;

   /* Check all places in the environment variable */
   ename = getenv(environn);

   /* Make local to prevent overwriting something important */
	
   if (ename != NULL && ename[0]!='\0') { 
      envcopy = (char *)malloc(strlen(ename) + 1);
      if (envcopy == NULL)
         return fopen(str, options);
      strcpy(envcopy, ename);

      /* Step through all the directories listed in the environment
         variable. */
      path=strtok(envcopy, PATH_SEPARATORS);

      while(path != NULL) {
         snprintf(filename, sizeof(filename), "%s/%s", path, str);
            file = fopen(filename, options);
            if (file != NULL) {
                free(envcopy);
                return file;
            }

            snprintf(filename, sizeof(filename), "%s/DAT/%s", path, str);
            file = fopen(filename, options);
            if (file != NULL) {
                free(envcopy);
                return file;
            }

            path=strtok(NULL,PATH_SEPARATORS);
      }
      free(envcopy);
    }


#ifdef _WIN32
    snprintf(filename, sizeof(filename), "c:\\polyray\\dat\\%s", str);
    file = fopen(filename, options);
    if (file != NULL)
        return file;
#endif
      
   /* Desperate last try in the current directory */
   return fopen(str, options);
}

void
SetMessageLog(char *str)
{
   if (message_log != NULL &&
       message_log != stderr)
      fclose(message_log);
   if (str == NULL)
      message_log = stderr;
   else {
      if ((message_log = fopen(str, "w")) == NULL)
         error("Cannot open %s\n", str);
      }
}

void
SetInputFile(char *str)
{
	#ifdef DEBUG_FN_CALLS
	printf("SetInputFile %s\n",str);
	#endif
   yylineno = 1;
   if (str == NULL) {
      yyin = stdin;
      CurrentFileName = "<stdin>";
      }
   else {
      CurrentFileName = str;
      if ((yyin = fopen(str, "r")) == NULL)
         error("Can't open file '%s'\n", str);
      }
   File_List[0].file = yyin;
   File_List[0].name = str;
   File_List[0].line = 0;
   File_Name_Depth = 1;
   #ifdef DEBUG_FN_CALLS
   printf("File_Name_Depth=%d for name %s\n",File_Name_Depth,str);
   #endif
}

int yywrap()
{
	return 0;
   #ifdef DEBUG_FN_CALLS
	printf("yywrap()\n");
	printf("file name depth=%d\n",File_Name_Depth);
	//polyray_pause();
	//return 0;
	exit(0);
   #endif
   if (File_Name_Depth > 0) {
      File_Name_Depth--;
      if (File_List[File_Name_Depth].file != stdin &&
          fclose(File_List[File_Name_Depth].file) != 0)
         error("Failed to close file '%s'\n",
               File_List[File_Name_Depth].name);
      if (File_Name_Depth == 0)
         return 1;
      free(File_List[File_Name_Depth].name);
      yyin            = File_List[File_Name_Depth-1].file;
      CurrentFileName = File_List[File_Name_Depth-1].name;
      yylineno        = File_List[File_Name_Depth].line;
      return 0;
      }
   else {
      yyin = stdin;
      return 1;
   }
}

extern void Beginagain();
void include_file_action(char *nviron, char *name)
{
	}

void message(char *fmt, ...)
{
     /* CM added 28/2/2002 - wintarget */
      #if defined (WINTARGET)
        extern char *textonscreen;
        char localtextonscreen[1000];
      #endif
     /* end CM */
   va_list ap;
   if (status_flag) {
      va_start(ap, fmt);

     /* CM added 28/2/2002 - wintarget */
      #if defined (WINTARGET)

	vsnprintf(localtextonscreen, sizeof(localtextonscreen), fmt, ap);
        strcat(textonscreen,localtextonscreen);
        // next enhancement is to split line by line into a new textonscreen array of strings!
      #else  // end CM
      vfprintf(message_log, fmt, ap);
      #endif // closes my new section


      va_end(ap);
      }
}

void
status(char *fmt, ...)
{
   va_list ap;
   if (status_flag) {
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      }
}

void
warning(char *fmt, ...)
{
   va_list ap;

   if (warnings_flag && (message_log!=NULL)) {
      fprintf(message_log, "WARNING: ");
      va_start(ap, fmt);
      vfprintf(message_log, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(message_log, "On or near line %d of file %s\n",
                 yylineno, CurrentFileName);
      }
}

void
error(char *fmt, ...)
{
   va_list ap;

   if (errors_flag && (message_log!=NULL)) {
      fprintf(message_log, "ERROR: ");
      va_start(ap, fmt);
      vfprintf(message_log, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(message_log, " on or near line %d of file %s\n",
                 yylineno, CurrentFileName);
      }
   EXIT_NOW(1);
}

void
fatal(char *fmt, ...)
{
   va_list ap;
   if (errors_flag && (message_log!=NULL)) {
      fprintf(message_log, "FATAL: ");
      va_start(ap, fmt);
      vfprintf(message_log, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(message_log, "On or near line %d of file %s\n",
                 yylineno, CurrentFileName);
      }
   EXIT_NOW(2);
}


