//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#include "utlvector.h"
#include "materialsystem/imaterialsystem.h"
#include "shadersystem.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imesh.h"
#include "tier0/dbg.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "meshvk.h"
#include "shaderapidevicevk.h"
#include "shaderapivk.h"
#include "shadershadowvk.h"

//-----------------------------------------------------------------------------
//
// Shader API GL
//
//-----------------------------------------------------------------------------

CShaderAPIVK::CShaderAPIVK()  : m_Mesh( false )
{
}

CShaderAPIVK::~CShaderAPIVK()
{
}


bool CShaderAPIVK::DoRenderTargetsNeedSeparateDepthBuffer() const
{
	return false;
}

// Can we download textures?
bool CShaderAPIVK::CanDownloadTextures() const
{
	return false;
}

// Used to clear the transition table when we know it's become invalid.
void CShaderAPIVK::ClearSnapshots()
{
}

// Members of IMaterialSystemHardwareConfig
bool CShaderAPIVK::HasDestAlphaBuffer() const
{
	return false;
}

bool CShaderAPIVK::HasStencilBuffer() const
{
	return false;
}

int CShaderAPIVK::MaxViewports() const
{
	return 1;
}

int CShaderAPIVK::GetShadowFilterMode() const
{
	return 0;
}

int CShaderAPIVK::StencilBufferBits() const
{
	return 0;
}

int	 CShaderAPIVK::GetFrameBufferColorDepth() const
{
	return 0;
}

int  CShaderAPIVK::GetSamplerCount() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 60))
		return 1;
	if (( ShaderUtil()->GetConfig().dxSupportLevel >= 60 ) && ( ShaderUtil()->GetConfig().dxSupportLevel < 80 ))
		return 2;
	return 4;
}

bool CShaderAPIVK::HasSetDeviceGammaRamp() const
{
	return false;
}

bool CShaderAPIVK::SupportsCompressedTextures() const
{
	return false;
}

VertexCompressionType_t CShaderAPIVK::SupportsCompressedVertices() const
{
	return VERTEX_COMPRESSION_NONE;
}

bool CShaderAPIVK::SupportsVertexAndPixelShaders() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 80))
		return false;

	return true;
}

bool CShaderAPIVK::SupportsPixelShaders_1_4() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 81))
		return false;

	return true;
}

bool CShaderAPIVK::SupportsPixelShaders_2_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

bool CShaderAPIVK::SupportsPixelShaders_2_b() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

bool CShaderAPIVK::ActuallySupportsPixelShaders_2_b() const
{
	return true;
}

bool CShaderAPIVK::SupportsShaderModel_3_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
		(ShaderUtil()->GetConfig().dxSupportLevel < 95))
		return false;

	return true;
}

bool CShaderAPIVK::SupportsStaticControlFlow() const
{
	if ( IsOpenGL() )
		return false;

	return SupportsVertexShaders_2_0();
}

bool CShaderAPIVK::SupportsVertexShaders_2_0() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 90))
		return false;

	return true;
}

int  CShaderAPIVK::MaximumAnisotropicLevel() const
{
	return 0;
}

void CShaderAPIVK::SetAnisotropicLevel( int nAnisotropyLevel )
{
}

int  CShaderAPIVK::MaxTextureWidth() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int  CShaderAPIVK::MaxTextureHeight() const
{
	// Should be big enough to cover all cases
	return 16384;
}

int  CShaderAPIVK::MaxTextureAspectRatio() const
{
	// Should be big enough to cover all cases
	return 16384;
}


int	 CShaderAPIVK::TextureMemorySize() const
{
	// fake it
	return 64 * 1024 * 1024;
}

int  CShaderAPIVK::GetDXSupportLevel() const 
{ 
	return 90; 
}

bool CShaderAPIVK::SupportsOverbright() const
{
	return false;
}

bool CShaderAPIVK::SupportsCubeMaps() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

bool CShaderAPIVK::SupportsNonPow2Textures() const
{
	return true;
}

bool CShaderAPIVK::SupportsMipmappedCubemaps() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

int  CShaderAPIVK::GetTextureStageCount() const
{
	return 4;
}

