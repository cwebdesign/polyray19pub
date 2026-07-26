# 5 Revision History

### Version 2.0.0prealpha
"Bowie" Build (named after David Bowie)
Released xxth July 2026
The first C++ version of Polyray to be released.

Replaced C-style constructs with C++ equivalents — e.g. migrated from printf to std::cout, NULL to nullptr, and header guards to include #pragma once.

Modernised macro patterns to functions — converted various macros to proper C++ functions, reducing preprocessor dependencies.
Adopted modern C++ features — introduced enum classes and other enumerations for type safety and namespace isolation, as well as memory allocation changed from polyray_malloc to
Use Factory functions (currently using new).

Cleaned up legacy code — removed scaffolding and refactored toward C++23 standards while maintaining functional equivalence across the renderer pipeline.

Polyray's C++ version builds on Windows (MSYS2,Visual Studio), Linux and MacOS using SDL to show the current render. It also builds without SDL on Tribblix (a Unix distribution).

Not using stl library anymore. Using lodepng library to handle png images.

Unit testing covers most of Polyray, using Google Test.


### Version 1.9.4
"Bella" Build (named after Malta's Eurovision 2026 Entry)
Released 16th May 2026
Switched to gcc 16.1.0 on MSYS2.
Various fixes to make it more secure.
This is the first open source version of Polyray with the blessing of the original developer.

### Version 1.9.3
"Travel Malta" Build (available on Github from this version)
Released 12th January 2023
Switched to gcc 13.2.0 on MSYS2.
Various fixes for old C cruft including uninitalised variable, fixing prototypes.
Changes for Linux and Macos compatilbity (Mac version will be available in the future).
This is still based on the stable C code but a newer C++ version is worked on (not directly based on version 1.9.2 beta).
Updated parser grammar. Skipcom has been deprecated and isn't used.
File includes work differently and environment variables should now work better under Windows.

###  Version 1.9.2
"Second Life" Beta Build
Released: 31st January 2020 
With full installer.
Switched to gcc 9.2.0, Mingw64/Msys2.0.
Further code changes. Switched to stb library for image loading, removed the old code for tga/jpeg reading.
Exper_node is now a std::variant. Introduced the use of std::arrays. Img is now cImg, a C++ class.
Bug fix in polynomial roots algorithm. Reduction of old C cruft and bring it up to date.
Code compiles under Visual Studio C++ too.
There may be a slight speedup as a result of various changes.

###  Version 1.9.2
"New Year 2020" Beta Build
Released: 5th January 2020
With full installer.
Switched to gcc 9.1.0, Mingw64/Msys2.0.
Further changes to switch part of the code to C++ standard. 
There may be a slight speedup as a result.

###  Version 1.9.1
"Chameleon Malta"  Stable Build on occasion of Eurovision Semifinal
Released: 16th May 2019 
With a full installer
Switched to gcc 8.3.0 and Mingw64.
Removed some insecure code

###  Version 1.9f
"Graphics Stars" Build
Released: 18th February 2015
With a full installer
Recompiled with TDM gcc 4.5.1 - 32 bit. 
Minor fixes, update to NSIS installer which now sets up Textpad shortcuts for Polyray, calling a Java compiler and the GCC c compiler.

###  Version 1.9 2009 'What if we' Build.
Full installer. 
Fixed uv_bounds bug.

###  Version 1.8e
"Human" Build
Released: 23rd February 2011
With a full installer

###  Version 1.8d
"Paloma Blanca" Build
Released: 16th February 2008
With a full installer
Recompiled with gcc 4.2.3 unofficial for MingW, with latest optimisations.
Added NSIS installer.
Minor code fixes.

###  Version 1.8d 2007 'The Core' Build:
New colours - besides gray, now there is Gray05, Gray10, up to Gray95. SkyBlue1 to 5 (use the new colors.inc in this zip file)
New random number generator
New matrix multiplication algorithm (faster)
Rough speed improvement should be around 33%
Fixed problem with some XP PC's environment variables
Compiled with gcc - special mingw version 3.4.5 - updated MingW environment

Set an environment variable POLYRAY_PATH to where you have placed polyray's dat subdirectory eg c:\polyray\dat
(under XP do this in the control panel, system)
Extended memory or DOS4GW is NOT needed for this version.
This is a full win32 mingw gcc compile for the first time.



(Other versions were released to CIS students at the University of Malta but are not listed here) 

###  Version 1.8
Released: 26 October 1995 (last version by the original author)

     o  Modified the way opacity in a color map interacts with transparency.
        It is now much closer to the way POV-Ray does it (this made the
        interface with MORAY far better, even though it kinda breaks the look
        of existing files).

     o  Removed GIF and added PNG image file formats.

     o  Added cylindrical and spherical indexed maps.

     o  Added 64K and 16 color video modes

     o  Keyword "noshadow" can be added to light.  Lights may also now
        be placed into object wrappers for use in CSG objects.

     o  Added several new primitives: superquadric, spherical height
        field, cylindrical height field, and an experimental 
        hypertexture primitive.  

     o  Added 3D point and line drawing

     o  Added several spline types

     o  added "brilliance" to surface components (power modifier of
        diffuse component)

     o  Added simple lens flares

     o  A full screen z-buffer is now used for all rendering modes.
        This means you may need to reduce the maximum amount of RAM
        dedicated to the screen to render large images (see -M command
        line flag).

     o  The environment variable POLYRAY_PATH will be used to find
        any include files.  Multiple directories can be put on this
        variable, in the same way they are for the standard PATH.

###  Version 1.7
Released: 17 March 1994

     o  Added parametric surfaces.  Now possible to define a mesh
        object (surface) where each point in the mesh is a function
        of u and v.

     o  Added bump maps

     o  Added noeval to definitions to cure a particle system bug

     o  Added support for GIF and JPEG images in textures, etc.

     o  Added support for system calls in 386 version.  Can now
        call an external program from within Polyray.

     o  The default shading flags for raytracing is now set to:
           SHADOW_CHECK + REFLECT_CHECK + TRANSMIT_CHECK +
           UV_CHECK + CAST_SHADOW
        The difference is that it is no longer assumed that surfaces
        should be lit on both sides (normal correction), this boosts
        rendering speed at the cost of funny shading once in a while.
        UV_CHECK was added to allow turning off checking of u/v bounds
        on objects (also a speedup).

     o  Changed u_steps, v_steps back to the way it was in v1.5.  Now
        for all objects it is uniformly subdivided by exactly the
        value of u_steps/v_steps.

     o  Significantly enhanced the function object.  Polyray will
        now accept any predefined function as part of the function
        definition.

     o  Added NURBS objects.  (Sorry, no trim curves yet.)

     o  Displacement functions are now possible on CSG objects.  They
        also work in raytracing now (although you may need to really
        boost the u_steps and v_steps values to get good results).

     o  Now supports /* ... */ comments (C style).  No you can't nest
        them.  You can nest the single line // comments within them.

     o  Internal modifications made that should result in less memory
        usage by objects.  Your mileage may vary.

     o  Modified numeric input to allow numbers like 1., .1

     o  Fixed up bug in polygon tracing that caused them to drop out
        if they were oriented in just the right way.

     o  Added a second raw triangle output format - only outputs the
        vertex coordinates.  This makes it more compatible with
        RAW2POV. Previous style with normals and u/v still available.

     o  Removed BSP tree bounding.  It wasn't any faster than slabs
        and locked up every once in a while.

     o  Added a glyph object to support TrueType style extruded
        surfaces. Glyphs are always closed at the top and bottom, they
        may have both straight and curved sides in the same shape.  A
        program to extract TrueType information and write into Polyray
        glyph format is included with the data file archive.

     o  Support for simple particle systems.  Can define birth and
        death conditions, # of objects to generate, and ability to
        bounce off other objects.

     o  Added many more VESA display modes.  There are now 5 SVGA 256
        color modes, 5 Hicolor modes, and 5 truecolor modes supported.
        If your board doesn't support a selected mode, Polyray tries a
        lower res one having the same # of bytes per pixel.

###  Version 1.6a (Crud, stuff was still broken...)
Released: 23 April 1993

     o  Problems in writing Targa images!  Uncompressed/RLE was being
        set improperly in the image file.

     o  Allocation/Deallocation of static variables had some problems.
        Mostly fixed, however: DONT use a non-static texture in a
        static object.

     o  Added some code to special surface to make use of an alpha
        value in a color map.  If you use, color xxx_map[foo_fn], it
        will grab the alpha value and use it for the transmission
        scale. (This overrides any transmission scale you might put
        in later, so beware.)

###  Version 1.6 (beta)
Released:

     o  Added opacity values to 16 and 32 bit Targa output.  Just a
        single bit in the 16 bit files.  In the 32 bit files the extra
        channel holds the percentage of the background that
        contributed to the pixel.

     o  Added static (last from frame to frame) variables

     o  Added indexed and summed textures.

     o  Added RLE compressed 8 bit Targa files.

     o  Added VESA Hicolor display in 640x480 (still a bit buggy).
        Don't use anything but '-t 0' or '-t 1' for status displays or
        the screen will go wierd after about 50 lines.

     o  Added texture maps (similar to color maps), indexed image
        files (for use with texture maps)

     o  Added output of raw triangle information.  Render type 3 will
        render the image as triangles, in ASCII form.  Each line of
        the output has the form: v1 v2 v3 n1 n2 n3 uv1 uv2 uv3.
        Where vx is a 3D vertex, nx is the normal at the corresponding
        vertex, and uv1 is a pair of u,v values for each vertex.
        (This means there are 24 floating point values per line.)

     o  Fixed bug involving conditional include files.  If an include
        directive was inside a conditional, the file was read
        regardless of the value of the conditional.

     o  Smoothed out Bezier surfaces.  A patch (smooth) triangle is
        now used instead of a flat one when performing final
        intersection tests.

     o  Fixed view bug in scan conversion - if the direction of view
        was along the y-axis & the up was along the z-axis, the image
        came out upside down.

     o  Added displacement to scan converted surfaces

     o  Added u-v mapping to most primitives.  The variables u and v
        work in all expressions.  This is especially useful for image
        mapping.

     o  Added u-v limits to several primitives.  This allows for
        creation of sections of a primitive.

     o  Changed scan conversion to be adaptive to the on-screen pixel
        size. Unless limited by u_steps or v_steps, the subdivision of
        a primitive continues until a polygon about the size of a
        pixel is created.  This is then drawn.  The biggest drawback
        is the appearance of cracks when the adaptive subdivision is
        different for adjacent parts of a surface.

     o  Changed the meaning of u_steps and v_steps for most
        primitives. They now mean the number of binary subdivisions
        that are performed rather than the number of steps.  Therefore
        u_steps 4 in this version is equivalent to u_steps 16 in
        previous versions. (Unaffected primitives: blobs, sweeps,
        lathes. These still work the old way.)

     o  Fixed dot product of vectors in expressions.

     o  Added Legendre polynomials as an expression.  Pretty cool for
        doing spherical harmonics.

     o  Added depth mapped lights.  Useful if you need to do shadows
        of things that can only be scan converted (like displacement
        surfaces).

     o  Fixed problems with using a torus in CSG.  (Only appeared in
        some versions of v1.5)

###  Version 1.5
(not released)

     o Buggy SVGA support removed.  Only standard VGA mode (320x200)
       supported.

     o Gridded objects added.

     o Arrays added

     o Components of CSG objects are now properly sorted by bounding
       slabs

     o User defined bounding slabs removed.  Polyray will always use
       bounding slabs aligned with the x, y, and z-axes.

     o Clipping and bounding objects removed.  Clipping is now
       performed in CSG, bounding is specified using a bounding_box
       declaration.

     o Added wireframe display mode.

     o Added planar blob types.  (Also added toroidal blob types, but
       they only appear in scan conversion images due to the extreme
       numerical precision needed to raytrace them.)

     o Added smooth height fields.

     o Fixed shading bug involving transparent objects & multiple
       light sources.

     o Fixed diffuse lighting from colored lights.

     o Changed RLE Targa output so that line boundaries are not
       crossed.

###  Version 1.4
Released: 11 April 1992

     o Support for many SVGA boards at 640x480 resolution in 256
       colors.  See documentation for the -V flag. (Note: SVGA
       displays only work on the 286 versions.)

     o Changed the way the status output is managed.  Now requires a
       number following the -t flag.  Note that line and pixel status
       will screw up SVGA displays - drawing goes to the wrong place
       starting around line 100.  If using SVGA display then either
       use no status, or totals.

     o Added cylindrical blob components.  Changed the syntax for
       blobs to accommodate the new type.

     o Added lathe surfaces made from either line segments or
       quadratic splines.

     o Added sweep surfaces made from quadratic splines.

     o Height field syntax changed slightly.  Non-square height fields
       now handled correctly.

     o Added adaptive antialiasing.

     o Squashed bug in shading routines that affected almost all
       primitives. This bug was most noticeable for objects that were
       scaled using different values for x, y, and z.

     o Added transparency values to color maps.

     o Added new keywords to the file polyray.ini: shadow_tolerance,
       antialias, alias_threshold, max_samples.  Lines that begin with
       // in polyray.ini are now treated as comments.

     o Short document called texture.txt is now included in
       PLYDOC.  This describes in a little more detail how to go
       about developing solid textures using Polyray.

     o Added command line argument -z start_line.  This allows the
       user to start a trace somewhere in the middle of an image.
       Note that an image that was started this way cannot be
       correctly resumed & completed.  (You may be able to use image
       cut and paste utilities though.)

###  Version 1.3
(not released)

     o Added support for scan converting implicit functions and
       polynomial surfaces using the marching cubes algorithm.  This
       technique can be slow, and is restricted to objects that have
       user defined bounding shapes, but now Polyray is able to scan
       convert any primitive.

     o A global shading flag has been added in order to selectively
       turn on/off some of the more time consuming shading options.
       This option will also allow for the use of raytracing as a way
       of determining shadows, reflectivity, and transparency during
       scan conversion.

     o Added new keywords to the file polyray.ini: pixel_size,
       pixel_encoding, shade_flags.

     o Improved refraction code to (mostly) handle transparent
       surfaces that are defined by CSG intersection.

     o Fixed discoloring of shadows that receive some light through a
       transparent object.

     o Jittered antialiasing was not being called when the option was
       selected, this has been fixed.

     o Fixed parsing of blobs and polygons that had large numbers of
       entries. Previously the parser would fail after 50-60 elements
       in a blob and the same number of vertices of a polygon.

     o In keeping with the format used by POV-Ray and Vivid, comments
       may now start with // as well as #.  The use of the pound
       symbol for comments may be phased out in future versions.

###  Version 1.2
Released: 16 February 1992

     o Scan conversion of many primitives, using Z-Buffer techniques.

     o New primitives: sweep surface, torus

     o Support for the standard 320x200 VGA display in 256 colors.

     o An initialization file, polyray.ini, is read before
       processing.  This allows greater flexibility in tuning many
       of the default values used by Polyray.

     o User defined bounding slabs added.  This greatly improves speed
       of rendering on data files with many small objects.

     o Noise surface added.

     o Symbol table routines completely reworked.  Improved speed for
       data files containing many definitions.

     o Bug in the texturing of height fields corrected.

###  Version 1.1
(not released)

     o Added parabola primitive

     o Dithering of rays, and objects

     o Blob code improved, shading corrected, intersection code is
       faster and returns fewer incorrect results.

###  Version 1.0
Released: 27 December 1991

     o Several changes in input syntax were made, the most notable
       result being that commas are required in many more places.  The
       reason for this is that due to the very flexible nature of
       expressions possible, a certain amount of syntactic sugar is
       required to remove ambiguities from the parser.

     o Several new primitives were added: boxes, cones, cylinders,
       discs, height fields, and Bezier patches.

     o A new way of doing textures was added - each component of the
       lighting model can be specified by an implicit function that is
       evaluated at run time.  Using this feature leads to slower
       textures, however because the textures are defined in the data
       file instead of within Polyray, development of mathematical
       texturing can be developed without making alterations to
       Polyray.

     o File flush commands in the data file and at the command line
       were added.

     o Several new Targa variants were added.

     o Image mapping added.

     o Numerous bug fixes have occurred.

###  Version 0.3 (beta)
Released: 14 October 1991

     o This release added Constructive Solid Geometry, functional
       surfaces defined in terms of transcendental functions, a
       checker texture, and compressed Targa output.

     o Polyray no longer accepted a list of bounding/clipping objects,
       only a single object is allowed. since CSG can be used to
       define complex shapes, this is not a limitation, and even
       better makes for cleaner data files.

###  Version 0.2 (beta)
(not released)

     o This release added animation support, defined objects,
       arithmetic expression parsing, and blobs.

###  Version 0.1 (beta)
(not released)

     o First incarnation of Polyray.  This version had code for
       polynomial equations and some of the basic surface types
       contained in mtv.
