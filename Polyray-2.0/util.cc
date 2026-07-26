/** @file util.cc
 *  @brief General-purpose utility functions for the Polyray renderer.
 *
 *  Covers numeric <-> string conversions, case folding, temporary-file creation,
 *  quadratic solving, 3x3 matrix inversion, and cross-platform environment
 *  variable access.  All functions are free (non-member) unless noted.
 *
 *  (C) C. Meli 2006-2026
 */
#include <string>
#include <ostream>
#include <iostream>
#include <algorithm>
#include <charconv> //for to_chars
#include <array>
#include <sstream>
#include <cmath>

#ifdef _MSC_VER
#include <io.h> //_mktemp_s - VS
#endif


#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "util.h"

/** @brief Round a double to the nearest integer using half-up rounding.
 *
 *  Equivalent to @c (int)(a + 0.5).  Negative values are not rounded
 *  symmetrically - use std::lround() if that matters.
 *  @param a  Value to round.
 *  @return   Nearest integer (biased toward positive infinity on 0.5).
 */
int polyray_round(double a) {
  return int(a + 0.5);
  }

/** @brief Convert an int to its decimal string representation.
 *
 *  Uses @c std::to_chars into a 10-char stack buffer (sufficient for any
 *  32-bit integer including the sign).  No heap allocation occurs.
 *  @param i  Integer to convert.
 *  @return   Decimal string of @p i.
 */
  std::string int2string(int i)
  	{

  		std::array<char, 10> s = {};
  		std::to_chars(s.data(), s.data() + s.size(), i);
  		return std::string(s.data());//since s is null terminated
  	}

/** @brief Convert a long to its decimal string representation.
 *
 *  Uses @c std::stringstream.  Any stream exception is caught and reported
 *  to @c std::cout, returning an empty string in that case.
 *  @param l  Value to convert.
 *  @return   Decimal string of @p l, or @c "" on stream error.
 */
  	std::string long2string(long l)
  	{
  		std::stringstream ss;
  		std::string s;
  		try {
  			ss << l;
  			s = ss.str();
  		}
  		catch (const int ex)
  		{
  			std::cout << "Exception occured in Util: value: " << ex << std::endl;
  		}

  		return s;
  	}

/** @brief Convert an unsigned long to its decimal string representation.
 *
 *  Uses @c std::stringstream.  Any stream exception is caught and reported
 *  to @c std::cout, returning an empty string in that case.
 *  @param ul  Value to convert.
 *  @return    Decimal string of @p ul, or @c "" on stream error.
 */
  	std::string ulong2string(unsigned long ul)
  	{
  		std::stringstream ss;
  		std::string s;
  		try {
  			ss << ul;
  			s = ss.str();
  		}
  		catch (const int ex)
  		{
  			std::cout << "Exception occured in Util: value: " << ex << std::endl;
  		}

  		return s;
  	}

/** @brief Convert a double to string via stringstream (legacy, superseded).
 *
 *  @deprecated Replaced by double2string() which uses @c std::to_string.
 *              Kept for reference; not exposed in util.h.
 *  @param d  Value to convert.
 *  @return   Default-precision decimal string of @p d, or @c "" on stream error.
 */
    static std::string double2stringold(double d)
    {
        std::stringstream ss;
        std::string s;
        try {
            ss << d;
            s = ss.str();
        }
        catch (const int ex)
        {
            std::cout << "Exception occured in Util: value: " << ex << std::endl;
        }

        return s;
    }

/** @brief Convert a double to its decimal string representation.
 *
 *  Delegates to @c std::to_string, which produces six decimal places.
 *  @param d  Value to convert.
 *  @return   Six-decimal-place string of @p d (e.g. "3.140000").
 */
    std::string double2string(double d)
    {
        return std::to_string(d);
    }

