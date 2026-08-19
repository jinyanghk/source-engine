

```sh
sudo apt install qtbase5-dev
```

https://github.com/StrataSource/hammer-qt-dialogs/wiki/Material-Preview

```cpp
QImage *GetMaterialPreview( const QString &material )
{
	// Look for the requested material to preview
	IMaterial *pMaterial = g_pMaterialSystem->FindMaterial( material.c_str(), TEXTURE_GROUP_OTHER );
	if ( !pMaterial )
		return;
	
	int width = 0;
	int height = 0;

	// Get dimensions of the image to query required memory
	if ( pMaterial->GetPreviewImageProperties( &width, &height, nullptr, nullptr ) != MATERIAL_PREVIEW_IMAGE_OK )
		return nullptr;

	// Query required memory
	int bytes = ImageLoader::GetMemRequired( width, height, IMAGE_FORMAT_RGB888, false );

	unsigned char *pData = new unsigned char[bytes];
	if ( pMaterial->GetPreviewImage( pData, width, height, IMAGE_FORMAT_RGB888 ) != MATERIAL_PREVIEW_IMAGE_OK )
		return nullptr;

	// Construct the QImage
	QImage *pPreview = new QImage( QImage::fromData( pData, bytes, QImage::Format::Format_RGB888 ) );

	// Delete the now unused data
	delete[] pData;

	return preview;
}
```

https://github.com/StrataSource/hammer-qt-dialogs/wiki/Engine-View

```cpp
class CEngineView : public QWindow
{
public:

	CEngineView()
	{
		g_pMaterialSystem->AddView( handle()->winId() );
		g_pMaterialSystem->SetMode( handle()->winId(), GLOBAL_MATSYS_CFG );
	}
	~CEngineView()
	{
		g_pMaterialSystem->RemoveView( handle()->winId() );
	}

	void focusInEvent( QFocusEvent *ev ) override
	{
		g_pInputSystem->AttachToWindow( handle()->winId() );
	}
	void focusOutEvent( QFocusEvent *ev ) override
	{
		g_pInputSystem->DetachFromWindow();
	}
	void paintEvent( QPaintEvent *event ) override
	{
		RenderFrame();
	}

	void RenderFrame()
	{
		g_pMaterialSystem->SetView( handle()->winId() );

		CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
		pRenderContext->Viewport( x(), y(), width(), height() );
		pRenderContext->ClearColor4ub( 0, 0, 0, 255 );
		pRenderContext->ClearBuffers( true, true, true );

		pRenderContext->MatrixMode( MATERIAL_VIEW );
		pRenderContext->PushMatrix();
		pRenderContext->LoadIdentity();

		pRenderContext->MatrixMode( MATERIAL_PROJECTION );
		pRenderContext->PushMatrix();
		pRenderContext->LoadIdentity();

		pRenderContext->PerspectiveX( fov, aspect, near, far );
		
		pRenderContext->BeginFrame( frameTime );

		pRenderContext->EndFrame();

		pRenderContext->MatrixMode( MATERIAL_VIEW );
		pRenderContext->PopMatrix();

		pRenderContext->MatrixMode( MATERIAL_PROJECTION );
		pRenderContext->PopMatrix();

		pRenderContext->SwapBuffers();
		pRenderContext.Release();
	}
};
```

```sh
sudo apt install qt6-base-dev
sudo apt install qt6-declarative-dev qml6-module-qtquick
```

## STYLE

This repo uses the patented VALVe vomit barf code formatting style.

Things to keep in mind:
* Hungarian notation for pointers (pFuckBitch and m_pFuckShit for member ptrs)
* C prefix for classes
* No namespace indent 
* Newline before all braces
* Pascal case class names
* tier0/memdbgon.h must be the last include in a header!!!