int	 CShaderAPIVK::NumVertexShaderConstants() const
{
	return 128;
}

int	 CShaderAPIVK::NumBooleanVertexShaderConstants() const
{
	return 0;
}

int	 CShaderAPIVK::NumIntegerVertexShaderConstants() const
{
	return 0;
}

int	 CShaderAPIVK::NumPixelShaderConstants() const
{
	return 8;
}

int	 CShaderAPIVK::MaxNumLights() const
{
	return 4;
}

bool CShaderAPIVK::SupportsSpheremapping() const
{
	return false;
}


// This is the max dx support level supported by the card
int	CShaderAPIVK::GetMaxDXSupportLevel() const
{
	return 90;
}

bool CShaderAPIVK::SupportsHardwareLighting() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
		return false;

	return true;
}

int	 CShaderAPIVK::MaxBlendMatrices() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
	{
		return 1;
	}

	return 0;
}

int	 CShaderAPIVK::MaxBlendMatrixIndices() const
{
	if ((ShaderUtil()->GetConfig().dxSupportLevel > 0) &&
	    (ShaderUtil()->GetConfig().dxSupportLevel < 70))
	{
		return 1;
	}

	return 0;
}

int	 CShaderAPIVK::MaxVertexShaderBlendMatrices() const
{
	return 0;
}

int	CShaderAPIVK::MaxUserClipPlanes() const
{
	return 0;
}

bool CShaderAPIVK::SpecifiesFogColorInLinearSpace() const
{
	return false;
}

bool CShaderAPIVK::SupportsSRGB() const
{
	return false;
}

bool CShaderAPIVK::FakeSRGBWrite() const
{
	return false;
}

bool CShaderAPIVK::CanDoSRGBReadFromRTs() const
{
	return true;
}

bool CShaderAPIVK::SupportsGLMixedSizeTargets() const
{
	return false;
}

const char *CShaderAPIVK::GetHWSpecificShaderDLLName() const
{
	return 0;
}

// Sets the default *dynamic* state
void CShaderAPIVK::SetDefaultState()
{
}


// Returns the snapshot id for the shader state
StateSnapshot_t	 CShaderAPIVK::TakeSnapshot( )
{
	StateSnapshot_t id = 0;
	if (g_ShaderShadow.m_IsTranslucent)
		id |= TRANSLUCENT;
	if (g_ShaderShadow.m_IsAlphaTested)
		id |= ALPHATESTED;
	if (g_ShaderShadow.m_bUsesVertexAndPixelShaders)
		id |= VERTEX_AND_PIXEL_SHADERS;
	if (g_ShaderShadow.m_bIsDepthWriteEnabled)
		id |= DEPTHWRITE;
	return id;
}

// Returns true if the state snapshot is transparent
bool CShaderAPIVK::IsTranslucent( StateSnapshot_t id ) const
{
	return (id & TRANSLUCENT) != 0; 
}

bool CShaderAPIVK::IsAlphaTested( StateSnapshot_t id ) const
{
	return (id & ALPHATESTED) != 0; 
}

bool CShaderAPIVK::IsDepthWriteEnabled( StateSnapshot_t id ) const
{
	return (id & DEPTHWRITE) != 0; 
}

bool CShaderAPIVK::UsesVertexAndPixelShaders( StateSnapshot_t id ) const
{
	return (id & VERTEX_AND_PIXEL_SHADERS) != 0; 
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIVK::ComputeVertexFormat( int numSnapshots, StateSnapshot_t* pIds ) const
{
	return 0;
}

// Gets the vertex format for a set of snapshot ids
VertexFormat_t CShaderAPIVK::ComputeVertexUsage( int numSnapshots, StateSnapshot_t* pIds ) const
{
	return 0;
}

// Uses a state snapshot
void CShaderAPIVK::UseSnapshot( StateSnapshot_t snapshot )
{
}

// Sets the color to modulate by
void CShaderAPIVK::Color3f( float r, float g, float b )
{
}

void CShaderAPIVK::Color3fv( float const* pColor )
{
}

void CShaderAPIVK::Color4f( float r, float g, float b, float a )
{
}

void CShaderAPIVK::Color4fv( float const* pColor )
{
}

// Faster versions of color
void CShaderAPIVK::Color3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVK::Color3ubv( unsigned char const* rgb )
{
}

void CShaderAPIVK::Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
}

