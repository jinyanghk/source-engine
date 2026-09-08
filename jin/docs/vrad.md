
```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vrad -game hl2
LoadLibrary: pModule: vrad_dll.dll, path: /home/jin/source-engine/hl2/bin/libvrad_dll.so
Valve Software - vrad.exe (Sep  7 2026)

      Valve Radiosity Simulator
Command line: "./vrad" "-game" "hl2"

usage  : vrad [options...] bspfile
example: vrad c:\hl2\hl2\maps\test

Common options:

  -v (or -verbose): Turn on verbose output (also shows more command
  -bounce #       : Set max number of bounces (default: 100).
  -fast           : Quick and dirty lighting.
  -fastambient    : Per-leaf ambient sampling is lower quality to save compute time.
  -final          : High quality processing. equivalent to -extrasky 16.
  -extrasky n     : trace N times as many rays for indirect light and sky ambient.
  -low            : Run as an idle-priority process.
  -mpi            : Use VMPI to distribute computations.
  -rederror       : Show errors in red.

  -vproject <directory> : Override the VPROJECT environment variable.
  -game <directory>     : Same as -vproject.

Other options:
  -novconfig      : Don't bring up graphical UI on vproject errors.
  -dump           : Write debugging .txt files.
  -dumpnormals    : Write normals to debug files.
  -dumptrace      : Write ray-tracing environment to debug files.
  -threads        : Control the number of threads vbsp uses (defaults to the #
                    or processors on your machine).
  -lights <file>  : Load a lights file in addition to lights.rad and the
                    level lights file.
  -noextra        : Disable supersampling.
  -debugextra     : Places debugging data in lightmaps to visualize
                    supersampling.
  -smooth #       : Set the threshold for smoothing groups, in degrees
                    (default 45).
  -dlightmap      : Force direct lighting into different lightmap than
                    radiosity.
  -stoponexit      : Wait for a keypress on exit.
  -mpi_pw <pw>    : Use a password to choose a specific set of VMPI workers.
  -nodetaillight  : Don't light detail props.
  -centersamples  : Move sample centers.
  -luxeldensity # : Rescale all luxels by the specified amount (default: 1.0).
                    The number specified must be less than 1.0 or it will be
                    ignored.
  -loghash        : Log the sample hash table to samplehash.txt.
  -onlydetail     : Only light detail props and per-leaf lighting.
  -maxdispsamplesize #: Set max displacement sample size (default: 512).
  -softsun <n>    : Treat the sun as an area light source of size <n> degrees.                    Produces soft shadows.
                    Recommended values are between 0 and 5. Default is 0.
  -FullMinidumps  : Write large minidumps on crash.
  -chop           : Smallest number of luxel widths for a bounce patch, used on edges
  -maxchop                 : Coarsest allowed number of luxel widths for a patch, used in face interiors

  -LargeDispSampleRadius: This can be used if there are splotches of bounced light
                          on terrain. The compile will take longer, but it will gather
                          light across a wider area.
  -StaticPropLighting   : generate backed static prop vertex lighting
  -StaticPropPolys   : Perform shadow tests of static props at polygon precision
  -OnlyStaticProps   : Only perform direct static prop lighting (vrad debug option)
  -StaticPropNormals : when lighting static props, just show their normal vector
  -textureshadows : Allows texture alpha channels to block light - rays intersecting alpha surfaces will sample the texture
  -noskyboxrecurse : Turn off recursion into 3d skybox (skybox shadows on world)
  -nossprops      : Globally disable self-shadowing on static props
```


```sh
cd hl2

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vrad -game hl2 ../../d1_town_01_d.bsp
```