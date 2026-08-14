

```cpp
CMaterial::~CMaterial()
{
	MaterialSystem()->UnbindMaterial( this );

	Uncache();

	if ( m_RefCount != 0 )
	{
		DevWarning( 2, "Reference Count for Material %s (%d) != 0\n", GetName(), (int) m_RefCount );
	}

	if ( m_pVMTKeyValues )
	{
		m_pVMTKeyValues->deleteThis();
		m_pVMTKeyValues = NULL;
	}

	DestroyRenderPassList( m_ShaderRenderState.m_pSnapshots ); 

	m_representativeTexture = NULL;

#if defined( _DEBUG )
	delete [] m_pDebugName;
#endif

	// Deliberately stomp our VTable so that we can detect cases where code tries to access freed materials.
	int *p = (int *)this;
	*p = 0xc0dedbad;
}
```


```cpp
CTexture::~CTexture()
{
#ifdef _DEBUG
	if ( m_nRefCount != 0 )
	{
		Warning( "Reference Count(%d) != 0 in ~CTexture for texture \"%s\"\n", (int)m_nRefCount, m_Name.String() );
	}
#endif

	Shutdown();

#ifdef _DEBUG
	if ( m_pDebugName )
	{
		// delete[] m_pDebugName;
	}
#endif

	// Deliberately stomp our VTable so that we can detect cases where code tries to access freed materials.
	int *p = (int *)this;
	*p = 0xdeadbeef;
}
```