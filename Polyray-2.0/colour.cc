/* colour.cc

   Provide named colors to polyray
  Polyray - MIT Licensed Revival  
  Copyright (C) 2006-2026, Clyde Meli, All rights reserved.

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
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <gsl/gsl>

#include "defs3.h"
//#include "symtab.h"
#include "colour.h"

constexpr int NCOLORS=  (71+19+5);//add CM
constexpr int NCOLORS2= (71+19+5);//end CM 


/*
 * Note: These colors must be in sorted order, because we binary search
 * for them.
 *
 * They were swiped from the X-11 distribution.  Sorry....
 */

/* add CM*/
#define GREYTRIPLE(p)  ((p),(p),(p))
struct CaseInsensitiveLess
{
    bool operator()( std::string const& lhs,  std::string const& rhs)  const
    {
        std::string l=lhs;
        std::string r=rhs;
        std::transform(l.begin(), l.end(), l.begin(),
                       [](char c) { return std::tolower(c); });
        std::transform(r.begin(), r.end(), r.begin(),
                       [](char c) { return std::tolower(c); });
        return l < r;
    }
};

std::map<std::string, NuVec, CaseInsensitiveLess> Colours1 {
        {"Aquamarine", {.439216, .858824, .576471}},
        {"Black", {0, 0, 0}},
        {"Blue", {0, 0, 1}},
        {"BlueViolet", {.623529, .372549, .623529}},
        {"Brown", {.647059, .164706, .164706}},
        {"CadetBlue", {.372549, .623529, .623529}},
        {"Coral", {1, .498039, 0}},
        {"CornflowerBlue", {.258824, .258824, .435294}},
        {"Cyan", {0, 1, 1}},
        {"DarkGreen", {.184314, .309804, .184314}},
        {"DarkOliveGreen", {.309804, .309804, .184314}},
        {"DarkOrchid", {.6, .196078, .8}},
        {"DarkSlateBlue", {.419608, .137255, .556863}},
        {"DarkSlateGray", {.184314, .309804, .309804}},
        {"DarkSlateGrey", {.184314, .309804, .309804}},
        {"DarkTurquoise", {.439216, .576471, .858824}},
        {"DimGray", {.329412, .329412, .329412}},
        {"DimGrey", {.329412, .329412, .329412}},
        {"Firebrick", {.556863, .137255, .137255}},
        {"ForestGreen", {.137255, .556863, .137255}},
        {"Gold", {.8, .498039, .196078}},
        {"Goldenrod", {.858824, .858824, .439216}},
        {"Gray", {.752941, .752941, .752941}},
        /* CM ADD*/
        {"Gray05", {0.05,0.05,0.05}} ,
/* following were GREYTRIPLE(1*0.10) etc pairs */
        {"Gray10", {.10,.10,.10}},/* first param is 1*itself (was 0*0.10) */
        {"Gray15", {.15,.15,.15}},
        {"Gray20", {.20,.20,.20}},
        {"Gray25", {.25,.25,.25}},
        {"Gray30", {.30,.30,.30}},
        {"Gray35", {.35,.35,.35}},
        {"Gray40", {.40,.40,40}},
        {"Gray45", {.45,.45,.45}},
        {"Gray50", {.50,.50,.50}},
        {"Gray55", {.55,.55,.55}},
        {"Gray60", {.60,.60,.60}},
        {"Gray65", {.65,.65,.65}},
        {"Gray70", {.70,.70,.70}},
        {"Gray75", {.75,.75,.75}},
        {"Gray80", {.80,.80,.80}},
        {"Gray85", {.85,.85,.85}},
        {"Gray90", {.90,.90,.90}},
        {"Gray95", {.95,.95,.95}},
        /* CM ADD */

        {"Green", {0, 1, 0}},
        {"GreenYellow", {.576471, .858824, .439216}},
        {"Grey", {.752941, .752941, .752941}},
        {"IndianRed", {.309804, .184314, .184314}},
        {"Khaki", {.623529, .623529, .372549}},
        {"LightBlue", {.74902, .847059, .847059}},
        {"LightGray", {.658824, .658824, .658824}},
        {"LightGrey", {.658824, .658824, .658824}},
        {"LightSteelBlue", {.560784, .560784, .737255}},
        {"LimeGreen", {.196078, .8, .196078}},
        {"Magenta", {1, 0, 1}},
        {"Maroon", {.556863, .137255, .419608}},
        {"MediumAquamarine", {.196078, .8, .6}},
        {"MediumBlue", {.196078, .196078, .8}},
        {"MediumForestGreen", {.419608, .556863, .137255}},
        {"MediumGoldenrod", {.917647, .917647, .678431}},
        {"MediumOrchid", {.576471, .439216, .858824}},
        {"MediumSeaGreen", {.258824, .435294, .258824}},
        {"MediumSlateBlue", {.498039, 0, 1}},
        {"MediumSpringGreen", {.498039, 1, 0}},
        {"MediumTurquoise", {.439216, .858824, .858824}},
        {"MediumVioletRed", {.858824, .439216, .576471}},
        {"MidnightBlue", {.184314, .184314, .309804}},
        {"Navy", {.137255, .137255, .556863}},
        {"NavyBlue", {.137255, .137255, .556863}},
        {"Orange", {.8, .196078, .196078}},
        {"OrangeRed", {1, 0, .498039}},
        {"Orchid", {.858824, .439216, .858824}},
        {"PaleGreen", {.560784, .737255, .560784}},
        {"Pink", {.737255, .560784, .560784}},
        {"Plum", {.917647, .678431, .917647}},
        {"Red", {1, 0, 0}},
        {"Salmon", {.435294, .258824, .258824}},
        {"SeaGreen", {.137255, .556863, .419608}},
        {"Sienna", {.556863, .419608, .137255}},
        {"SkyBlue", {.196078, .6, .8}},

/* CM ADD */
        {"SkyBlue1", {0.5273, 0.8047,0.9180}},
        {"SkyBlue2", {0.5273, 0.8047,1.0000}},
        {"SkyBlue3", {0.4922, 0.7500,0.9297}},
        {"SkyBlue4", {0.4219, 0.6484,0.8008}},
        {"SkyBlue5", {0.2891, 0.4375,0.5430}},
/* CM END */


        {"SlateBlue", {0, .498039, 1}},
        {"SpringGreen", {0, 1, .498039}},
        {"SteelBlue", {.137255, .419608, .556863}},
        {"Tan", {.858824, .576471, .439216}},
        {"Thistle", {.847059, .74902, .847059}},
        {"Turquoise", {.678431, .917647, .917647}},
        {"Violet", {.309804, .184314, .309804}},
        {"VioletRed", {.8, .196078, .6}},
        {"Wheat", {.847059, .847059, .74902}},
        {"White", {1, 1, 1}},
        {"Yellow", {1, 1, 0}},
        {"YellowGreen", {.6, .8, .196078}}
         };

