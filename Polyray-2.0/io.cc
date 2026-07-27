/*io.cc
  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <cstdlib>
#include <cstring>
#include <cstdio>//for snprintf

#include <fstream>      // std::ifstream
#include <iostream>
#include <string>
#include <filesystem>

/* CM*/
#define IN_IOC
/* end CM */
#include "io_ply.h"
#include "memory.h"
#include "runtime_state.h"
#include "util.h" //for getenvbyplatform


int status_flag = 1;    /* By default, print all status messages */
int warnings_flag = 1;  /* By default, print all warnings */
int errors_flag = 1;    /* By default, print all errors */
#define EXIT_NOW(val) { SetMessageLog(""); free_all_memory(); exit(val); }

#ifdef _WIN32
#define PATH_SEPARATORS ";"
#else
#define PATH_SEPARATORS ":;"
#endif

//static
        FILE *message_log = nullptr; // = stderr; /* File to write all messages */


std::string CurrentFileName{ "" };
file_table File_List[MAX_FILE_DEPTH];
int File_Name_Depth = 0;

/* To work with LEX & YACC, we need to be able to manipulate "yyin" and
   "yylineno" */
extern FILE* yyin;
extern int yylineno;

//! Open File with Environment Path Search
/*!
      Attempts to open a file by searching through environment paths.
      \param environn Environment variable name to search for paths (can be NULL)
      \param str Filename or relative path to open
      \param soptions File open mode ("r", "w", etc.)
      \return FILE pointer if successful, NULL otherwise
      \note Also checks c:\\polyray\\dat\\ directory on Windows
*/

FILE *PathFileOpen(char *environn, std::string str, std::string soptions)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"io::PathFileOpen\n";
   #endif
   char* options = (char*)soptions.c_str();
   char* cstr = (char*)str.c_str();
   char *path;
   FILE *file;

   if (environn == nullptr)
     return fopen(cstr, options);

#ifdef DEBUG_FN_CALLS
   std::cout << "io::PathFileOpen checking environment\n";
#endif
   if ((file = fopen(cstr, options)) != nullptr)
         return file;

   size_t requiredSize=255;

   char* ename = getenvbyplatform(requiredSize, environn);



   if (ename != nullptr) {
      for (path = strtok(ename, PATH_SEPARATORS);
           path != nullptr;
           path = strtok(nullptr, PATH_SEPARATORS)) {
         //sprintf(filename, "%s/%s", path, cstr);
         std::string filename = (std::filesystem::path(path) / cstr).string();

         if ((file = fopen(filename.c_str(), options)) != nullptr)
            return file;

         //sprintf(filename, "%s/DAT/%s", path, cstr);//pass the cstring
         filename = (std::filesystem::path(path) / "DAT" / cstr).string();

         if ((file = fopen(filename.c_str(), options)) != nullptr)
            return file;
         }
   }//if ename

   std::string spat = "c:\\polyray\\dat\\" + str;
   if ((file = fopen((char*)spat.c_str(), options)) != nullptr)
      return file;


   return fopen(cstr, options);
}

//! Open File with Environment Path Search (C++ ifstream Version)
/*!
      C++ version that returns an ifstream pointer for file operations.
      \param environn Environment variable name to search for paths
      \param sstr Filename or relative path to open
      \param coptions File open mode string (not actively used)
      \return Pointer to ifstream object, caller must deallocate
      \note Also checks c:\\polyray\\dat\\ directory on Windows
*/
//}//extern

std::ifstream *PathFileOpencppifstr(const std::string environn, std::string sstr, const std::string coptions)
{
   

   const char *options=(const char*)coptions.c_str();
   //const char *str=(const char*)sstr.c_str();
   std::ifstream *file = new std::ifstream;   
   if (environn == "") {
   #ifdef DEBUG
      std::cout<<"open options are "<<coptions<<"\n";
   #endif
      serror("io:::PathFileOpencpp-options");
      file->open(sstr);
      return file;
   }

   file->open(sstr);
   if (*file) {      
         return file;
   }

   size_t requiredSize = 255;

   char* ename = getenvbyplatform(requiredSize, (char*)environn.c_str());

   if (ename != nullptr) {
      for (char* path = strtok(ename, " ;");
           path != nullptr;
           path = strtok(nullptr, " ;")) {
         std::string filename = (std::filesystem::path(path) / sstr).string();
         file->open(filename);//todo:add options
         if (*file)
            return file;
         }
   }//if ename

   std::string spat="c:\\polyray\\dat\\";
   spat += sstr;
   file->open(spat);
   if (*file)
      return file;

   file->open(sstr);
   return file;
   //todo:add proper options
}

