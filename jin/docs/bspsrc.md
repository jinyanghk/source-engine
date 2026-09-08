
https://github.com/ata4/bspsrc

```sh
$ ./bspsrc.sh --help
Usage: bspsrc [OPTIONS] <paths>...

Parameters:
      <paths>...             One or more bsp files or folders.
                             Alternatively, if --list is specified, one or more
                               text files containing a list of bsp file or
                               folder paths.

Options:
      --appids               List all available application IDs
  -d, --debug                Enable debug mode. Increases verbosity and adds
                               additional data to the VMF file.
  -r, --recursive            Recursively decompile files found in
                               subdirectories.
  -o, --output=<path>        Override output path for VMF file(s). Treated as
                               directory if multiple BSP files are provided.
  -l, --list                 Treat specified files as text files containing a
                               BSP file list. BSP files are seperated by new
                               lines.
  -h, --help                 Show this help message and exit.
  -V, --version              Print version information and exit.

Entity related options
      --no_point_ents        Don't write any point entities.
      --no_brush_ents        Don't write any brush entities.
      --no_sprp              Don't write prop_static entities.
      --no_overlays          Don't write info_overlay entities.
      --no_cubemaps          Don't write env_cubemap entities.
      --no_details           Don't write func_detail entities.
      --no_areaportals       Don't write func_areaportal(_window) entities.
      --no_occluders         Don't write func_occluder entities.
      --no_ladders           Don't write func_ladder entities.
      --no_visclusters       Don't write func_viscluster entities.
      --no_rotfix            Don't fix instance entity brush rotations for
                               Hammer.
      --force_manual_areaportal
                             Force manual entity mapping for areaportal
                               entities.
      --merge_details        Merge func_detail brushes with touch into one
                               entity.

Brush related options
      --no_brushes           Don't write any world brushes.
      --no_disps             Don't write displacement surfaces.
      --brushmode=<mode>     Brush decompiling mode:
                             null - brushes and planes
                             null - original faces only
                             null - original + split faces
                             null - split faces only
                               Default: Brushes and planes
      --thickness=<value>    Thickness of brushes create from flat faces in
                               units.
                               Default: 1.0

Texture related options
      --facetex=<texture>    Replace all face textures with this one.
                               Default:
      --bfacetex=<texture>   Replace all back-face textures with this one. Used
                               in face-based decompiling modes only.
                               Default:
      --no_cubemaptexfix     Don't fix environment-mapped materials.
      --no_ttfix             Don't fix tool textures such as toolsnodraw or
                               toolsblocklight.
      --nodraw_invis-sides   Apply toolsnodraw texture to brushsides which are
                               not rendered in game. This applied mainly to
                               brushsides facing the void. Tool Texture fixed
                               brushsides are not affected by this.

Miscellaneous options
      --no_vmf               Don't write any VMF files, read BSP only.
      --no_lumpfiles         Don't load lump files (.lmp) associated with the
                               BSP file.
      --no_prot              Skip decompiling protection checking. Can increase
                               speed when mass-decompiling unprotected maps.
      --no_visgroups         Don't group entities from instances into visgroups.
      --no_cams              Don't create Hammer cameras above each player
                               spawn.
      --appid=<id>           Overrides game detection by using this Steam
                               Application ID instead
                             Use -appids to list all known app-IDs.
                               Default: 0
      --format=<format>      Sets the VMF format used for the decompiled maps:
                             AUTO - Automatic
                             OLD - Source 2004 to 2009
                             NEW - Source 2010 and later
                               Default: Automatic
      --unpack_embedded      Unpack embedded files in the bsp.
      --no_smart_unpack      Disable 'smart' extracting of embedded files.
                             Smart extracting automatically skips all files
                               generated by vbsp, that are only relevant to
                               running the map in the engine.
```