void CShaderAPIVK::Color4ubv( unsigned char const* rgba )
{
}

// The shade mode
void CShaderAPIVK::ShadeMode( ShaderShadeMode_t mode )
{
}

// Binds a particular material to render with
void CShaderAPIVK::Bind( IMaterial* pMaterial )
{
}

// Cull mode
void CShaderAPIVK::CullMode( MaterialCullMode_t cullMode )
{
}

void CShaderAPIVK::ForceDepthFuncEquals( bool bEnable )
{
}

// Forces Z buffering on or off
void CShaderAPIVK::OverrideDepthEnable( bool bEnable, bool bDepthEnable )
{
}

void CShaderAPIVK::OverrideAlphaWriteEnable( bool bOverrideEnable, bool bAlphaWriteEnable )
{
}

void CShaderAPIVK::OverrideColorWriteEnable( bool bOverrideEnable, bool bColorWriteEnable )
{
}

//legacy fast clipping linkage
void CShaderAPIVK::SetHeightClipZ( float z )
{
}

void CShaderAPIVK::SetHeightClipMode( enum MaterialHeightClipMode_t heightClipMode )
{
}

// Sets the lights
void CShaderAPIVK::SetLight( int lightNum, const LightDesc_t& desc )
{
}

// Sets lighting origin for the current model
void CShaderAPIVK::SetLightingOrigin( Vector vLightingOrigin )
{
}

void CShaderAPIVK::SetAmbientLight( float r, float g, float b )
{
}

void CShaderAPIVK::SetAmbientLightCube( Vector4D cube[6] )
{
}

// Get lights
int CShaderAPIVK::GetMaxLights( void ) const
{
	return 0;
}

const LightDesc_t& CShaderAPIVK::GetLight( int lightNum ) const
{
	static LightDesc_t blah;
	return blah;
}

// Render state for the ambient light cube (vertex shaders)
void CShaderAPIVK::SetVertexShaderStateAmbientLightCube()
{
}

void CShaderAPIVK::SetSkinningMatrices()
{
}

// Lightmap texture binding
void CShaderAPIVK::BindLightmap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindBumpLightmap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindFullbrightLightmap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindWhite( TextureStage_t stage )
{
}

void CShaderAPIVK::BindBlack( TextureStage_t stage )
{
}

void CShaderAPIVK::BindGrey( TextureStage_t stage )
{
}

// Gets the lightmap dimensions
void CShaderAPIVK::GetLightmapDimensions( int *w, int *h )
{
	g_pShaderUtil->GetLightmapDimensions( w, h );
}

// Special system flat normal map binding.
void CShaderAPIVK::BindFlatNormalMap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindSignedNormalizationCubeMap( TextureStage_t stage )
{
}

void CShaderAPIVK::BindFBTexture( TextureStage_t stage, int textureIndex )
{
}

// Flushes any primitives that are buffered
void CShaderAPIVK::FlushBufferedPrimitives()
{
}

// Gets the dynamic mesh; note that you've got to render the mesh
// before calling this function a second time. Clients should *not*
// call DestroyStaticMesh on the mesh returned by this call.
IMesh* CShaderAPIVK::GetDynamicMesh( IMaterial* pMaterial, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	return &m_Mesh;
}

IMesh* CShaderAPIVK::GetDynamicMeshEx( IMaterial* pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount, bool buffered, IMesh* pVertexOverride, IMesh* pIndexOverride )
{
	return &m_Mesh;
}

IMesh* CShaderAPIVK::GetFlexMesh()
{
	return &m_Mesh;
}

// Begins a rendering pass that uses a state snapshot
void CShaderAPIVK::BeginPass( StateSnapshot_t snapshot  )
{
}

// Renders a single pass of a material
void CShaderAPIVK::RenderPass( int nPass, int nPassCount )
{
}

// stuff related to matrix stacks
void CShaderAPIVK::MatrixMode( MaterialMatrixMode_t matrixMode )
{
}

void CShaderAPIVK::PushMatrix()
{
}