//! Open File with Environment Path Search (FILE Version - Deprecated)
/*!
      FILE version of path-based file opening. This function is deprecated.
      \param environn Environment variable name to search for paths
      \param sstr Filename or relative path to open
      \param coptions File open mode ("r", "w", etc.)
      \return FILE pointer if successful, NULL otherwise
      \note Will be deprecated in favor of C++ stream versions
*/
//FILE version, will be deprecated
FILE *PathFileOpencpp(const std::string environn, std::string sstr, const std::string coptions)
{
   char *path;
   FILE *file;

   const char *options=(const char*)coptions.c_str();
   const char *str=(const char*)sstr.c_str();
   if (environn == "")
     return fopen(str, options);


   if ((file = fopen(str, options)) != nullptr)
         return file;

   char *ename=NULL;
   #if defined(_MSC_VER)
   size_t len;
   errno_t err = _dupenv_s( &ename, &len, (const char*)environn.c_str());
   #endif
   #if defined(__MINGW32__) || defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__unix)
   size_t requiredSize=255;

#if !defined(__APPLE__) && !defined(__linux__) && !defined(__MINGW32__) && !defined(__sun) && !defined(__sun__) && !defined(__illumos__)
   getenv_s( &requiredSize, NULL, 0, environn);
   if (requiredSize == 0)
   {

         ename=(char*)malloc(1);
         ename[0]='\0';
   }
   else {
#endif
      //ename = (char*) malloc(requiredSize * sizeof(char));
      //if (!ename)
      //{
      //   error("Failed to allocate memory!\n\n");
      //   exit(1);
      //}
#if defined(__APPLE__) || defined(__linux__) || defined(__MINGW32__)||defined(__sun) || defined(__sun__) || defined(__illumos__)
      ename=getenv((const char*) environn.c_str());
      #else
      getenv_s( &requiredSize, ename, requiredSize, environn );
      #endif
#if !defined(__APPLE__) && !defined(__linux__) && !defined(__MINGW32__) && !defined(__sun) && !defined(__sun__) && !defined(__illumos__)
   }
#endif //APPLE/linux

   #endif


   if (ename != nullptr) {
      for (path = strtok(ename, " ;");
           path != nullptr;
           path = strtok(nullptr, " ;")) {
         //sprintf(filename, "%s/%s", path, str);
         std::string filename = (std::filesystem::path(path) / str).string();
         
         if ((file = fopen(filename.c_str(), options)) != nullptr)
            return file;
         }
   }//if ename

   std::string spat = "c:\\polyray\\dat\\";
   spat += sstr;
   if ((file = fopen(spat.c_str(), options)) != nullptr)
      return file;


   return fopen(str, options);
}

//! Set Message Log File
/*!
      Redirects all messages to a specified file instead of stderr.
      \param str Output filename (empty string redirects to stderr)
      \return void
*/
void
SetMessageLog(std::string str)
{
   if (message_log != nullptr &&
       message_log != stderr)
      fclose(message_log);
   if (str == "")
      message_log = stderr;
   else {
      if ((message_log = fopen((const char*)str.c_str(), "w")) == nullptr)
         serror("Cannot open %s\n", str.c_str());
      }
}

//! Set Input File for Parsing
/*!
      Sets the input file for the lexer/parser, or stdin if NULL.
      \param str Filename to parse (NULL for stdin)
      \return void
*/
void SetInputFile(char *str)
{
	#ifdef DEBUG_FN_CALLS
	std::cout<<"io::SetInputFile "<<str<<"\n";
	#endif
   yylineno = 1;
   std::string sstr{str};
   if (str == nullptr) {
      yyin = stdin;
      CurrentFileName = (char*)"<stdin>";
      }
   else {
      CurrentFileName = sstr;
      if ((yyin = fopen(str, "r")) == nullptr)
         serror("Can't open file '%s'\n", str);
      }
   File_List[0].file = yyin;
   File_List[0].name = sstr;
   File_List[0].line = 0;
   File_Name_Depth = 1;
   #ifdef DEBUG_FN_CALLS
   std::cout<<"(last)File_Name_Depth="<<File_Name_Depth<<" for name"<<str<<"\n";
   #endif
}

//! Lexer Wrap Function
/*!
      Required by LEX - called at end of file to continue lexing.
      \return Always returns 0 to allow continued lexing
*/
static int yywrap(void)
{
	return 0;
}

extern void Beginagain();

//! Print Status Message
/*!
      Prints a formatted status message to the message log or stderr if status_flag is set.
      \param fmt Format string (printf-style)
      \param ... Variable arguments matching format string
      \return void
*/
void message(char *fmt, ...)
{
   va_list ap;
   if (RuntimeState::status_flag) {
      va_start(ap, fmt);
      FILE* out = (message_log != nullptr) ? message_log : stderr;
      vfprintf(out, fmt, ap);
      va_end(ap);
      }
}

//! Print Status Message (String Version)
/*!
      C++ string version of message() for formatted status output.
      \param fmt Format string (printf-style, as std::string)
      \param ... Variable arguments matching format string
      \return void
*/
void smessage(std::string fmt, ...)
{
   va_list ap;
   if (RuntimeState::status_flag) {
      const char* cfmt = fmt.c_str();
      FILE* out = (message_log != NULL) ? message_log : stderr;
	  va_start(ap, fmt);
	  vfprintf(out, cfmt, ap);
     va_end(ap);
   }
}

