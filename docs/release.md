
when build in release mode `-T release`

link fails for
* dmxconvert
* studiomdl

static lib issues
* dmserializers
* datamodel

```sh
importvmt.cpp:(.text+0x225): undefined reference to `bool CDmAttribute::IsTypeConvertable<CUtlString>() const'
/usr/bin/ld: importvmt.cpp:(.text+0x29a): undefined reference to `bool CDmAttribute::IsTypeConvertable<Color>() const'
/usr/bin/ld: importvmt.cpp:(.text+0x322): undefined reference to `bool CDmAttribute::IsTypeConvertable<Vector2D>() const'
/usr/bin/ld: importvmt.cpp:(.text+0x3ba): undefined reference to `bool CDmAttribute::IsTypeConvertable<Vector>() const'
/usr/bin/ld: importvmt.cpp:(.text+0x462): undefined reference to `bool CDmAttribute::IsTypeConvertable<Vector4D>() const'
/usr/bin/ld: importvmt.cpp:(.text+0x522): undefined reference to `bool CDmAttribute::IsTypeConvertable<VMatrix>() const'
/usr/bin/ld: importvmt.cpp:(.text+0xa1e): undefined reference to `void CDmAttribute::CopyDataOut<Vector>(Vector&) const'
/usr/bin/ld: importvmt.cpp:(.text+0xa6a): undefined reference to `void CDmAttribute::CopyDataOut<CUtlString>(CUtlString&) const'
/usr/bin/ld: importvmt.cpp:(.text+0xace): undefined reference to `void CDmAttribute::CopyDataOut<Vector4D>(Vector4D&) const'
/usr/bin/ld: importvmt.cpp:(.text+0xaee): undefined reference to `void CDmAttribute::CopyDataOut<Vector2D>(Vector2D&) const'
/usr/bin/ld: importvmt.cpp:(.text+0xb19): undefined reference to `void CDmAttribute::CopyDataOut<Color>(Color&) const'
/usr/bin/ld: importvmt.cpp:(.text+0xb41): undefined reference to `void CDmAttribute::CopyDataOut<VMatrix>(VMatrix&) const'
collect2: error: ld returned 1 exit status
```


```sh
cd build

nm -C datamodel/libdatamodel.a | grep CopyDataOut
0000000000000db0 T void CDmAttribute::CopyDataOut<Quaternion>(Quaternion&) const
0000000000000cd0 T void CDmAttribute::CopyDataOut<QAngle>(QAngle&) const
0000000000000a80 T void CDmAttribute::CopyDataOut<bool>(bool&) const
0000000000000bd0 T void CDmAttribute::CopyDataOut<float>(float&) const
0000000000000b30 T void CDmAttribute::CopyDataOut<int>(int&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<bool>(bool&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
```

in debug mode

```sh
cd build

nm -C datamodel/libdatamodel.a | grep CopyDataOut
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlString>(CUtlString&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<CUtlString, CUtlMemory<CUtlString, int> > >(CUtlVector<CUtlString, CUtlMemory<CUtlString, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<Quaternion, CUtlMemory<Quaternion, int> > >(CUtlVector<Quaternion, CUtlMemory<Quaternion, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<UniqueId_t, CUtlMemory<UniqueId_t, int> > >(CUtlVector<UniqueId_t, CUtlMemory<UniqueId_t, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<CUtlBinaryBlock, CUtlMemory<CUtlBinaryBlock, int> > >(CUtlVector<CUtlBinaryBlock, CUtlMemory<CUtlBinaryBlock, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<DmElementHandle_t, CUtlMemory<DmElementHandle_t, int> > >(CUtlVector<DmElementHandle_t, CUtlMemory<DmElementHandle_t, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<Color, CUtlMemory<Color, int> > >(CUtlVector<Color, CUtlMemory<Color, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<QAngle, CUtlMemory<QAngle, int> > >(CUtlVector<QAngle, CUtlMemory<QAngle, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<Vector, CUtlMemory<Vector, int> > >(CUtlVector<Vector, CUtlMemory<Vector, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<VMatrix, CUtlMemory<VMatrix, int> > >(CUtlVector<VMatrix, CUtlMemory<VMatrix, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<Vector2D, CUtlMemory<Vector2D, int> > >(CUtlVector<Vector2D, CUtlMemory<Vector2D, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<Vector4D, CUtlMemory<Vector4D, int> > >(CUtlVector<Vector4D, CUtlMemory<Vector4D, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<bool, CUtlMemory<bool, int> > >(CUtlVector<bool, CUtlMemory<bool, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<float, CUtlMemory<float, int> > >(CUtlVector<float, CUtlMemory<float, int> >&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlVector<int, CUtlMemory<int, int> > >(CUtlVector<int, CUtlMemory<int, int> >&) const
0000000000001838 T void CDmAttribute::CopyDataOut<Quaternion>(Quaternion&) const
0000000000000000 W void CDmAttribute::CopyDataOut<UniqueId_t>(UniqueId_t&) const
0000000000000000 W void CDmAttribute::CopyDataOut<CUtlBinaryBlock>(CUtlBinaryBlock&) const
0000000000000000 W void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
0000000000000000 W void CDmAttribute::CopyDataOut<DmUnknownAttribute_t>(DmUnknownAttribute_t&) const
0000000000000000 W void CDmAttribute::CopyDataOut<Color>(Color&) const
000000000000173e T void CDmAttribute::CopyDataOut<QAngle>(QAngle&) const
0000000000000000 W void CDmAttribute::CopyDataOut<Vector>(Vector&) const
0000000000000000 W void CDmAttribute::CopyDataOut<VMatrix>(VMatrix&) const
0000000000000000 W void CDmAttribute::CopyDataOut<Vector2D>(Vector2D&) const
0000000000000000 W void CDmAttribute::CopyDataOut<Vector4D>(Vector4D&) const
00000000000013ee T void CDmAttribute::CopyDataOut<bool>(bool&) const
0000000000001616 T void CDmAttribute::CopyDataOut<float>(float&) const
00000000000014fc T void CDmAttribute::CopyDataOut<int>(int&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<bool>(bool&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
                 U void CDmAttribute::CopyDataOut<DmElementHandle_t>(DmElementHandle_t&) const
```

workaround for gcc:

```sh
git diff public/datamodel/dmattribute.h
diff --git a/public/datamodel/dmattribute.h b/public/datamodel/dmattribute.h
index e383eabf..6348c1d6 100644
--- a/public/datamodel/dmattribute.h
+++ b/public/datamodel/dmattribute.h
@@ -204,10 +204,10 @@ private:
        // Called by elements after unserialization of their attributes is complete
        void OnUnserializationFinished();

-       template< class T > bool IsTypeConvertable() const;
+       template< class T > bool IsTypeConvertable() const __attribute__((used));
        template< class T > bool ShouldModify( const T& src );
        template< class T > void CopyData( const T& src );
-       template< class T > void CopyDataOut( T& dest ) const;
+       template< class T > void CopyDataOut( T& dest ) const __attribute__((used));

 private:
        CDmAttribute *m_pNext;
```