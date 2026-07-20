// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
//util.h
#if !defined(__POLYRAY_UTIL)
#define __POLYRAY_UTIL

#include <string>

#include "defs3.h"

 int polyray_round(double a);
 std::string int2string(int i);
 std::string long2string(long l);
 std::string ulong2string(unsigned long ul);

 std::string double2string(double d);
 bool String2Int(const std::string& str, int& result);
 bool String2Double(const std::string& str, double& result);
 int StringToWString(std::wstring& ws, const std::string& s);
 std::string ToUpperString(const std::string& textv);

void touppertester(void);
int mkstemp(std::string& filename);

bool solveQuadratic(double a, double b, double c, double& t0, double& t1);
extern int InvertMatrix(fVec in[3], NuMatrix& out);
//extern void ConvertToMatrix(NuMatrix in, fVec out[3]);
struct RetIM
{
	int i;
	fVec out[3];
};
RetIM InvertMatrix(fVec in[3]);
RetIM InvertMatrix(NuVec in[3]);
std::string getEnvVar(std::string const& key);
char* getenvbyplatform(size_t& requiredSize, char* environn);
char* myCopy(std::string strtmp);

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _kbhit()
#else
int _kbhit();
#endif
void defs3_hitanykey();

#endif //POLYRAY_UTIL