//! Print Status Output
/*!
      Prints formatted status output directly to stderr if status_flag is set.
      \param fmt Format string (printf-style)
      \param ... Variable arguments matching format string
      \return void
*/
void status(char *fmt, ...)
{
   va_list ap;
   if (RuntimeState::status_flag) {
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      }
}

//! Print Status Output (String Version)
/*!
      C++ string version of status() for direct stderr output.
      \param fmt Format string (printf-style, as std::string)
      \param ... Variable arguments matching format string
      \return void
*/
void sstatus(std::string fmt, ...)
{
   va_list ap;
   if (RuntimeState::status_flag) {
      const char* cfmt=fmt.c_str();
	  va_start(ap, fmt);
	  vfprintf(stderr, cfmt, ap);
	  va_end(ap);
	  }
}

//! Print Warning Message
/*!
      Prints a formatted warning message to the message log with file and line info if warnings_flag is set.
      \param fmt Format string (printf-style)
      \param ... Variable arguments matching format string
      \return void
*/
void warning(char *fmt, ...)
{
   va_list ap;

   if (warnings_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
      fprintf(out, "%s", "WARNING: ");
      va_start(ap, fmt);
      vfprintf(out, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(out, (char*)"On or near line %d of file %s\n",
                 yylineno, CurrentFileName.c_str());
      }
}

//! Print Warning Message (String Version)
/*!
      C++ string version of warning() for formatted warning output with context information.
      \param fmt Format string (printf-style, as std::string)
      \param ... Variable arguments matching format string
      \return void
*/
void swarning(std::string fmt, ...)
{
   va_list ap;

   if (warnings_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
	  fprintf(out, "%s", (char*)"WARNING: ");
      const char* cfmt = fmt.c_str();
      va_start(ap, fmt);
	  vfprintf(out, cfmt, ap);
	  va_end(ap);
	  if (File_Name_Depth)
		 fprintf(out, (char*)"On or near line %d of file %s\n",
				 yylineno, CurrentFileName.c_str());
	  }
}

//! Print Error Message and Exit
/*!
      Prints a formatted error message and exits the program with code 1 if errors_flag is set.
      \param fmt Format string (printf-style)
      \param ... Variable arguments matching format string
      \return void (never returns - calls EXIT_NOW(1))
*/
[[noreturn]] void error(char *fmt, ...)
{
   va_list ap;

   if (errors_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
      fprintf(out, "%s", "ERROR: ");
      va_start(ap, fmt);
      vfprintf(out, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(out, " on or near line %d of file %s\n",
                 yylineno, CurrentFileName.c_str());
      }
   EXIT_NOW(1);
}

//! Print Error Message and Exit (String Version)
/*!
      C++ string version of error() for formatted error output with context information.
      \param fmt Format string (printf-style, as std::string)
      \param ... Variable arguments matching format string
      \return void (never returns - calls EXIT_NOW(1))
*/
[[noreturn]] void serror(std::string fmt, ...)
{
   va_list ap;

   if (errors_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
      fprintf(out, "%s",  "ERROR: ");
      const char* cfmt = fmt.c_str();
      va_start(ap, fmt);
      vfprintf(out, cfmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(out, " on or near line %d of file %s\n",
                 yylineno, CurrentFileName.c_str());
      }
   EXIT_NOW(1);
}

//! Print Fatal Error Message and Exit
/*!
      Prints a formatted fatal error message and exits the program with code 2 if errors_flag is set.
      \param fmt Format string (printf-style)
      \param ... Variable arguments matching format string
      \return void (never returns - calls EXIT_NOW(2))
*/
[[noreturn]] void fatal(char *fmt, ...)
{
   va_list ap;
   if (errors_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
      fprintf(out, "%s","FATAL: ");
      va_start(ap, fmt);
      vfprintf(out, fmt, ap);
      va_end(ap);
      if (File_Name_Depth)
         fprintf(out, "On or near line %d of file %s\n",
                 yylineno, CurrentFileName.c_str());
      }
   EXIT_NOW(2);
}

//! Print Fatal Error Message and Exit (String Version)
/*!
      C++ string version of fatal() for formatted fatal error output with context information.
      \param fmt Format string (printf-style, as std::string)
      \param ... Variable arguments matching format string
      \return void (never returns - calls EXIT_NOW(2))
*/
[[noreturn]] void sfatal(std::string fmt, ...)
{
   va_list ap;
   if (errors_flag) {
      FILE* out = (message_log != nullptr) ? message_log : stderr;
	  fprintf(out, "%s", "FATAL: ");
	  const char* cfmt = fmt.c_str();
	  va_start(ap, fmt);
	  vfprintf(out, cfmt, ap);
	  va_end(ap);
	  if (File_Name_Depth)
		 fprintf(out, "On or near line %d of file %s\n",
				 yylineno, CurrentFileName.c_str());
	  }
   EXIT_NOW(2);
}
