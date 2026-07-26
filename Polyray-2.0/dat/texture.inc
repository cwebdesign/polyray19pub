// Various textures
//
// This file requires that "colors.inc" already be parsed
//
if (frame == start_frame) {
//
// Names of the various shading flags
static define shadow_check       1
static define reflect_check      2
static define transmit_check     4
static define two_sided_surface  8
static define check_uv          16
static define normal_correct    32
static define cast_shadows      64

// Default shading flags during raytracing - everything gets checked.
static define all_shading_flags
   shadow_check + reflect_check + transmit_check + cast_shadows +
   two_sided_surface + check_uv

//
// Define a set of shading flags for surfaces that shouldn't have their
// backsided illuminated
static define one_sided_surface
   all_shading_flags - two_sided_surface

// Useful definitions to use with noise surfaces.  These
// will come in handy below when we declare marble and
// wood textures.
static define position_plain       0
static define position_objectx     1
static define position_worldx      2
static define position_cylindrical 3
static define position_spherical   4
static define position_radial      5

static define lookup_plain    0
static define lookup_sawtooth 1
static define lookup_cos      2
static define lookup_ramp     3

static define plain_normal  0
static define bump_normal   1
static define ripple_normal 2
static define dented_normal 3

// The standard sort of marble texture
static define white_marble
texture { special shiny { color white_marble_map[marble_fn] } }

// Nice blue agate texture
static define blue_agate_map
      color_map([0.0, 0.3, <0, 0, 0.9>, <0, 0, 0.8>]
	        [0.3, 1,   <0, 0, 0.8>, <0, 0, 0.4>])
static define sapphire_agate
texture { special shiny { color blue_agate_map[agate_fn] } }

// Simple color map texture
static define whorl_texture
texture {
   special shiny {
      color color_map([0.0, 0.3, green,   blue]
		      [0.3, 0.6, blue,    skyblue]
		      [0.6, 0.8, skyblue, orange]
		      [0.8, 1.0, orange,  red])[noise(P)]
      }
   scale <0.5, 0.5, 0.5>
   }

// Create a wood texture.  Concentric rings of color
// are wrapped around the z-axis.  There is some turbulence
// in order to keep the rings from looking too perfect.
static define wooden
   texture {
      special shiny {
	 color wood_map1[wood_fn]
	 position P + 0.5 * dnoise(P, 3)
	 }
      }

// Define a texture using a color wheel
static define xz_wheel_texture
texture {
   special surface {
      color color_wheel(x, y, z)
      ambient 0.2
      diffuse 0.8
      specular white, 0.2
      microfacet Reitz 10
      }
   }

// This is an example of a gradient texture.
static define mountain_color_map
      color_map(
         [-128,   0, blue,  blue]
         [   0,  20, green, green]
         [  20,  40, green, tan]
         [  40,  90, tan,   tan]
         [  90, 128, white, white])
static define mountain_colors
texture { special shiny { color mountain_color_map[y] } }

// This makes a nice texture to use on leaves
static define bumpy_green
texture {
   special shiny {
      color green normal N + (dnoise(3*W) - <0.5, 0.5, 0.5>)
      }
   scale <0.02, 0.02, 0.02>
   }

// Red dented/wrinkled appearance
static define dented_red
texture {
   special shiny {
      color <1, 0.2, 0.2>
      normal N + (dnoise(2*W) - <0.5,0.5,0.5>)
      }
   }

// When used on a big sphere, this makes a nice cloudy sky.  You will
// probably need to scale to make it look good.
static define cloudy_sky_map
      color_map(
	 [0.0, 0.6, <0.4, 0.4, 0.4>, <1, 1, 1>]
	 [0.6, 0.8, <1, 1, 1>, <0.196078, 0.6, 0.8>]
	 [0.8, 1.0, <0.196078, 0.6, 0.8>, <0.196078, 0.6, 0.8>])
static define cloudy_sky
texture {
   special surface {
      color cloudy_sky_map[noise(P)]
      ambient 0.9
      diffuse 0
      specular 0
      }
   }

//
// Definitions to make a ripple texture.  Adjust the values of ripple_freq
// and ripple_phase below for the desired effect
//
static define ripple_freq 10
static define ripple_phase 0

// Centers of the ripple effect
static define ripple_center1 brownian(black, 2*white)
static define ripple_center2 brownian(black, 2*white)
static define ripple_center3 brownian(black, 2*white)
static define ripple_center4 brownian(black, 2*white)
static define ripple_center5 brownian(black, 2*white)

// Direction of travel of the ripples
static define ripple_axis1 (P - ripple_center1)
static define ripple_axis2 (P - ripple_center2)
static define ripple_axis3 (P - ripple_center3)
static define ripple_axis4 (P - ripple_center4)
static define ripple_axis5 (P - ripple_center5)

// Piece together the centers to make an overall ripple
static define ripple_val1 cos(|ripple_axis1| * ripple_freq + ripple_phase)
static define ripple_val2 cos(|ripple_axis2| * ripple_freq + ripple_phase)
static define ripple_val3 cos(|ripple_axis3| * ripple_freq + ripple_phase)
static define ripple_val4 cos(|ripple_axis4| * ripple_freq + ripple_phase)
static define ripple_val5 cos(|ripple_axis5| * ripple_freq + ripple_phase)
static define ripple_val ripple_axis1 * ripple_val1 / (5 * |ripple_axis1|) +
		  ripple_axis2 * ripple_val2 / (5 * |ripple_axis2|) +
		  ripple_axis3 * ripple_val3 / (5 * |ripple_axis3|) +
		  ripple_axis4 * ripple_val4 / (5 * |ripple_axis4|) +
		  ripple_axis5 * ripple_val5 / (5 * |ripple_axis5|)

/* Adjust the color, reflectivity, and scale of this texture for a
   nice ripple effect */
static define basic_ripple_texture
texture {
   special matte {
      color 0.8*white
      normal N + ripple_val
      }
   }

static define blue_ripple
texture {
   special shiny {
      color <0.4, 0.4, 1.0>
      normal N + ripple_val
      reflection 0.5
      }
   }

/* Bumpy_Silver - */
static define bumpy_silver
texture {
   special surface {
      color 0.9*white
      normal N + 0.2 * (dnoise(5*W) - <0.5,0.5,0.5>)
      ambient 0.05
      diffuse 0.1
      specular 0.9*white, 0.3
      reflection 0.8
      microfacet Cook 5
      }
   }

static define dark_wooden wooden

/* A fairly diffuse white texture */
static define milky_white
texture { shiny {
   color white
   ambient 0.2
   diffuse 0.1
   transmission white, 0.4, 1.0
   specular white, 0.1
   microfacet Cook 5
   } }

/* Tigers eye colors */
static define tigers_eye_colors2 color_map(
   [0.000, 0.115, <0.698, 0.533, 0.122>, 0.000, <0.529, 0.173, 0.071>, 0.000]
   [0.115, 0.230, <0.529, 0.173, 0.071>, 0.000, <0.698, 0.533, 0.122>, 0.000]
   [0.230, 0.336, <0.698, 0.533, 0.122>, 0.000, <0.424, 0.325, 0.075>, 0.000]
   [0.336, 0.460, <0.424, 0.325, 0.075>, 0.000, <0.698, 0.533, 0.122>, 0.000]
   [0.460, 0.628, <0.698, 0.533, 0.122>, 0.000, <0.306, 0.235, 0.055>, 0.000]
   [0.628, 0.708, <0.306, 0.235, 0.055>, 0.000, <0.698, 0.341, 0.122>, 0.000]
   [0.708, 0.796, <0.698, 0.341, 0.122>, 0.000, <0.000, 0.000, 0.000>, 0.000]
   [0.796, 0.858, <0.000, 0.000, 0.000>, 0.000, <0.596, 0.373, 0.106>, 0.000]
   [0.858, 0.982, <0.596, 0.373, 0.106>, 0.000, <0.494, 0.376, 0.086>, 0.000]
   [0.982, 1.000, <0.494, 0.376, 0.086>, 0.000, <0.698, 0.533, 0.122>, 0.000])

/* Tigers_Eye1 - A noisy tigers eye texture */
static define Tigers_Eye1
texture {
   special shiny {
      color tigers_eye_colors2[marble_fn]
      }
   }

/* Tigers_Eye2 - A fractured tigers eye matrix */
static define noisy_x x + 1.5 * dnoise(P)[0]
static define noisy_y y + 1.5 * dnoise(P)[1]
static define noisy_z y + 1.5 * dnoise(P)[2]
static define noisy_x1 noisy_x - floor(noisy_x)
static define noisy_y1 noisy_y - floor(noisy_y)
static define noisy_z1 noisy_z - floor(noisy_z)
static define noisy_checker3_fn fmod((noisy_x1+noisy_y1+noisy_z1)/3, 1)
static define Tigers_Eye2 texture {
   special shiny { color tigers_eye_colors2[noisy_checker3_fn] }
   scale <0.5, 0.5, 0.5>
   }

/* splash1 - A collection of primary colors in a splattered pattern */

static define dens1 0.6
static define dens2 (dens1 + 1) / 2
static define splash1_layer1_map color_map(
   [0, dens1, white, 1, white, 1]
   [dens1, dens2, red/3, 0.5, red, 0]
   [dens2, 1, red, red])
static define splash1_layer2_map color_map(
   [0, dens1, white, 1, white, 1]
   [dens1, dens2, blue/3, 0.5, blue, 0]
   [dens2, 1, blue, blue])
static define splash1_layer3_map color_map(
   [0, dens1, white, 1, white, 1]
   [dens1, dens2, green/3, 0.5, green, 0]
   [dens2, 1, green, green])
static define splash1_layer4_map color_map(
   [0, dens1, white, 1, white, 1]
   [dens1, dens2, yellow/3, 0.5, yellow, 0]
   [dens2, 1, yellow, yellow])

static define noisy_x3 x + 5 * dnoise(4*P)[0]
static define noisy_y3 y + 5 * dnoise(4*P)[1]
static define noisy_z3 y + 5 * dnoise(4*P)[2]
static define noisy_x4 noisy_x3 - floor(noisy_x3)
static define noisy_y4 noisy_y3 - floor(noisy_y3)
static define noisy_z4 noisy_z3 - floor(noisy_z3)
static define noisy_checker3_fn fmod((noisy_x4+noisy_y4+noisy_z4)/3, 1)

static define splash1_layer1
texture { special shiny { color splash1_layer1_map[noisy_checker3_fn] } }

static define splash1_layer2
texture {
   special shiny { color splash1_layer2_map[noisy_checker3_fn] }
   scale <0.75, 1.7, 0.9>
   rotate <60,-30, 0>
   }

static define splash1_layer3
texture {
   special shiny { color splash1_layer3_map[noisy_checker3_fn] }
   scale <1.5, 0.7, 1.2>
   rotate <0, 30, 20>
   }

static define splash1_layer4
texture {
   special shiny { color splash1_layer4_map[noisy_checker3_fn] }
   scale <1.5, 0.7, 1.2>
   rotate <10,-80, 0>
   }

static define splash1
texture {
   layered splash1_layer1, splash1_layer2, splash1_layer3,
	   splash1_layer4, matte_white
   }

/* Lapis_Lazuli -
   Adapted from the Persistence of Vision Raytracer
   Lapis Lazuli texture by Chad Eby */

static define lapis_layer1
color_map([0.0, 0.1, blue, midnight_blue]
          [0.1, 0.2, blue, midnight_blue]
          [0.2, 0.3, blue, midnight_blue]
          [0.3, 0.4, blue, midnight_blue]
          [0.4, 0.5, blue, midnight_blue]
          [0.5, 0.75, midnight_blue, slate_blue]
          [0.75, 0.9, slate_blue, gold]
	  [0.9, 0.97, midnight_blue, midnight_blue]
          [0.97, 1.0, slate_blue, gold])

static define lapis_layer2
color_map([0.0, 0.4, white, 1.0, white, 1.0]
	  [0.4, 0.5, white, 1.0, white, 0.6]
	  [0.5, 0.7, white, 0.6, white, 0.6]
	  [0.7, 0.8, white, 0.6, white, 1.0]
	  [0.8, 0.97, white, 1.0, white/2, 0.5]
	  [0.97, 0.98, <0.2,0.2,1.0>, 0.5, blue, 0]
	  [0.98, 0.99, white, 0.5, white/2, 0.5]
	  [0.99, 1.0, white/2, 0.5, blue, 0])

static define lapis_layer2_fn sawtooth(x + 2*noise(P, 4))
static define lapis_layer1_fn noise(15*P, 5)
static define Lapis_Lazuli
texture {
   layered
      texture { special shiny { color lapis_layer2[lapis_layer2_fn] } },
      texture { special shiny { color lapis_layer1[lapis_layer1_fn] } }
   }

static define snow_offset_y (y + 1) / 2
static define snow_start      0.1  // lowest boundary (no snow below this point)
static define snow_alt_range  0.8  // points above start+range are snowy
static define snow_alt_factor 0.8  // importance of slope in accumulation
static define snow_threshold  0.3
static define snow_value ((snow_offset_y-snow_start)/snow_alt_range)*
			 snow_alt_factor
static define snow_slope1 N[1]

static define snow_color
   (snow_offset_y < snow_start
    ? blue
    : (snow_slope1 < 0
       ? ForestGreen
       : (snow_offset_y > snow_start + snow_alt_range
	  ? white
	  : (snow_value * snow_slope1 > snow_threshold ? white : ForestGreen))))

static define snow
   texture { special surface { color snow_color ambient 0.2 diffuse 0.8 } }

} // End of definitions
