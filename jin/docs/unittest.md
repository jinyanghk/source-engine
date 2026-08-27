```sh
$ python3 ./waf configure -T debug --prefix=hl2 --tests --disable-warns
Setting top to                                          : /home/jin/source-engine
Setting out to                                          : /home/jin/source-engine/build
Build type                                              : debug
LTO build                                               : no
PolyOpt build                                           : no
Checking for 'gcc' (C compiler)                         : /usr/bin/gcc
Target OS                                               : linux
Target CPU                                              : x86_64
Target binfmt                                           : elf
Checking for 'g++' (C++ compiler)                       : /usr/bin/g++
Target OS                                               : linux
Target CPU                                              : x86_64
Target binfmt                                           : elf
Checking for c flags '-MMD'                             : yes
Checking for cxx flags '-MMD'                           : yes
Checking for program 'git'                              : /usr/bin/git
Checking git hash                                       : e5f69ea2
Checking for program 'strip'                            : /usr/bin/strip
Checking for program 'objcopy'                          : /usr/bin/objcopy
Testing 32bit support                                   : no
Testing 64bit support                                   : yes
Checking for required C flags                           : yes
Checking for required C++ flags                         : yes
Checking supported flags for g++ in parallel            : started
... -w                                                  : yes
-> processing test results                              : all ok
Checking supported flags for gcc in parallel            : started
... -w                                                  : yes
... -fnonconst-initializers                             : no
-> processing test results                              : 1 test failed
Checking for library dl                                 : yes
Checking for library bz2                                : yes
Checking for library rt                                 : yes
Checking for library m                                  : yes
--> appframework                                        : in progress
<-- appframework                                        : done
--> tier0                                               : in progress
<-- tier0                                               : done
--> tier1                                               : in progress
<-- tier1                                               : done
--> tier2                                               : in progress
<-- tier2                                               : done
--> tier3                                               : in progress
<-- tier3                                               : done
--> unitlib                                             : in progress
<-- unitlib                                             : done
--> mathlib                                             : in progress
<-- mathlib                                             : done
--> vstdlib                                             : in progress
<-- vstdlib                                             : done
--> filesystem                                          : in progress
<-- filesystem                                          : done
--> vpklib                                              : in progress
<-- vpklib                                              : done
--> unittests/tier0test                                 : in progress
<-- unittests/tier0test                                 : done
--> unittests/tier1test                                 : in progress
<-- unittests/tier1test                                 : done
--> unittests/tier2test                                 : in progress
<-- unittests/tier2test                                 : done
--> unittests/tier3test                                 : in progress
<-- unittests/tier3test                                 : done
--> unittests/mathlibtest                               : in progress
<-- unittests/mathlibtest                               : done
--> utils/unittest                                      : in progress
<-- utils/unittest                                      : done
'configure' finished successfully (0.867s)
```

