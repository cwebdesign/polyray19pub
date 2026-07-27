// define surface characteristics this file includes simple matte and
// reflective surface characteristics for some common colors.
// Polyray input file - Alexander Enzmann

// We only need to define these things on the very first frame.
if (frame == start_frame) {
// First define some standard surface characteristics, matte, shiny (plastic),
// reflective.
static define matte
surface {
   ambient 0.1
   diffuse 0.7
   }

static define shiny
surface {
   ambient 0.1
   diffuse 0.6
   specular white, 0.6
   microfacet Phong 7
   }

static define reflective
surface {
   ambient 0.1
   diffuse 0.5
   specular white, 0.5
   microfacet Phong 7
   reflection white, 0.5
   }

static define metallic
surface {
   ambient 0.1
   diffuse 0.5
   brilliance 6
   specular 0.6
   microfacet Phong 5
   reflection white, 0.25
   }

// Now build textures using different colors for the surfaces above

// First the matte colors.  Diffuse reflections only, no shiny spot
static define matte_white texture { matte { color white } }
static define matte_grey texture { matte { color grey } }
static define matte_gray70 texture { matte { color gray70 } }
static define matte_black texture { matte { color dark_slate_gray } }
static define matte_red texture { matte { color red } }
static define matte_blue texture { matte { color blue } }
static define matte_green texture { matte { color green } }
static define matte_cyan texture { matte { color cyan } }
static define matte_yellow texture { matte { color yellow } }
static define matte_magenta texture { matte { color magenta } }
static define matte_orange texture { matte { color orange } }
static define matte_aquamarine texture { matte { color aquamarine } }
static define matte_brown texture { matte { color brown } }

// Now the standard plastic coloration, diffuse color plus shiny white spot
static define shiny_red texture { shiny { color red } }
static define shiny_green texture { shiny { color green } }
static define shiny_blue texture { shiny { color blue } }
static define shiny_cyan texture { shiny { color cyan } }
static define shiny_yellow texture { shiny { color yellow } }
static define shiny_magenta texture { shiny { color magenta } }
static define shiny_orange texture { shiny { color orange } }
static define shiny_coral texture { shiny { color coral } }
static define steely_blue texture { shiny { color steel_blue } }

// A set of surfaces that combine color with reflectivity
static define reflective_white texture { reflective { color white } }
static define reflective_grey texture { reflective { color grey } }
static define reflective_red texture { reflective { color red } }
static define reflective_blue texture { reflective { color blue } }
static define reflective_green texture { reflective { color green } }
static define reflective_cyan texture { reflective { color cyan } }
static define reflective_yellow texture { reflective { color yellow } }
static define reflective_magenta texture { reflective { color magenta } }
static define reflective_orange texture { reflective { color orange } }
static define reflective_coral texture { reflective { color coral } }
static define reflective_brown texture { reflective { color brown } }
static define reflective_tan texture { reflective { color tan } }
static define reflective_gold texture { reflective { color coral } }

// Metallic surfaces, reflection with brilliance to modify the diffuse
static define metallic_white texture { metallic { color white } }
static define metallic_grey texture { metallic { color grey } }
static define metallic_red texture { metallic { color red } }
static define metallic_blue texture { metallic { color blue } }
static define metallic_green texture { metallic { color green } }
static define metallic_cyan texture { metallic { color cyan } }
static define metallic_yellow texture { metallic { color yellow } }
static define metallic_magenta texture { metallic { color magenta } }
static define metallic_orange texture { metallic { color orange } }
static define metallic_coral texture { metallic { color coral } }
static define metallic_brown texture { metallic { color brown } }
static define metallic_tan texture { metallic { color tan } }
static define metallic_gold texture { metallic { color coral } }

// And now some specific colors
static define glass
texture {
   surface {
      ambient 0
      diffuse 0
      specular 0.2
      reflection white, 0.1
      transmission white, 1, 1.5
      }
   }

static define mirror
texture {
   surface {
      ambient white, 0.1
      diffuse white, 0.2
      specular 0
      reflection white, 1
      }
   }

// Now we define some color map lookup functions that provide the ability
// to mimic the predefined texture functions in other raytracers.  Most of
// these would be modified in some way, perhaps to add more or less turbulence
// to one of the variables, or perhaps to perform scaling in one or more
// direction.  The look of a texture that uses one of these functions will
// depend on how much it is scaled and what the color map looks like
static define agate_fn ((sin(6.2832*(1.1*z+2.6*noise(P,<2.8,0.5,4>)))+1)/2)^0.77
static define marble_fn sawtooth(x + 3 * noise(P, 5))
static define granite_fn noise(7*P, 5)
static define wood_fn sawtooth(sqrt(x*x+y*y))
static define odd_wood1
   sawtooth(|<x+sin(x+dnoise(P,3)[0]), y+sin(y+dnoise(P,3)[1]), 0>|)
static define noisy_wood_fn
   sawtooth(|<x+0.05*sin(6.28*(x+2*dnoise(P,3)[0])),
	      y+0.05*sin(6.28*(y+2*dnoise(P,3)[1])), 0>|)
static define bozo_fn noise(P, 1)
static define leopard_fn ((sin(x)+sin(y)+sin(z))/3)^2
static define onion_fn fmod(|P|, 1)
static define noisy_onion_fn fmod(|P|+noise(P, 3), 1)
static define checker_fn |fmod(floor(x)+floor(y)+floor(z), 2)|
static define checker3_fn |fmod(floor(x)+floor(y)+floor(z), 3)|
static define pi 3.1415926535897932384626
static define radial_fn (1 + atan2(x, z)/pi)/2

// Some standard sorts of color maps to use with the functions above (or
// modifications thereof).
static define white_marble_map
   color_map([0, 0.8, white, 0.5*white]
	     [0.8, 1, 0.5*white, 0.2*white])
static define red_marble_map
   color_map([0, 0.8, <1, 0.8, 0.8>, <1, 0.4, 0.4>]
	     [0.8, 1, <1, 0.4, 0.4>, <1, 0.1, 0.1>])
static define jade_color_map
   color_map([0, 0.8, <1, 0.6, 0.1>, <0, 0.3, 0>]
	     [0.8, 1, <0.1, 0.5, 0.1>, <0, 0, 0.3>])
static define sky_color_map
   color_map([0, 0.5, <0.2, 0.2, 0.5>, <0.2, 0.2, 0.5>]
	     [0.5, 0.6, <0.2, 0.2, 0.5>, 0.75*white]
	     [0.6, 1, 0.75*white, 0.25*white])
static define light_wood <0.6, 0.24, 0.1>
static define median_wood <0.3, 0.12, 0.03>
static define dark_wood <0.05, 0.01, 0.005>
static define wood_map1
   color_map([0.0, 0.2, light_wood, light_wood]
	     [0.2, 0.3, light_wood, median_wood]
	     [0.3, 0.4, median_wood, light_wood]
	     [0.4, 0.7, light_wood, light_wood]
	     [0.7, 0.8, light_wood, median_wood]
	     [0.8, 0.9, median_wood, light_wood]
	     [0.9, 1.0, light_wood, dark_wood])

//some defs by CM
static define psyched_cloudy_sky
texture {
   special surface {
      ambient 0.9
      diffuse 0
      specular 0
      color color_map(
         [0.0, 0.61, <0.4, 0.4, 0.4>, <1, 1, 1>]
         [0.6, 0.8, <1, 1, 1>, <0.146078, 0.6, 0.8>]
         [0.8, 1.4, <0.196078, 0.6, 0.8>, <0.146078, 0.6, 0.8>])[
         fmod(|P|+noise(P, 3), 1) ]
         //noise((3+0.2)*P, 2.5)]
      }
   }

static define cloudy_sky2
texture {
   special surface {
      ambient 0.9
      diffuse 0
      specular 0
      color color_map(
         [0.0, 0.61, <0.4, 0.4, 0.4>, <1, 1, 1>]
         [0.6, 0.8, <1, 1, 1>, <0.146078, 0.6, 0.8>]
         [0.8, 1.4, <0.196078, 0.6, 0.8>, <0.146078, 0.6, 0.8>])[
         noise(3.2*P, 2.5)]
      }
   }


} // End of if surrounding color defininitions