/** @brief Parse a decimal string into an int.
 *
 *  Uses @c std::stoi with base 10.  Returns @c false (leaving @p result
 *  unmodified on failure) if the string is not a pure integer literal or if
 *  the value is out of @c int range.  Trailing non-digit characters also
 *  cause a @c false return.
 *  @param str     Input string to parse.
 *  @param result  Output: parsed integer value on success.
 *  @return        @c true if the entire string was consumed as a valid int;
 *                 @c false on invalid input or overflow.
 */
    bool String2Int(const std::string& str, int& result)
    {
        try
        {
            std::size_t lastChar;
            result = std::stoi(str, &lastChar, 10);
            return lastChar == str.size();
        }
        catch (const std::invalid_argument&)
        {
            return false;
        }
        catch (const std::out_of_range&)
        {
            return false;
        }
    }

/** @brief Parse a decimal string into a double.
 *
 *  Uses @c std::stod.  Returns @c false if any trailing non-numeric characters
 *  remain after conversion, or on @c std::invalid_argument / @c std::out_of_range.
 *  @param str     Input string to parse.
 *  @param result  Output: parsed double value on success.
 *  @return        @c true if the entire string was consumed as a valid double;
 *                 @c false on invalid input or overflow.
 */
    bool String2Double(const std::string& str, double& result)
    {
        try
        {
            std::size_t lastChar;
            result = std::stod(str, &lastChar);
            return lastChar == str.size();
        }
        catch (std::invalid_argument&)
        {
            return false;
        }
        catch (std::out_of_range&)
        {
            return false;
        }
    }

/** @brief Convert a narrow (UTF-8/ASCII) string to a wide string.
 *
 *  Simple range constructor - only safe for pure ASCII content. Does not
 *  perform any locale-aware multi-byte decoding.
 *  @param ws  Output: wide-character string populated from @p s.
 *  @param s   Input narrow string to convert.
 *  @return    Always 0.
 */
    int StringToWString(std::wstring& ws, const std::string& s)
    {
        std::wstring wsTmp(s.begin(), s.end());

        ws = wsTmp;

        return 0;
    }


/** @brief Convert an ASCII character to uppercase without locale overhead.
 *
 *  Operates purely on the range @c 'a'-@c 'z'; all other characters are
 *  returned unchanged.  @c constexpr so it can be used in compile-time contexts
 *  and as a predicate for @c std::transform without a virtual dispatch cost.
 *  @param ch  Character to convert.
 *  @return    Uppercase equivalent if @p ch is a lowercase ASCII letter;
 *             @p ch unchanged otherwise.
 */
	constexpr char polyraytoupper(char ch) {
    if (ch >= 'a' && ch <= 'z')
        return ch - 32;
    return ch;
    }
/** @brief Convert an ASCII character to lowercase without locale overhead.
 *
 *  Operates purely on the range @c 'A'-@c 'Z'; all other characters are
 *  returned unchanged.
 *  @param ch  Character to convert.
 *  @return    Lowercase equivalent if @p ch is an uppercase ASCII letter;
 *             @p ch unchanged otherwise.
 */
    constexpr char polyraytolower(char ch) {
	  if (ch >= 'A' && ch <= 'Z')
		  return ch + 32;
	  return ch;
    }


/** @brief Return a copy of @p textv with all ASCII letters uppercased.
 *
 *  Applies polyraytoupper() to every character via @c std::transform.
 *  Empty strings are returned as-is without calling transform.
 *  @param textv  Input string (not modified).
 *  @return       A new string with lowercase ASCII letters replaced by their
 *                uppercase equivalents; all other characters unchanged.
 */
    std::string ToUpperString(const std::string& textv)
    {
	  std::string text{textv};
	  if (text != "") std::transform(text.begin(), text.end(), text.begin(), polyraytoupper); //::toupper takes 0.56, toupper takes 0.26, faster

	  return text; // return in upper case
    }