void CShaderAPIVK::PopMatrix()
{
}

void CShaderAPIVK::LoadMatrix( float *m )
{
}

void CShaderAPIVK::MultMatrix( float *m )
{
}

void CShaderAPIVK::MultMatrixLocal( float *m )
{
}

void CShaderAPIVK::GetMatrix( MaterialMatrixMode_t matrixMode, float *dst )
{
}

void CShaderAPIVK::LoadIdentity( void )
{
}

void CShaderAPIVK::LoadCameraToWorld( void )
{
}

void CShaderAPIVK::Ortho( double left, double top, double right, double bottom, double zNear, double zFar )
{
}

void CShaderAPIVK::PerspectiveX( double fovx, double aspect, double zNear, double zFar )
{
}

void CShaderAPIVK::PerspectiveOffCenterX( double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right )
{
}

void CShaderAPIVK::PickMatrix( int x, int y, int width, int height )
{
}

void CShaderAPIVK::Rotate( float angle, float x, float y, float z )
{
}

void CShaderAPIVK::Translate( float x, float y, float z )
{
}

void CShaderAPIVK::Scale( float x, float y, float z )
{
}

void CShaderAPIVK::ScaleXY( float x, float y )
{
}

// Fog methods...
void CShaderAPIVK::FogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIVK::FogStart( float fStart )
{
}

void CShaderAPIVK::FogEnd( float fEnd )
{
}

void CShaderAPIVK::SetFogZ( float fogZ )
{
}
	
void CShaderAPIVK::FogMaxDensity( float flMaxDensity )
{
}

void CShaderAPIVK::GetFogDistances( float *fStart, float *fEnd, float *fFogZ )
{
}


void CShaderAPIVK::SceneFogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}


void CShaderAPIVK::SceneFogMode( MaterialFogMode_t fogMode )
{
}

void CShaderAPIVK::GetSceneFogColor( unsigned char *rgb )
{
}

MaterialFogMode_t CShaderAPIVK::GetSceneFogMode( )
{
	return MATERIAL_FOG_NONE;
}

int CShaderAPIVK::GetPixelFogCombo( )
{
	return 0;
}

void CShaderAPIVK::FogColor3f( float r, float g, float b )
{
}

void CShaderAPIVK::FogColor3fv( float const* rgb )
{
}

void CShaderAPIVK::FogColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVK::FogColor3ubv( unsigned char const* rgb )
{
}

void CShaderAPIVK::SetViewports( int nCount, const ShaderViewport_t* pViewports )
{
}

int CShaderAPIVK::GetViewports( ShaderViewport_t* pViewports, int nMax ) const
{
	return 1;
}

// Sets the vertex and pixel shaders
void CShaderAPIVK::SetVertexShaderIndex( int vshIndex )
{
}

void CShaderAPIVK::SetPixelShaderIndex( int pshIndex )
{
}