```sh
$ python3 ./waf build
Build commands will be stored in build/compile_commands.json
Waf: Entering directory `/home/jin/source-engine/build'
[  1/167] Compiling tier0/pme_posix.cpp
[  2/167] Compiling tier0/memstd.cpp
[  3/167] Compiling tier0/cpumonitoring.cpp
[  4/167] Compiling tier0/platform_posix.cpp
[  5/167] Compiling tier0/cpu_posix.cpp
[  6/167] Compiling tier0/memdbg.cpp
[  7/167] Compiling tier0/vprof.cpp
[  8/167] Compiling tier0/tslist.cpp
[  9/167] Compiling tier0/mem_helpers.cpp
[ 10/167] Compiling tier0/cpu.cpp
[ 11/167] Compiling tier0/tier0_strtools.cpp
[ 12/167] Compiling tier0/threadtools.cpp
[ 13/167] Compiling tier0/thread.cpp
[ 14/167] Compiling tier0/systeminformation.cpp
[ 15/167] Compiling tier0/minidump.cpp
[ 16/167] Compiling tier0/pch_tier0.cpp
[ 17/167] Compiling tier0/commandline.cpp
[ 18/167] Compiling tier0/assert_dialog.cpp
[ 19/167] Compiling tier0/dbg.cpp
[ 20/167] Compiling tier0/mem.cpp
[ 21/167] Compiling tier0/fasttimer.cpp
[ 22/167] Compiling tier0/PMELib.cpp
[ 23/167] Compiling tier0/progressbar.cpp
[ 24/167] Compiling tier0/stacktools.cpp
[ 25/167] Compiling tier0/dynfunction.cpp
[ 26/167] Compiling tier0/memvalidate.cpp
[ 27/167] Compiling tier0/security.cpp
[ 28/167] Compiling tier0/vcrmode_posix.cpp
[ 29/167] Compiling tier0/cpu_usage.cpp
[ 30/167] Compiling tier1/lzss.cpp
[ 31/167] Compiling tier1/datamanager.cpp
[ 32/167] Compiling tier1/commandbuffer.cpp
[ 33/167] Compiling tier1/processor_detect_linux.cpp
[ 34/167] Compiling tier1/tokenreader.cpp
[ 35/167] Compiling utils/lzma/C/LzmaDec.c
[ 36/167] Compiling tier1/tier1.cpp
[ 37/167] Compiling tier1/strtools.cpp
[ 38/167] Compiling tier1/strtools_unicode.cpp
[ 39/167] Compiling tier1/mempool.cpp
[ 40/167] Compiling tier1/memstack.cpp
[ 41/167] Compiling tier1/diff.cpp
[ 42/167] Compiling tier1/NetAdr.cpp
[ 43/167] Compiling tier1/newbitbuf.cpp
[ 44/167] Linking build/tier0/libtier0.so
[ 45/167] Compiling tier1/checksum_crc.cpp
[ 46/167] Compiling tier1/rangecheckedvar.cpp
[ 47/167] Compiling tier1/reliabletimer.cpp
[ 48/167] Compiling tier1/qsort_s.cpp
[ 49/167] Compiling tier1/splitstring.cpp
[ 50/167] Compiling tier1/snappy-sinksource.cpp
[ 51/167] Compiling tier1/bitbuf.cpp
[ 52/167] Compiling tier1/checksum_sha1.cpp
[ 53/167] Compiling tier1/snappy-stubs-internal.cpp
[ 54/167] Compiling tier1/checksum_md5.cpp
[ 55/167] Compiling tier1/uniqueid.cpp
[ 56/167] Compiling tier1/snappy.cpp
[ 57/167] Compiling tier1/characterset.cpp
[ 58/167] Compiling tier1/utlsymbol.cpp
[ 59/167] Compiling tier1/sparsematrix.cpp
[ 60/167] Compiling tier1/stringpool.cpp
[ 61/167] Compiling tier1/convar.cpp
[ 62/167] Compiling tier1/utlbufferutil.cpp
[ 63/167] Compiling tier1/generichash.cpp
[ 64/167] Compiling tier1/ilocalize.cpp
[ 65/167] Compiling tier1/utlstring.cpp
[ 66/167] Compiling tier1/interface.cpp
[ 67/167] Compiling tier1/KeyValues.cpp
[ 68/167] Compiling tier1/byteswap.cpp
[ 69/167] Compiling tier1/utlbinaryblock.cpp
[ 70/167] Compiling tier1/keyvaluesjson.cpp
[ 71/167] Compiling tier1/kvpacker.cpp
[ 72/167] Compiling tier1/utlbuffer.cpp
[ 73/167] Compiling tier1/lzmaDecoder.cpp
[ 74/167] Compiling unitlib/unitlib.cpp
[ 75/167] Compiling tier2/vconfig.cpp
[ 76/167] Compiling tier2/tier2.cpp
[ 77/167] Compiling tier2/fileutils.cpp
[ 78/167] Compiling tier2/keybindings.cpp
[ 79/167] Linking build/unitlib/libunitlib.so
[ 80/167] Compiling tier2/util_init.cpp
[ 81/167] Compiling tier2/utlstreambuffer.cpp
[ 82/167] Compiling tier2/renderutils.cpp
[ 83/167] Compiling tier2/p4helpers.cpp
[ 84/167] Compiling tier2/defaultfilesystem.cpp
[ 85/167] Compiling tier2/riff.cpp
[ 86/167] Compiling tier2/camerautils.cpp
[ 87/167] Compiling tier2/dmconnect.cpp
[ 88/167] Compiling public/materialsystem/MaterialSystemUtil.cpp
[ 89/167] Compiling tier2/keyvaluesmacros.cpp
[ 90/167] Compiling tier2/beamsegdraw.cpp
[ 91/167] Compiling tier2/soundutils.cpp
[ 92/167] Linking build/tier1/libtier1.a
[ 93/167] Compiling tier2/meshutils.cpp
[ 94/167] Compiling public/map_utils.cpp
[ 95/167] Compiling mathlib/powsse.cpp
[ 96/167] Compiling mathlib/color_conversion.cpp
[ 97/167] Compiling mathlib/almostequal.cpp
[ 98/167] Compiling mathlib/randsse.cpp
[ 99/167] Compiling mathlib/simdvectormatrix.cpp
[100/167] Compiling mathlib/IceKey.cpp
[101/167] Compiling mathlib/lightdesc.cpp
[102/167] Compiling mathlib/spherical.cpp
[103/167] Compiling mathlib/vmatrix.cpp
[104/167] Compiling mathlib/polyhedron.cpp
[105/167] Compiling mathlib/anorms.cpp
[106/167] Compiling mathlib/vector.cpp
[107/167] Compiling mathlib/quantize.cpp
[108/167] Compiling mathlib/imagequant.cpp
[109/167] Linking build/tier2/libtier2.a
[110/167] Compiling mathlib/bumpvects.cpp
[111/167] Compiling mathlib/3dnow.cpp
[112/167] Compiling mathlib/sseconst.cpp
[113/167] Compiling mathlib/mathlib_base.cpp
[114/167] Compiling mathlib/ssenoise.cpp
[115/167] Compiling mathlib/sparse_convolution_noise.cpp
[116/167] Compiling mathlib/halton.cpp
[117/167] Compiling mathlib/sse.cpp
[118/167] Compiling tier3/scenetokenprocessor.cpp
[119/167] Compiling tier3/tier3.cpp
[120/167] Compiling tier3/studiohdrstub.cpp
[121/167] Compiling tier3/mdlutils.cpp
[122/167] Compiling tier3/choreoutils.cpp
[123/167] Compiling vstdlib/jobthread.cpp
[124/167] Compiling vstdlib/random.cpp
[125/167] Compiling vstdlib/cvar.cpp
[126/167] Compiling vstdlib/vcover.cpp
[127/167] Compiling vstdlib/KeyValuesSystem.cpp
[128/167] Compiling vstdlib/coroutine.cpp
[129/167] Compiling public/tier0/memoverride.cpp
[130/167] Linking build/mathlib/libmathlib.a
[131/167] Compiling public/filesystem_init.cpp
[132/167] Compiling appframework/AppSystemGroup.cpp
[133/167] Compiling appframework/posixapp.cpp
[134/167] Compiling common/simplebitstring.cpp
[135/167] Compiling vpklib/packedstore.cpp
[136/167] Compiling filesystem/QueuedLoader.cpp
[137/167] Compiling filesystem/basefilesystem.cpp
[138/167] Compiling filesystem/filetracker.cpp
[139/167] Compiling public/tier0/memoverride.cpp
[140/167] Linking build/tier3/libtier3.a
[141/167] Compiling public/kevvaluescompiler.cpp
[142/167] Linking build/vstdlib/libvstdlib.so
[143/167] Compiling filesystem/linux_support.cpp
[144/167] Compiling filesystem/filesystem_stdio.cpp
[145/167] Compiling public/zip_utils.cpp
[146/167] Compiling filesystem/filesystem_async.cpp
[147/167] Compiling filesystem/packfile.cpp
[148/167] Compiling unittests/tier0test/tslisttests.cpp
[149/167] Compiling unittests/tier0test/tier0test.cpp
[150/167] Linking build/appframework/libappframework.a
[151/167] Linking build/unittests/tier0test/libtier0test.so
[152/167] Compiling unittests/tier1test/lzsstest.cpp
[153/167] Compiling unittests/tier1test/commandbuffertest.cpp
[154/167] Compiling unittests/tier1test/utlstringtest.cpp
[155/167] Compiling unittests/tier1test/tier1test.cpp
[156/167] Compiling unittests/tier2test/tier2test.cpp
[157/167] Compiling unittests/tier3test/tier3test.cpp
[158/167] Compiling unittests/mathlibtest/mathlib_test.cpp
[159/167] Compiling unittests/mathlibtest/mathlib_performance_test.cpp
[160/167] Linking build/vpklib/libvpklib.a
[161/167] Compiling utils/unittest/unittest.cpp
[162/167] Linking build/unittests/tier1test/libtier1test.so
[163/167] Linking build/unittests/tier2test/libtier2test.so
[164/167] Linking build/unittests/tier3test/libtier3test.so
[165/167] Linking build/unittests/mathlibtest/libmathlibtest.so
[166/167] Linking build/utils/unittest/unittest
[167/167] Linking build/filesystem/libfilesystem_stdio.so
/usr/bin/ld: public/zip_utils.cpp.8.o: in function `CWin32File::CreateTempFile(CUtlString&, CUtlString&)':
/home/jin/source-engine/build/../public/zip_utils.cpp:181:(.text._ZN10CWin32File14CreateTempFileER10CUtlStringS1_[_ZN10CWin32File14CreateTempFileER10CUtlStringS1_]+0x49): warning: the use of `tmpnam' is dangerous, better use `mkstemp'