/** @brief Manual smoke-test for ToUpperString() - prints results to stdout.
 *
 *  Converts a hard-coded lowercase string two ways (raw @c std::transform
 *  and ToUpperString()) and prints both results to @c std::cout for visual
 *  comparison.  Not called from production code.
 */
	void touppertester(void)
    {
      char olds[]="this is an old string";
      std::string text(olds);
      if (text!="") std::transform(text.begin(), text.end(), text.begin(), polyraytoupper); //::toupper
       std::string text2(olds);
	   text2 = ToUpperString(text2);
	   std::cout<<"s1="<<text<<"\ns2="<<text2<<"\n";
	}


/** @brief Portable wrapper that generates a unique temporary filename.
 *
 *  Calls @c std::tmpnam to obtain a unique name, creates the file with
 *  @c std::fopen to verify it is writable, then immediately removes it and
 *  returns the name in @p filename.  The caller is responsible for creating
 *  the file again before use (inherent TOCTOU race).
 *  @param filename  Output: unique temporary filename on success.
 *  @return          0 on success; -1 if the file could not be opened.
 */
    int mkstemp(std::string& filename) //portable version
    {
        char* tempFilename = std::tmpnam(nullptr);
        std::FILE* file = std::fopen(tempFilename, "w");
        if (file == nullptr)
        {
            return -1; // Error occurred
        }
        std::fclose(file);
        filename = tempFilename;
        std::remove(tempFilename); // Remove the temporary file
        return 0; // Success
    }



/** @brief Solve the quadratic equation ax^2 + bx + c = 0 for real roots.
 *
 *  Computes the discriminant b^2-4ac. If it is negative the equation has no
 *  real solutions and the function returns @c false with @p t0 and @p t1 set
 *  to zero.  Otherwise both roots are returned in ascending order
 *  (@p t0 <= @p t1).
 *  @param a   Coefficient of x^2.
 *  @param b   Coefficient of x.
 *  @param c   Constant term.
 *  @param t0  Output: smaller root on success; 0 on failure.
 *  @param t1  Output: larger root on success; 0 on failure.
 *  @return    @c true if two real roots exist (discriminant >= 0); @c false otherwise.
 */
bool solveQuadratic(double a, double b, double c, double& t0, double& t1) {
    double discriminant = b * b - 4 * a * c;

    //std::cout<<"Discriminant="<<discriminant<<"\n";
    t0=0;t1=0;
    // Check if the equation has no real roots
    if (discriminant < 0) {
        return false;
    }

    // Calculate the two roots of the quadratic equation
    double root1 = (-b + std::sqrt(discriminant)) / (2 * a);
    double root2 = (-b - std::sqrt(discriminant)) / (2 * a);

    // Assign the roots to t0 and t1
    if (root1 > root2) {
        std::swap(root1, root2);
    }
    t0 = root1;
    t1 = root2;

    return true;
}


/** @brief Compute the inverse of a 3x3 matrix via the adjugate method.
 *
 *  Computes the cofactor transpose into @c RetIM::out, then divides by the
 *  determinant.  The determinant is computed using Sarrus' rule.
 *  @param in   Input matrix as three row vectors @p in[0..2].
 *  @return     @c RetIM where @c ret.i is 1 on success or 0 if the matrix is
 *              singular (|det| < PLY_EPSILON); @c ret.out always contains the
 *              scaled cofactor matrix (may be garbage on singular input).
 */