// Sets the constant registers for vertex and pixel shaders
void CShaderAPIVK::SetVertexShaderConstant( int var, float const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIVK::SetBooleanVertexShaderConstant( int var, BOOL const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIVK::SetIntegerVertexShaderConstant( int var, int const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIVK::SetPixelShaderConstant( int var, float const* pVec, int numConst, bool bForce )
{
}

void CShaderAPIVK::SetBooleanPixelShaderConstant( int var, BOOL const* pVec, int numBools, bool bForce )
{
}

void CShaderAPIVK::SetIntegerPixelShaderConstant( int var, int const* pVec, int numIntVecs, bool bForce )
{
}

void CShaderAPIVK::InvalidateDelayedShaderConstants( void )
{
}

float CShaderAPIVK::GammaToLinear_HardwareSpecific( float fGamma ) const
{
	return 0.0f;
}

float CShaderAPIVK::LinearToGamma_HardwareSpecific( float fLinear ) const
{
	return 0.0f;
}

void CShaderAPIVK::SetLinearToGammaConversionTextures( ShaderAPITextureHandle_t hSRGBWriteEnabledTexture, ShaderAPITextureHandle_t hIdentityTexture )
{
}


// Returns the nearest supported format
ImageFormat CShaderAPIVK::GetNearestSupportedFormat( ImageFormat fmt, bool bFilteringRequired /* = true */ ) const
{
	return fmt;
}

ImageFormat CShaderAPIVK::GetNearestRenderTargetFormat( ImageFormat fmt ) const
{
	return fmt;
}

// Sets the texture state
void CShaderAPIVK::BindTexture( Sampler_t stage, ShaderAPITextureHandle_t textureHandle )
{
}

void CShaderAPIVK::ClearColor3ub( unsigned char r, unsigned char g, unsigned char b )
{
}

void CShaderAPIVK::ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
}

// Indicates we're going to be modifying this texture
// TexImage2D, TexSubImage2D, TexWrap, TexMinFilter, and TexMagFilter
// all use the texture specified by this function.
void CShaderAPIVK::ModifyTexture( ShaderAPITextureHandle_t textureHandle )
{
}

// Texture management methods
void CShaderAPIVK::TexImage2D( int level, int cubeFace, ImageFormat dstFormat, int zOffset, int width, int height, 
						 ImageFormat srcFormat, bool bSrcIsTiled, void *imageData )
{
}

void CShaderAPIVK::TexSubImage2D( int level, int cubeFace, int xOffset, int yOffset, int zOffset, int width, int height,
						 ImageFormat srcFormat, int srcStride, bool bSrcIsTiled, void *imageData )
{
}

void CShaderAPIVK::TexImageFromVTF( IVTFTexture *pVTF, int iVTFFrame )
{
}

bool CShaderAPIVK::TexLock( int level, int cubeFaceID, int xOffset, int yOffset, 
								int width, int height, CPixelWriter& writer )
{
	return false;
}

void CShaderAPIVK::TexUnlock( )
{
}


// These are bound to the texture, not the texture environment
void CShaderAPIVK::TexMinFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIVK::TexMagFilter( ShaderTexFilterMode_t texFilterMode )
{
}

void CShaderAPIVK::TexWrap( ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode )
{
}

void CShaderAPIVK::TexSetPriority( int priority )
{
}

ShaderAPITextureHandle_t CShaderAPIVK::CreateTexture( 
	int width, 
	int height,
	int depth,
	ImageFormat dstImageFormat, 
	int numMipLevels, 
	int numCopies, 
	int flags, 
	const char *pDebugName,
	const char *pTextureGroupName )
{
	return 0;
}

// Create a multi-frame texture (equivalent to calling "CreateTexture" multiple times, but more efficient)
void CShaderAPIVK::CreateTextures( 
							ShaderAPITextureHandle_t *pHandles,
							int count,
							int width, 
							int height,
							int depth,
							ImageFormat dstImageFormat, 
							int numMipLevels, 
							int numCopies, 
							int flags, 
							const char *pDebugName,
							const char *pTextureGroupName )
{
	for ( int k = 0; k < count; ++ k )
		pHandles[ k ] = 0;
}


ShaderAPITextureHandle_t CShaderAPIVK::CreateDepthTexture( ImageFormat renderFormat, int width, int height, const char *pDebugName, bool bTexture )
{
	return 0;
}

void CShaderAPIVK::DeleteTexture( ShaderAPITextureHandle_t textureHandle )
{
}

bool CShaderAPIVK::IsTexture( ShaderAPITextureHandle_t textureHandle )
{
	return true;
}

bool CShaderAPIVK::IsTextureResident( ShaderAPITextureHandle_t textureHandle )
{
	return false;
}

// stuff that isn't to be used from within a shader
void CShaderAPIVK::ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil, int renderTargetWidth, int renderTargetHeight )
{
}

void CShaderAPIVK::ClearBuffersObeyStencil( bool bClearColor, bool bClearDepth )
{
}

void CShaderAPIVK::ClearBuffersObeyStencilEx( bool bClearColor, bool bClearAlpha, bool bClearDepth )
{
}

void CShaderAPIVK::PerformFullScreenStencilOperation( void )
{
}

void CShaderAPIVK::SetScissorRect( const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor )
{
}

void CShaderAPIVK::ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat )
{
}

void CShaderAPIVK::ReadPixels( Rect_t *pSrcRect, Rect_t *pDstRect, unsigned char *data, ImageFormat dstFormat, int nDstStride )
{
}