Waf: Leaving directory `/home/jin/source-engine/build'
'build' finished successfully (12.849s)
jin@NWT0922PF41CJDP:~/source-engine$ python3 ./waf install
Waf: Entering directory `/home/jin/source-engine/build'
+ install /home/jin/source-engine/hl2/bin/libunitlib.so (from build/unitlib/libunitlib.so)
+ install /home/jin/source-engine/hl2/bin/libtier0.so (from build/tier0/libtier0.so)
+ install /home/jin/source-engine/hl2/bin/libvstdlib.so (from build/vstdlib/libvstdlib.so)
+ install /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so (from build/filesystem/libfilesystem_stdio.so)
+ install /home/jin/source-engine/hl2/tests/libtier0test.so (from build/unittests/tier0test/libtier0test.so)
+ install /home/jin/source-engine/hl2/tests/libtier1test.so (from build/unittests/tier1test/libtier1test.so)
+ install /home/jin/source-engine/hl2/tests/libtier2test.so (from build/unittests/tier2test/libtier2test.so)
+ install /home/jin/source-engine/hl2/tests/libtier3test.so (from build/unittests/tier3test/libtier3test.so)
+ install /home/jin/source-engine/hl2/tests/libmathlibtest.so (from build/unittests/mathlibtest/libmathlibtest.so)
+ install /home/jin/source-engine/hl2/unittest (from build/utils/unittest/unittest)
Waf: Leaving directory `/home/jin/source-engine/build'
'install' finished successfully (0.131s)
```

