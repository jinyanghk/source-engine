
### vpk

```sh
mkdir -p build/utils/vpk
```

```sh
g++ utils/vpk/packtest.cpp -MMD -U_FORTIFY_SOURCE -fno-strict-aliasing -fvisibility=hidden -g -O0 -pipe -fPIC -pthread -march=core2 -mfpmath=sse -std=c++11 -fpermissive -w -DPLATFORM_64BITS=1 -DLINUX=1 -D_LINUX=1 -DPOSIX=1 -D_POSIX=1 -DPLATFORM_POSIX=1 -DGNUC=1 -DCOMPILER_GCC=1 -DTIER1_STATIC_LIB=1 -Ipublic -Ipublic/tier0 -Iutils/common -Lbuild/tier0 -Lbuild/tier1 -Lbuild/tier2 -Lbuild/bitmap -Lbuild/mathlib -Lbuild/vstdlib -Lbuild/vpklib -ltier2 -ltier1 -ltier0 -lbitmap -lmathlib -lvstdlib -lvpklib -o build/utils/vpk/vpk
```

#### openssl

openssl version does not work, there're a few undefined variables not found anywhere

```sh
$ g++ utils/vpk/packtest.cpp -MMD -U_FORTIFY_SOURCE -fno-strict-aliasing -fvisibility=hidden -g -O0 -pipe -fPIC -pthread -march=core2 -mfpmath=sse -std=c++11 -fpermissive -w -DPLATFORM_64BITS=1 -DLINUX=1 -D_LINUX=1 -DPOSIX=1 -D_POSIX=1 -DPLATFORM_POSIX=1 -DGNUC=1 -DCOMPILER_GCC=1 -DTIER1_STATIC_LIB=1 -DVPK_ENABLE_SIGNING=1 -Ipublic -Ipublic/tier0 -Iutils/common -Lbuild/tier0 -Lbuild/tier1 -Lbuild/tier2 -Lbuild/bitmap -Lbuild/mathlib -Lbuild/vstdlib -Lbuild/vpklib -ltier2 -ltier1 -ltier0 -lbitmap -lmathlib -lvstdlib -lvpklib -o build/utils/vpk/vpk
utils/vpk/packtest.cpp: In function ‘void LoadKeyFile(const char*, const char*, CUtlVector<unsigned char>&)’:
utils/vpk/packtest.cpp:228:31: error: ‘k_nRSAKeyLenMax’ was not declared in this scope
  228 |         uint8 rgubDecodedData[k_nRSAKeyLenMax*2];
      |                               ^~~~~~~~~~~~~~~
In file included from public/tier0/basetypes.h:11,
                 from public/tier0/platform.h:49,
                 from utils/vpk/packtest.cpp:9:
utils/vpk/packtest.cpp:229:44: error: ‘rgubDecodedData’ was not declared in this scope; did you mean ‘cubDecodedData’?
  229 |         uint cubDecodedData = Q_ARRAYSIZE( rgubDecodedData );
      |                                            ^~~~~~~~~~~~~~~
public/tier0/commonmacros.h:61:37: note: in definition of macro ‘RTL_NUMBER_OF_V1’
   61 | #define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))
      |                                     ^
public/tier0/commonmacros.h:152:25: note: in expansion of macro ‘RTL_NUMBER_OF_V2’
  152 | #define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
      |                         ^~~~~~~~~~~~~~~~
public/tier0/commonmacros.h:155:33: note: in expansion of macro ‘ARRAYSIZE’
  155 | #define Q_ARRAYSIZE(p)          ARRAYSIZE(p)
      |                                 ^~~~~~~~~
utils/vpk/packtest.cpp:229:31: note: in expansion of macro ‘Q_ARRAYSIZE’
  229 |         uint cubDecodedData = Q_ARRAYSIZE( rgubDecodedData );
      |                               ^~~~~~~~~~~
utils/vpk/packtest.cpp:230:14: error: ‘CCrypto’ has not been declared
  230 |         if( !CCrypto::HexDecode( pszEncodedBytes, rgubDecodedData, &cubDecodedData ) || cubDecodedData <= 0 )
      |              ^~~~~~~
utils/vpk/packtest.cpp: In function ‘void GenerateKeyPair(const char*)’:
utils/vpk/packtest.cpp:1469:29: error: ‘k_nRSAKeyLenMax’ was not declared in this scope
 1469 |         uint8 rgubPublicKey[k_nRSAKeyLenMax]={0};
      |                             ^~~~~~~~~~~~~~~
utils/vpk/packtest.cpp:1470:42: error: ‘rgubPublicKey’ was not declared in this scope; did you mean ‘cubPublicKey’?
 1470 |         uint cubPublicKey = Q_ARRAYSIZE( rgubPublicKey );
      |                                          ^~~~~~~~~~~~~
public/tier0/commonmacros.h:61:37: note: in definition of macro ‘RTL_NUMBER_OF_V1’
   61 | #define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))
      |                                     ^
public/tier0/commonmacros.h:152:25: note: in expansion of macro ‘RTL_NUMBER_OF_V2’
  152 | #define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
      |                         ^~~~~~~~~~~~~~~~
public/tier0/commonmacros.h:155:33: note: in expansion of macro ‘ARRAYSIZE’
  155 | #define Q_ARRAYSIZE(p)          ARRAYSIZE(p)
      |                                 ^~~~~~~~~
utils/vpk/packtest.cpp:1470:29: note: in expansion of macro ‘Q_ARRAYSIZE’
 1470 |         uint cubPublicKey = Q_ARRAYSIZE( rgubPublicKey );
      |                             ^~~~~~~~~~~
utils/vpk/packtest.cpp:1473:43: error: ‘rgubPrivateKey’ was not declared in this scope; did you mean ‘cubPrivateKey’?
 1473 |         uint cubPrivateKey = Q_ARRAYSIZE( rgubPrivateKey );
      |                                           ^~~~~~~~~~~~~~
public/tier0/commonmacros.h:61:37: note: in definition of macro ‘RTL_NUMBER_OF_V1’
   61 | #define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))
      |                                     ^
public/tier0/commonmacros.h:152:25: note: in expansion of macro ‘RTL_NUMBER_OF_V2’
  152 | #define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
      |                         ^~~~~~~~~~~~~~~~
public/tier0/commonmacros.h:155:33: note: in expansion of macro ‘ARRAYSIZE’
  155 | #define Q_ARRAYSIZE(p)          ARRAYSIZE(p)
      |                                 ^~~~~~~~~
utils/vpk/packtest.cpp:1473:30: note: in expansion of macro ‘Q_ARRAYSIZE’
 1473 |         uint cubPrivateKey = Q_ARRAYSIZE( rgubPrivateKey );
      |                              ^~~~~~~~~~~
utils/vpk/packtest.cpp:1475:14: error: ‘CCrypto’ has not been declared
 1475 |         if( !CCrypto::RSAGenerateKeys( rgubPublicKey, &cubPublicKey, rgubPrivateKey, &cubPrivateKey ) )
      |              ^~~~~~~
utils/vpk/packtest.cpp:1481:49: error: ‘rgchEncodedPublicKey’ was not declared in this scope; did you mean ‘cubEncodedPublicKey’?
 1481 |         uint cubEncodedPublicKey = Q_ARRAYSIZE( rgchEncodedPublicKey );
      |                                                 ^~~~~~~~~~~~~~~~~~~~
public/tier0/commonmacros.h:61:37: note: in definition of macro ‘RTL_NUMBER_OF_V1’
   61 | #define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))
      |                                     ^
public/tier0/commonmacros.h:152:25: note: in expansion of macro ‘RTL_NUMBER_OF_V2’
  152 | #define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
      |                         ^~~~~~~~~~~~~~~~
public/tier0/commonmacros.h:155:33: note: in expansion of macro ‘ARRAYSIZE’
  155 | #define Q_ARRAYSIZE(p)          ARRAYSIZE(p)
      |                                 ^~~~~~~~~
utils/vpk/packtest.cpp:1481:36: note: in expansion of macro ‘Q_ARRAYSIZE’
 1481 |         uint cubEncodedPublicKey = Q_ARRAYSIZE( rgchEncodedPublicKey );
      |                                    ^~~~~~~~~~~
utils/vpk/packtest.cpp:1483:14: error: ‘CCrypto’ has not been declared
 1483 |         if( !CCrypto::HexEncode( rgubPublicKey, cubPublicKey, rgchEncodedPublicKey, cubEncodedPublicKey ) )
      |              ^~~~~~~
utils/vpk/packtest.cpp:1499:14: error: ‘CCrypto’ has not been declared
 1499 |         if( !CCrypto::HexEncode( rgubPrivateKey, cubPrivateKey, rgchEncodedEncryptedPrivateKey, Q_ARRAYSIZE(rgchEncodedEncryptedPrivateKey) ) )
      |              ^~~~~~~
utils/vpk/packtest.cpp:1499:65: error: ‘rgchEncodedEncryptedPrivateKey’ was not declared in this scope
 1499 |         if( !CCrypto::HexEncode( rgubPrivateKey, cubPrivateKey, rgchEncodedEncryptedPrivateKey, Q_ARRAYSIZE(rgchEncodedEncryptedPrivateKey) ) )
      |                                                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
utils/vpk/packtest.cpp:1561:25: error: ‘rgchEncodedEncryptedPrivateKey’ was not declared in this scope
 1561 |                         rgchEncodedEncryptedPrivateKey, rgchEncodedPublicKey );
```