std::map<std::string, NuVec, CaseInsensitiveLess> Colours2 {
//std::less<>> Colours2{
        {"aquamarine", {.439216, .858824, .576471}},
        {"black", {0, 0, 0}},
        {"blue", {0, 0, 1}},
        {"blue_violet", {.623529, .372549, .623529}},
        {"brown", {.647059, .164706, .164706}},
        {"cadet_blue", {.372549, .623529, .623529}},
        {"coral", {1, .498039, 0}},
        {"cornflower_blue", {.258824, .258824, .435294}},
        {"cyan", {0, 1, 1}},
        {"dark_green", {.184314, .309804, .184314}},
        {"dark_olive_green", {.309804, .309804, .184314}},
        {"dark_orchid", {.6, .196078, .8}},
        {"dark_slate_blue", {.419608, .137255, .556863}},
        {"dark_slate_gray", {.184314, .309804, .309804}},
        {"dark_slate_grey", {.184314, .309804, .309804}},
        {"dark_turquoise", {.439216, .576471, .858824}},
        {"dim_gray", {.329412, .329412, .329412}},
        {"dim_grey", {.329412, .329412, .329412}},
        {"firebrick", {.556863, .137255, .137255}},
        {"forest_green", {.137255, .556863, .137255}},
        {"gold", {.8, .498039, .196078}},
        {"goldenrod", {.858824, .858824, .439216}},
        {"gray", {.752941, .752941, .752941}},
        {"gray05", {0.05,0.05,0.05}} ,
 //       {"gray10", {0, 0, 0}},
 // was greytriple(1*0.10) - see below setupcolor2
        {"gray10", {.10,.10,.10}},/* first param is 1*itself (was 0*0.10) */
        {"gray15", {.15,.15,.15}},
        {"gray20", {.20,.20,.20}},
        {"gray25", {.25,.25,.25}},
        {"gray30", {.30,.30,.30}},
        {"gray35", {.35,.35,.35}},
        {"gray40", {.40,.40,40}},
        {"gray45", {.45,.45,.45}},
        {"gray50", {.50,.50,.50}},
        {"gray55", {.55,.55,.55}},
        {"gray60", {.60,.60,.60}},
        {"gray65", {.65,.65,.65}},
        {"gray70", {.70,.70,.70}},
        {"gray75", {.75,.75,.75}},
        {"gray80", {.80,.80,.80}},
        {"gray85", {.85,.85,.85}},
        {"gray90", {.90,.90,.90}},
        {"gray95", {.95,.95,.95}},

        {"green", {0, 1, 0}},
        {"green_yellow", {.576471, .858824, .439216}},
        {"grey", {.752941, .752941, .752941}},
        {"indian_red", {.309804, .184314, .184314}},
        {"khaki", {.623529, .623529, .372549}},
        {"light_blue", {.74902, .847059, .847059}},
        {"light_gray", {.658824, .658824, .658824}},
        {"light_grey", {.658824, .658824, .658824}},
        {"light_steel_blue", {.560784, .560784, .737255}},
        {"lime_green", {.196078, .8, .196078}},
        {"magenta", {1, 0, 1}},
        {"maroon", {.556863, .137255, .419608}},
        {"medium_aquamarine", {.196078, .8, .6}},
        {"medium_blue", {.196078, .196078, .8}},
        {"medium_forest_green", {.419608, .556863, .137255}},
        {"medium_goldenrod", {.917647, .917647, .678431}},
        {"medium_orchid", {.576471, .439216, .858824}},
        {"medium_sea_green", {.258824, .435294, .258824}},
        {"medium_slate_blue", {.498039, 0, 1}},
        {"medium_spring_green", {.498039, 1, 0}},
        {"medium_turquoise", {.439216, .858824, .858824}},
        {"medium_violet_red", {.858824, .439216, .576471}},
        {"midnight_blue", {.184314, .184314, .309804}},
        {"navy", {.137255, .137255, .556863}},
        {"navy_blue", {.137255, .137255, .556863}},
        {"orange", {.8, .196078, .196078}},
        {"orange_red", {1, 0, .498039}},
        {"orchid", {.858824, .439216, .858824}},
        {"pale_green", {.560784, .737255, .560784}},
        {"pink", {.737255, .560784, .560784}},
        {"plum", {.917647, .678431, .917647}},
        {"red", {1, 0, 0}},
        {"salmon", {.435294, .258824, .258824}},
        {"sea_green", {.137255, .556863, .419608}},
        {"sienna", {.556863, .419608, .137255}},
        {"sky_blue", {.196078, .6, .8}},

/* CM ADD */
        {"skyBlue1", {0.5273, 0.8047,0.9180}},
        {"skyBlue2", {0.5273, 0.8047,1.0000}},
        {"skyBlue3", {0.4922, 0.7500,0.9297}},
        {"skyBlue4", {0.4219, 0.6484,0.8008}},
        {"skyBlue5", {0.2891, 0.4375,0.5430}},
/* CM END */


        {"slate_blue", {0, .498039, 1}},
        {"spring_green", {0, 1, .498039}},
        {"steel_blue", {.137255, .419608, .556863}},
        {"tan", {.858824, .576471, .439216}},
        {"thistle", {.847059, .74902, .847059}},
        {"turquoise", {.678431, .917647, .917647}},
        {"violet", {.309804, .184314, .309804}},
        {"violet_red", {.8, .196078, .6}},
        {"wheat", {.847059, .847059, .74902}},
        {"white", {1, 1, 1}},
        {"yellow", {1, 1, 0}},
        {"yellow_green", {.6, .8, .196078}}
   };



/// @brief 
/// @param name Name to search
/// @param colour Colour to be returned
/// @return 1 if found, 0 if not. //todo:change to bool
int SearchColorByName(std::string_view name, NuVec& colour)
{
    #ifdef DEBUG_FN_CALLS
    std::cout<<"colours::SearchColorByName("<<name<<"\n";
    #endif
    auto res = Colours1.find((std::string)name);
    if (res != Colours1.end()) //found
    {
        colour = res->second;        
        return 1;
    }
    res = Colours2.find(std::string(name));
    if (res != Colours2.end()) //found
    {
        colour = res->second;        
        return 1;
    }    
    return 0;
}



