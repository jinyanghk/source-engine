
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

release mode hammer:

```sh
Core was generated by `./hammer'.
Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x0000756b55827afe in IDirect3DDevice9::SetRenderTarget(unsigned int, IDirect3DSurface9*) () from bin/libtogl.so
[Current thread is 1 (Thread 0x756b50397940 (LWP 84585))]
(gdb) bt
#0  0x0000756b55827afe in IDirect3DDevice9::SetRenderTarget(unsigned int, IDirect3DSurface9*) () from bin/libtogl.so
#1  0x0000756b4dd2af9f in CShaderAPIDx8::SetRenderTargetEx(int, long long, long long) ()
   from /home/jin/source-engine/hl2/bin/libshaderapidx9.so
#2  0x0000756b4fdf92a2 in CMatRenderContext::CommitRenderTargetAndViewport() ()
   from /home/jin/source-engine/hl2/bin/libmaterialsystem.so
#3  0x0000639355b7a9f7 in QModelView3::RenderEngineFrame() ()
#4  0x0000639355b7c7dc in QModelView3::paintEvent(QPaintEvent*) ()
#5  0x0000756b54bd46ff in QWidget::event(QEvent*) () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#6  0x0000756b54b8b3b0 in QApplicationPrivate::notify_helper(QObject*, QEvent*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#7  0x0000756b55338448 in QCoreApplication::notifyInternal2(QObject*, QEvent*) ()
   from /lib/x86_64-linux-gnu/libQt6Core.so.6
#8  0x0000756b54bc30ce in QWidgetPrivate::sendPaintEvent(QRegion const&) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#9  0x0000756b54bc4bbd in QWidgetPrivate::drawWidget(QPaintDevice*, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#10 0x0000756b54bc7109 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#11 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#12 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#13 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#14 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion con--Type <RET> for more, q to quit, c to continue without paging--
st&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#15 0x0000756b54bc482d in QWidgetPrivate::drawWidget(QPaintDevice*, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#16 0x0000756b54bc7109 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#17 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#18 0x0000756b54bc6f60 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#19 0x0000756b54bc482d in QWidgetPrivate::drawWidget(QPaintDevice*, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#20 0x0000756b54bc7109 in QWidgetPrivate::paintSiblingsRecursive(QPaintDevice*, QList<QObject*> const&, int, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#21 0x0000756b54bc482d in QWidgetPrivate::drawWidget(QPaintDevice*, QRegion const&, QPoint const&, QFlags<QWidgetPrivate::DrawWidgetFlag>, QPainter*, QWidgetRepaintManager*) () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#22 0x0000756b54be193f in QWidgetRepaintManager::paintAndFlush() () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#23 0x0000756b54bdd55a in QWidgetRepaintManager::sync(QWidget*, QRegion const&) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#24 0x0000756b54be5170 in ?? () from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#25 0x0000756b54b8b3b0 in QApplicationPrivate::notify_helper(QObject*, QEvent*) ()
   from /lib/x86_64-linux-gnu/libQt6Widgets.so.6
#26 0x0000756b55338448 in QCoreApplication::notifyInternal2(QObject*, QEvent*) ()
   from /lib/x86_64-linux-gnu/libQt6Core.so.6
#27 0x0000756b543a0ba7 in QGuiApplicationPrivate::processExposeEvent(QWindowSystemInterfacePrivate::ExposeEvent*) ()
--Type <RET> for more, q to quit, c to continue without paging--
   from /lib/x86_64-linux-gnu/libQt6Gui.so.6
#28 0x0000756b543e15cc in QWindowSystemInterface::sendWindowSystemEvents(QFlags<QEventLoop::ProcessEventsFlag>) ()
   from /lib/x86_64-linux-gnu/libQt6Gui.so.6
#29 0x0000756b547152b4 in ?? () from /lib/x86_64-linux-gnu/libQt6Gui.so.6
#30 0x0000756b53314585 in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#31 0x0000756b53373977 in ?? () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#32 0x0000756b53313a23 in g_main_context_iteration () from /lib/x86_64-linux-gnu/libglib-2.0.so.0
#33 0x0000756b555315ef in QEventDispatcherGlib::processEvents(QFlags<QEventLoop::ProcessEventsFlag>) ()
   from /lib/x86_64-linux-gnu/libQt6Core.so.6
#34 0x0000756b553429a3 in QEventLoop::exec(QFlags<QEventLoop::ProcessEventsFlag>) ()
   from /lib/x86_64-linux-gnu/libQt6Core.so.6
#35 0x0000756b5533b35e in QCoreApplication::exec() () from /lib/x86_64-linux-gnu/libQt6Core.so.6
#36 0x0000639355b84e7f in CHammerApp::Main() ()
#37 0x0000639355b92627 in CAppSystemGroup::Run() ()
#38 0x0000756b5382a1ca in __libc_start_call_main (main=main@entry=0x639355b4a780 <main>, argc=argc@entry=1,
    argv=argv@entry=0x7fffeb279d68) at ../sysdeps/nptl/libc_start_call_main.h:58
#39 0x0000756b5382a28b in __libc_start_main_impl (main=0x639355b4a780 <main>, argc=1, argv=0x7fffeb279d68,
    init=<optimized out>, fini=<optimized out>, rtld_fini=<optimized out>, stack_end=0x7fffeb279d58)
    at ../csu/libc-start.c:360
#40 0x0000639355b4ad55 in _start ()
```