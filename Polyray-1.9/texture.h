#if !defined(__POLYRAY_TEXTURE_DEFS)
#define __POLYRAY_TEXTURE_DEFS

/* Define texture functions */
void create_plain(Texture *, Special_Surface *);
void create_checker(Texture *, Texture *, Texture *);
void create_special(Texture *, void *);
void create_noise(Texture *, Special_Surface *);
void create_hexagon(Texture *, Texture *, Texture *, Texture *);
void create_layered(Texture *, tstackptr);
void create_indexed(Texture *, NODE_PTR, texture_map_entries);
void create_summed(Texture *, texture_fn_entries);
void TextureCopy(Texture *, Texture *);
void TextureDelete(Texture *);
void TextureShear(Texture *, Flt, Flt, Flt, Flt, Flt, Flt);
void TextureTranslate(Texture *, Vec);
void TextureRotate(Texture *, Vec);
void TextureAxisRotate(Texture *, Vec, Flt);
void TextureScale(Texture *, Vec);
void copy_special0(Special_Surface *, Special_Surface *);
void delete_texture_map(texture_map_entries);

#endif /* __POLYRAY_TEXTURE_DEFS */

