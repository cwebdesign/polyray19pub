

#if !defined(__POLYRAY_SCREEN_DEFS)
#define __POLYRAY_SCREEN_DEFS

#ifdef TESTING
void FilterScan(Viewpoint* eye, Vec viewvec, Vec rightvec, int maxdepth,
	int ystart, int yend);
void Scan(Viewpoint *eye, Vec viewvec, Vec rightvec, int ystart, int yend);
void throw_rays(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec, Vec rightvec,
           Flt focaldist, Flt aperture, Ray *ray,
           Flt xlen, Flt ylen, Flt xdelta, Flt ydelta,
           Vec corner_colors[4], Flt corner_opacs[4],
           Vec color, Flt *opacity,
           int depth, int limit);
void single_pixel(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec,
             Flt focaldist, Flt aperture, Ray *ray,
             int depth, Vec color, Flt *opacity);

#endif

void Screen(Viewpoint* eye, int y_start, int y_end);


#endif /* __POLYRAY_SCREEN_DEFS */