```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./unittest
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
LoadLibrary: pModule: tests/libtier3test.so, path: /home/jin/source-engine/hl2/tests/libtier3test.so
LoadLibrary: pModule: tests/libtier0test.so, path: /home/jin/source-engine/hl2/tests/libtier0test.so
LoadLibrary: pModule: tests/libtier2test.so, path: /home/jin/source-engine/hl2/tests/libtier2test.so
LoadLibrary: pModule: tests/libmathlibtest.so, path: /home/jin/source-engine/hl2/tests/libmathlibtest.so
LoadLibrary: pModule: tests/libtier1test.so, path: /home/jin/source-engine/hl2/tests/libtier1test.so
Valve Software - unittest (Jul  6 2026)
Starting test TSListTestSuite....

Testing 2 threads:
CTSList test: single thread push/pop, in order... pass
CTSList test: single thread push/pop, interleaved... pass
CTSList test: sequential push, multithread pop, no affinity...pass
CTSList test: single thread push, multithread pop, no affinity...pass
CTSList test: multithread push, sequential pop, no affinity...pass
CTSList test: multithread push, single thread pop, no affinity...pass
CTSList test: multithread push, multithread pop, no affinity...pass
CTSList test: multithread interleaved push/pop, no affinity...pass
CTSList test: sequential push, multithread pop, distributed...pass
CTSList test: single thread push, multithread pop, distributed...pass
CTSList test: multithread push, sequential pop, distributed...pass
CTSList test: multithread push, single thread pop, distributed...pass
CTSList test: multithread push, multithread pop, distributed...pass
CTSList test: multithread interleaved push/pop, distributed...pass

Testing 4 threads:
CTSList test: single thread push/pop, in order... pass
CTSList test: single thread push/pop, interleaved... pass
CTSList test: sequential push, multithread pop, no affinity...pass
CTSList test: single thread push, multithread pop, no affinity...pass
CTSList test: multithread push, sequential pop, no affinity...pass
CTSList test: multithread push, single thread pop, no affinity...pass
CTSList test: multithread push, multithread pop, no affinity...pass
CTSList test: multithread interleaved push/pop, no affinity...pass
CTSList test: sequential push, multithread pop, distributed...pass
CTSList test: single thread push, multithread pop, distributed...pass
CTSList test: multithread push, sequential pop, distributed...pass
CTSList test: multithread push, single thread pop, distributed...pass
CTSList test: multithread push, multithread pop, distributed...pass
CTSList test: multithread interleaved push/pop, distributed...pass

Testing 8 threads:
CTSList test: single thread push/pop, in order... pass
CTSList test: single thread push/pop, interleaved... pass
CTSList test: sequential push, multithread pop, no affinity...pass
CTSList test: single thread push, multithread pop, no affinity...pass
CTSList test: multithread push, sequential pop, no affinity...pass
CTSList test: multithread push, single thread pop, no affinity...pass
CTSList test: multithread push, multithread pop, no affinity...pass
CTSList test: multithread interleaved push/pop, no affinity...pass
CTSList test: sequential push, multithread pop, distributed...pass
CTSList test: single thread push, multithread pop, distributed...pass
CTSList test: multithread push, sequential pop, distributed...pass
CTSList test: multithread push, single thread pop, distributed...pass
CTSList test: multithread push, multithread pop, distributed...pass
CTSList test: multithread interleaved push/pop, distributed...pass

Testing 16 threads:
CTSList test: single thread push/pop, in order... pass
CTSList test: single thread push/pop, interleaved... pass
CTSList test: sequential push, multithread pop, no affinity...pass
CTSList test: single thread push, multithread pop, no affinity...pass
CTSList test: multithread push, sequential pop, no affinity...pass
CTSList test: multithread push, single thread pop, no affinity...pass
CTSList test: multithread push, multithread pop, no affinity...pass
CTSList test: multithread interleaved push/pop, no affinity...pass
CTSList test: sequential push, multithread pop, distributed...pass
CTSList test: single thread push, multithread pop, distributed...pass
CTSList test: multithread push, sequential pop, distributed...pass
CTSList test: multithread push, single thread pop, distributed...pass
CTSList test: multithread push, multithread pop, distributed...pass
CTSList test: multithread interleaved push/pop, distributed...pass
Tests done, purging test memory...done

Testing 2 threads:
CTSQueue test: single thread push/pop, in order... pass
CTSQueue test: single thread push/pop, interleaved... pass
CTSQueue test: sequential push, multithread pop, no affinity...pass
CTSQueue test: single thread push, multithread pop, no affinity...pass
CTSQueue test: multithread push, sequential pop, no affinity...pass
CTSQueue test: multithread push, single thread pop, no affinity...pass
CTSQueue test: multithread push, multithread pop, no affinity...pass
CTSQueue test: multithread interleaved push/pop, no affinity...pass
CTSQueue test: sequential push, multithread pop, distributed...pass
CTSQueue test: single thread push, multithread pop, distributed...pass
CTSQueue test: multithread push, sequential pop, distributed...pass
CTSQueue test: multithread push, single thread pop, distributed...pass
CTSQueue test: multithread push, multithread pop, distributed...pass
CTSQueue test: multithread interleaved push/pop, distributed...pass

Testing 4 threads:
CTSQueue test: single thread push/pop, in order... pass
CTSQueue test: single thread push/pop, interleaved... pass
CTSQueue test: sequential push, multithread pop, no affinity...pass
CTSQueue test: single thread push, multithread pop, no affinity...pass
CTSQueue test: multithread push, sequential pop, no affinity...pass
CTSQueue test: multithread push, single thread pop, no affinity...pass
CTSQueue test: multithread push, multithread pop, no affinity...pass
CTSQueue test: multithread interleaved push/pop, no affinity...pass
CTSQueue test: sequential push, multithread pop, distributed...pass
CTSQueue test: single thread push, multithread pop, distributed...pass
CTSQueue test: multithread push, sequential pop, distributed...pass
CTSQueue test: multithread push, single thread pop, distributed...pass
CTSQueue test: multithread push, multithread pop, distributed...pass
CTSQueue test: multithread interleaved push/pop, distributed...pass

Testing 8 threads:
CTSQueue test: single thread push/pop, in order... pass
CTSQueue test: single thread push/pop, interleaved... pass
CTSQueue test: sequential push, multithread pop, no affinity...pass
CTSQueue test: single thread push, multithread pop, no affinity...pass
CTSQueue test: multithread push, sequential pop, no affinity...pass
CTSQueue test: multithread push, single thread pop, no affinity...pass
CTSQueue test: multithread push, multithread pop, no affinity...pass
CTSQueue test: multithread interleaved push/pop, no affinity...pass
CTSQueue test: sequential push, multithread pop, distributed...pass
CTSQueue test: single thread push, multithread pop, distributed...pass
CTSQueue test: multithread push, sequential pop, distributed...pass
CTSQueue test: multithread push, single thread pop, distributed...pass
CTSQueue test: multithread push, multithread pop, distributed...pass
CTSQueue test: multithread interleaved push/pop, distributed...pass

Testing 16 threads:
CTSQueue test: single thread push/pop, in order... pass
CTSQueue test: single thread push/pop, interleaved... pass
CTSQueue test: sequential push, multithread pop, no affinity...pass
CTSQueue test: single thread push, multithread pop, no affinity...pass
CTSQueue test: multithread push, sequential pop, no affinity...pass
CTSQueue test: multithread push, single thread pop, no affinity...pass
CTSQueue test: multithread push, multithread pop, no affinity...pass
CTSQueue test: multithread interleaved push/pop, no affinity...pass
CTSQueue test: sequential push, multithread pop, distributed...pass
CTSQueue test: single thread push, multithread pop, distributed...pass
CTSQueue test: multithread push, sequential pop, distributed...pass
CTSQueue test: multithread push, single thread pop, distributed...pass
CTSQueue test: multithread push, multithread pop, distributed...pass
CTSQueue test: multithread interleaved push/pop, distributed...pass
Tests done, purging test memory...done
Starting test MathlibTestSuite....
cos Cycles: 26228184
cos sum - 2073697.500000
ssecos Cycles: 25937792
ssecos sum - 2024880.750000
Starting test CommandBufferTestSuite....
Simple command buffer test...
Delayed command buffer test...
Nested command buffer test...
Command buffer overflow test...
Starting test UtlStringTestSuite....
Running CUtlString tests
Starting test LZSSSafeUncompressTestSuite....
Running CLZSS::SafeUncompress tests
```