void CShaderAPIVK::FlushHardware()
{
}

void CShaderAPIVK::ResetRenderState( bool bFullReset )
{
}

// Set the number of bone weights
void CShaderAPIVK::SetNumBoneWeights( int numBones )
{
}

void CShaderAPIVK::EnableHWMorphing( bool bEnable )
{
}

// Selection mode methods
int CShaderAPIVK::SelectionMode( bool selectionMode )
{
	return 0;
}

void CShaderAPIVK::SelectionBuffer( unsigned int* pBuffer, int size )
{
}

void CShaderAPIVK::ClearSelectionNames( )
{
}

void CShaderAPIVK::LoadSelectionName( int name )
{
}

void CShaderAPIVK::PushSelectionName( int name )
{
}

void CShaderAPIVK::PopSelectionName()
{
}


// Use this to get the mesh builder that allows us to modify vertex data
CMeshBuilder* CShaderAPIVK::GetVertexModifyBuilder()
{
	return 0;
}

// Board-independent calls, here to unify how shaders set state
// Implementations should chain back to IShaderUtil->BindTexture(), etc.

// Use this to begin and end the frame
void CShaderAPIVK::BeginFrame()
{
}

void CShaderAPIVK::EndFrame()
{
}

// returns the current time in seconds....
double CShaderAPIVK::CurrentTime() const
{
	return Sys_FloatTime();
}

// Get the current camera position in world space.
void CShaderAPIVK::GetWorldSpaceCameraPosition( float * pPos ) const
{
}

void CShaderAPIVK::ForceHardwareSync( void )
{
}

void CShaderAPIVK::SetClipPlane( int index, const float *pPlane )
{
}

void CShaderAPIVK::EnableClipPlane( int index, bool bEnable )
{
}

void CShaderAPIVK::SetFastClipPlane( const float *pPlane )
{
}

void CShaderAPIVK::EnableFastClip( bool bEnable )
{
}

int CShaderAPIVK::GetCurrentNumBones( void ) const
{
	return 0;
}

bool CShaderAPIVK::IsHWMorphingEnabled( void ) const
{
	return false;
}

int CShaderAPIVK::GetCurrentLightCombo( void ) const
{
	return 0;
}

void CShaderAPIVK::GetDX9LightState( LightState_t *state ) const
{
	state->m_nNumLights = 0;
	state->m_bAmbientLight = false;
	state->m_bStaticLightVertex = false;
	state->m_bStaticLightTexel = false;
}

MaterialFogMode_t CShaderAPIVK::GetCurrentFogType( void ) const
{
	return MATERIAL_FOG_NONE;
}

void CShaderAPIVK::RecordString( const char *pStr )
{
}

bool CShaderAPIVK::ReadPixelsFromFrontBuffer() const
{
	return true;
}

bool CShaderAPIVK::PreferDynamicTextures() const
{
	return false;
}

bool CShaderAPIVK::PreferReducedFillrate() const
{ 
	return false; 
}

bool CShaderAPIVK::HasProjectedBumpEnv() const
{
	return true;
}

int  CShaderAPIVK::GetCurrentDynamicVBSize( void )
{
	return 0;
}

void CShaderAPIVK::DestroyVertexBuffers( bool bExitingLevel )
{
}

void CShaderAPIVK::EvictManagedResources()
{
}

void CShaderAPIVK::SetTextureTransformDimension( TextureStage_t textureStage, int dimension, bool projected )
{
}

void CShaderAPIVK::SetBumpEnvMatrix( TextureStage_t textureStage, float m00, float m01, float m10, float m11 )
{
}

void CShaderAPIVK::SyncToken( const char *pToken )
{
}

void CShaderAPIVK::GetBackBufferDimensions( int& width, int& height ) const
{
	s_ShaderDeviceGL.GetBackBufferDimensions( width, height );
}

void CShaderAPIVK::GetCurrentColorCorrection( ShaderColorCorrectionInfo_t* pInfo )
{
	pInfo->m_bIsEnabled = false;
	pInfo->m_nLookupCount = 0;
	pInfo->m_flDefaultWeight = 0.0f;
}