//getopt.cc
/*
Polyray - MIT Licensed Revival
Copyright(C) 1993 - 1996, Alexander Enzmann, All rights reserved.
Copyright(C) 2024 - 2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <string>
#include <stdexcept>
#include <iostream>

#include "getopt.h"
#include "io_ply.h"

    struct res_updatesp {
        std::string place;
        int ret{0};
        bool incoptind{false};
    };

    auto updatescanningpointer_Lambda = [](const int argc, const char** argv, const int _optind,
        const std::string place) -> res_updatesp {
#ifdef DEBUG_FN_CALLS
            std::cout << "getopt::updatescanningpointer_Lambda\n";
#endif
            res_updatesp res = { place, 0, false };
#ifdef DEBUG
            std::cout << "lambda. optind=" << _optind << " argc=" << argc << "\n";
#endif
            if (place.empty()) {  // update scanning pointer
                if (_optind >= argc || (res.place = argv[_optind])[0] != '-' || res.place.empty()) {
#ifdef DEBUG
                    std::cout << "returning res1\n";
                    std::cout << "place is /" << res.place << "/\n";
#endif
                    res.ret = EOF;
                    return res;
                }
                if (res.place.length() > 1 && res.place[1] == '-') {  // found "--"
#ifdef DEBUG
                    std::cout << "returning res2\n";
#endif
                    res.ret = EOF;
                    res.incoptind = true;
                    return res;
                }
                res.place.erase(0, 1); // skip '-'
            }
            return res;
        };


    int GetOpt::getopt(const int argc, const char** argv, const std::string& ostr) const {
        #ifdef DEBUG_FN_CALLS
        std::cout<<"getopt::getopt\n";
        #endif
        static std::string place = EMESG;  // option letter processing
        std::string::size_type oli;        // option letter list index

        auto ret=updatescanningpointer_Lambda(argc,argv,_optind,place);
        if (ret.incoptind) _optind++;
        place=ret.place;
        if (ret.ret==EOF) return EOF;
        

        // check for a good letter option
        char optopt = place[0];
        #ifdef DEBUG
        std::cout<<"optopt="<<optopt<<"\n";
        #endif
        place.erase(0, 1);
        if (optopt == ':' || optopt == '+' || (oli = ostr.find(optopt)) == std::string::npos) {
            if (place.empty()) ++_optind;
            serror(": illegal option -- '" + std::string(1, optopt) + "'");
            return 0;
        }
        #ifdef DEBUG
        std::cout<<"legal option\n";
        #endif
        if (ostr[oli + 1] == ':') {
            // need an argument
            #ifdef DEBUG
            std::cout<<"need an argument\n";
            #endif
            if (!place.empty()) {
                optarg1 = place;
            } else if (argc <= ++_optind) { // no arg
            #ifdef DEBUG
            std::cout<<"no arg\n";
            #endif
                place = EMESG;
                serror(": option requires an argument -- '" + std::string(1, optopt) + "'");
                return 0;
            } else {
                #ifdef DEBUG
                std::cout<<"else part\n";
                #endif
                optarg1 = argv[_optind];
                #ifdef DEBUG
                std::cout<<"argument1="<<optarg1<<"\n";
                #endif
            }
            place = EMESG;
            ++_optind;
            #ifdef DEBUG
            std::cout<<"optind now "<<_optind<<"\n";
            #endif
        } else if (ostr[oli + 1] == '+') {
            // handle case for two arguments
            // similar to the above case, but requires two arguments
            #ifdef DEBUG
            std::cout<<"Case two args\n";
            #endif
            optarg1 = argv[++_optind];
            optarg2 = argv[++_optind];
            #ifdef DEBUG
            std::cout<<"arg 1="<<optarg1<<"\n";
            std::cout<<"arg 2="<<optarg2<<"\n";
            #endif
        } else {
            // don't need argument
            //std::cout<<"no arg needed\n";
            optarg1.clear();
            if (place.empty()) ++_optind;
        }
        return optopt;
    }