RetIM InvertMatrix(fVec in[3])
{
   Flt det;
   RetIM ret;
   ret.out[0][0] =  (in[1][1] * in[2][2] - in[1][2] * in[2][1]);
   ret.out[1][0] = -(in[0][1] * in[2][2] - in[0][2] * in[2][1]);
   ret.out[2][0] =  (in[0][1] * in[1][2] - in[0][2] * in[1][1]);

   ret.out[0][1] = -(in[1][0] * in[2][2] - in[1][2] * in[2][0]);
   ret.out[1][1] =  (in[0][0] * in[2][2] - in[0][2] * in[2][0]);
   ret.out[2][1] = -(in[0][0] * in[1][2] - in[0][2] * in[1][0]);

   ret.out[0][2] =  (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
   ret.out[1][2] = -(in[0][0] * in[2][1] - in[0][1] * in[2][0]);
   ret.out[2][2] =  (in[0][0] * in[1][1] - in[0][1] * in[1][0]);
   
   det = in[0][0] * in[1][1] * in[2][2] +
         in[0][1] * in[1][2] * in[2][0] +
         in[0][2] * in[1][0] * in[2][1] -
         in[0][2] * in[1][1] * in[2][0] -
         in[0][0] * in[1][2] * in[2][1] -
         in[0][1] * in[1][0] * in[2][2];

   if (std::fabs(det) < PLY_EPSILON) {
	  ret.i=0;
      return ret;
   }

   det = 1 / det;

   for (int i=0;i<3;i++)
      for (int j=0;j<3;j++)
         ret.out[i][j] *= det;
   ret.i=1;
   return ret;
}

/** @brief Compute the inverse of a 3x3 matrix via the adjugate method.
 *
 *  Computes the cofactor transpose into @c RetIM::out, then divides by the
 *  determinant.  The determinant is computed using Sarrus' rule.
 *  @param in   Input matrix as three row vectors @p in[0..2].
 *  @return     @c RetIM where @c ret.i is 1 on success or 0 if the matrix is
 *              singular (|det| < PLY_EPSILON); @c ret.out always contains the
 *              scaled cofactor matrix (may be garbage on singular input).
 */
RetIM InvertMatrix(NuVec in[3])
{
    Flt det;
    RetIM ret;
    ret.out[0][0] = (in[1][1] * in[2][2] - in[1][2] * in[2][1]);
    ret.out[1][0] = -(in[0][1] * in[2][2] - in[0][2] * in[2][1]);
    ret.out[2][0] = (in[0][1] * in[1][2] - in[0][2] * in[1][1]);

    ret.out[0][1] = -(in[1][0] * in[2][2] - in[1][2] * in[2][0]);
    ret.out[1][1] = (in[0][0] * in[2][2] - in[0][2] * in[2][0]);
    ret.out[2][1] = -(in[0][0] * in[1][2] - in[0][2] * in[1][0]);

    ret.out[0][2] = (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
    ret.out[1][2] = -(in[0][0] * in[2][1] - in[0][1] * in[2][0]);
    ret.out[2][2] = (in[0][0] * in[1][1] - in[0][1] * in[1][0]);

    det = in[0][0] * in[1][1] * in[2][2] +
        in[0][1] * in[1][2] * in[2][0] +
        in[0][2] * in[1][0] * in[2][1] -
        in[0][2] * in[1][1] * in[2][0] -
        in[0][0] * in[1][2] * in[2][1] -
        in[0][1] * in[1][0] * in[2][2];

    if (std::fabs(det) < PLY_EPSILON) {
        ret.i = 0;
        return ret;
    }

    det = 1 / det;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            ret.out[i][j] *= det;
    ret.i = 1;
    return ret;
}
/** @brief Read an environment variable and return its value as a string.
 *
 *  On MSVC uses @c _dupenv_s to avoid the deprecation warning on @c getenv.
 *  On all other platforms uses @c std::getenv directly.
 *  @param key  Name of the environment variable.
 *  @return     Value of the variable, or an empty string if it is not set.
 */
std::string getEnvVar(std::string const& key)
{
    size_t len;
    char* val;
	#if defined(_MSC_VER) 
    auto err = _dupenv_s(&val,&len,key.c_str());;
    #else
    //#if __APPLE__
    val = std::getenv(key.c_str());
	#endif

    return val == nullptr ? std::string("") : std::string(val);
}

/** @brief Platform-specific environment variable lookup returning a C string.
 *
 *  Dispatches to the appropriate runtime call for each toolchain:
 *  - MSVC: @c _dupenv_s (heap-allocates; caller must @c free the result).
 *  - MinGW / non-Apple Unix: @c getenv_s (writes into a caller-allocated buffer).
 *  - Apple / Linux: @c getenv (returns a pointer into the process environment;
 *    do @b not free the result on these platforms).
 *
 *  @param requiredSize  On MinGW/non-Apple Unix: receives the size needed for
 *                       the value buffer; unused on other platforms.
 *  @param environn      Name of the environment variable (null-terminated).
 *  @return              Pointer to the variable's value, or a heap-allocated
 *                       empty string if not found (MSVC/MinGW).  Ownership
 *                       rules differ by platform - see note above.
 */
char* getenvbyplatform(size_t& requiredSize, char* environn)
{
    char* ename = nullptr;
#if defined(_MSC_VER)
    size_t len;
    errno_t err = _dupenv_s(&ename, &len, environn);
#endif
#if defined(__MINGW32__) || defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__unix)||defined(__sun) || defined(__sun__) || defined(__illumos__)
#if !defined(__APPLE__) && !defined(__linux__) && !defined(__illumos__)
    getenv_s(&requiredSize, NULL, 0, environn);
    if (requiredSize == 0)
    {

        ename = (char*)malloc(1);
        ename[0] = '\0';
    }
    else {
#endif
#if defined(__APPLE__) || defined(__linux__) || defined(__illumos__)
        ename = getenv(environn);
#else
        ename = (char*)malloc(requiredSize * sizeof(char));
        if (!ename)
        {
            serror("Failed to allocate memory!\n\n");
            exit(1);
        }
        getenv_s(&requiredSize, ename, requiredSize, environn);
#endif
#if !defined(__APPLE__) && !defined(__linux__) && !defined(__illumos__)
    }
#endif //APPLE/linux

#endif
    return ename;
}

//todo:portable setEnvVar


/** @brief Duplicate a @c std::string into a @c polyray_malloc-owned C string.
 *
 *  Allocates @c length+1 bytes via @c polyray_malloc and copies the string
 *  content including the null terminator.  The caller is responsible for
 *  freeing the result with @c polyray_free.
 *  @param strtmp  Source string to duplicate.
 *  @return        Heap-allocated null-terminated copy of @p strtmp, or
 *                 @c nullptr if allocation fails (after calling serror()).
 */
char* myCopy(std::string strtmp)
{
    char* temp = static_cast<char*>(polyray_malloc(strtmp.length() + 1)); // +1 for '\0'
    if (temp == nullptr) {
        serror("Failed to allocate memory in myCopy\n");
        return nullptr;
    }
    strcpy(temp, strtmp.c_str());
    return temp;
}

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _kbhit()

using UWORD32 = uint32_t;//not a type
using UWORD64 = uint64_t;
#else
#if __APPLE__
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


//unsigned long _wait=0;
using UWORD32 = uint32_t;//not a type
using UWORD64 = uint64_t;


/** @brief Non-blocking keyboard-hit test (macOS implementation).
 *  @return 1 if a key is waiting in stdin, 0 otherwise.
 */
int _kbhit()
{
  struct termios oldt, newt;
  int oldf;


  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);


  int ch = getchar();


  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);


  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }


  return 0;
}

#else
//linux kbhit from  https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
//added the includes
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
/** @brief Non-blocking keyboard-hit test (Linux implementation).
 *  @return Number of bytes waiting in stdin (non-zero means a key is ready).
 *
 *  Not marked inline: must have external linkage so other TUs can call it
 *  (declaration in util.h). An inline definition in this .cc would leave
 *  unresolved references at link time on Linux.
 */
int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (!initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#endif
#endif

/** @brief Block until the user presses Enter (cross-platform pause helper). */
void defs3_hitanykey()
{
    //slog(std::make_tuple("Press ENTER to continue\n");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //polyray::pause();//cm160519
}
/*CM end */
