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

//static
        FILE *message_log = NULL; // = stderr; /* File to write all messages */


char *CurrentFileName = NULL;
file_table File_List[MAX_FILE_DEPTH];
int File_Name_Depth = 0;

/* To work with LEX & YACC, we need to be able to manipulate "yyin" and
   "yylineno" */
extern FILE *yyin;
/*CM ??? #define yyin stdin */
extern int yylineno;

FILE *PathFileOpen(char *environn, char *str, char *options)
{
   char *path, tenviron[256], filename[256];
   FILE *file;

   if (environn == NULL) 
     return fopen(str, options);

   if ((file = fopen(str, options)) != NULL)
         return file;
   
   char *ename=NULL;
   #if defined(_MSC_VER)
   size_t len;
   errno_t err = _dupenv_s( &ename, &len, environn);
   #endif
   #if defined(__MINGW32__) || defined(__LINUX__) || defined(__APPLE__) || defined(__unix__) || defined(__unix)
   size_t requiredSize=255;

#if !defined(__APPLE__)
   getenv_s( &requiredSize, NULL, 0, environn);
   if (requiredSize == 0)
   {

         ename=(char*)malloc(1);
         ename[0]='\0';
   }
   else {
#endif
      
      #if defined(__APPLE__)
      ename=getenv(environn);
      #else
      ename = (char*) malloc(requiredSize * sizeof(char));
      if (!ename)
      {
         error("Failed to allocate memory!\n\n");
         exit(1);
      }
      getenv_s( &requiredSize, ename, requiredSize, environn );
      #endif
#if !defined(__APPLE__)
   }
#endif //APPLE

   #endif


   if (ename != NULL) {
      for (path = strtok(ename, ";");
           path != NULL;
           path = strtok(NULL, ";")) {
         sprintf(filename, "%s/%s", path, str);

         if ((file = fopen(filename, options)) != NULL)
            return file;
         
         sprintf(filename, "%s/DAT/%s", path, str);

         if ((file = fopen(filename, options)) != NULL)
            return file;
         }
   }

   char spat[250];
   strcpy(spat,"c:\\polyray\\dat\\");
   strcat(spat,str);
   if ((file = fopen(spat, options)) != NULL)
      return file;


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

        vsprintf(localtextonscreen, fmt, ap);
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

   if (warnings_flag) {
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

   if (errors_flag) {
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
   if (errors_flag) {
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


