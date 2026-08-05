#include "utlvector.h"
#include "materialsystem/imaterialsystem.h"
#include "shaderapi/ishaderutil.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imesh.h"
#include "materialsystem/idebugtextureinfo.h"
#include "materialsystem/deformations.h"
#include "meshvk.h"
#include "shaderapivk.h"
#include "shaderapidevicevk.h"
#include "shadershadowvk.h"


//-----------------------------------------------------------------------------
// The shader shadow interface
//-----------------------------------------------------------------------------
CShaderShadowVK::CShaderShadowVK()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bUsesVertexAndPixelShaders = false;
}

CShaderShadowVK::~CShaderShadowVK()
{
}

// Sets the default *shadow* state
void CShaderShadowVK::SetDefaultState()
{
	m_IsTranslucent = false;
	m_IsAlphaTested = false;
	m_bIsDepthWriteEnabled = true;
	m_bUsesVertexAndPixelShaders = false;
}

// Methods related to depth buffering
void CShaderShadowVK::DepthFunc( ShaderDepthFunc_t depthFunc )
{
}

void CShaderShadowVK::EnableDepthWrites( bool bEnable )
{
	m_bIsDepthWriteEnabled = bEnable;
}

void CShaderShadowVK::EnableDepthTest( bool bEnable )
{
}

void CShaderShadowVK::EnablePolyOffset( PolygonOffsetMode_t nOffsetMode )
{
}

// Suppresses/activates color writing 
void CShaderShadowVK::EnableColorWrites( bool bEnable )
{
}

// Suppresses/activates alpha writing 
void CShaderShadowVK::EnableAlphaWrites( bool bEnable )
{
}

// Methods related to alpha blending
void CShaderShadowVK::EnableBlending( bool bEnable )
{
	m_IsTranslucent = bEnable;
}

void CShaderShadowVK::BlendFunc( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
}

// A simpler method of dealing with alpha modulation
void CShaderShadowVK::EnableAlphaPipe( bool bEnable )
{
}

void CShaderShadowVK::EnableConstantAlpha( bool bEnable )
{
}

void CShaderShadowVK::EnableVertexAlpha( bool bEnable )
{
}

void CShaderShadowVK::EnableTextureAlpha( TextureStage_t stage, bool bEnable )
{
}


// Alpha testing
void CShaderShadowVK::EnableAlphaTest( bool bEnable )
{
	m_IsAlphaTested = bEnable;
}

void CShaderShadowVK::AlphaFunc( ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */ )
{
}


// Wireframe/filled polygons
void CShaderShadowVK::PolyMode( ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode )
{
}


// Back face culling
void CShaderShadowVK::EnableCulling( bool bEnable )
{
}


// Alpha to coverage
void CShaderShadowVK::EnableAlphaToCoverage( bool bEnable )
{
}


// constant color + transparency
void CShaderShadowVK::EnableConstantColor( bool bEnable )
{
}

// Indicates the vertex format for use with a vertex shader
// The flags to pass in here come from the VertexFormatFlags_t enum
// If pTexCoordDimensions is *not* specified, we assume all coordinates
// are 2-dimensional
void CShaderShadowVK::VertexShaderVertexFormat( unsigned int nFlags, 
												   int nTexCoordCount,
												   int* pTexCoordDimensions,
												   int nUserDataSize )
{
}

// Indicates we're going to light the model
void CShaderShadowVK::EnableLighting( bool bEnable )
{
}

void CShaderShadowVK::EnableSpecular( bool bEnable )
{
}

// Activate/deactivate skinning
void CShaderShadowVK::EnableVertexBlend( bool bEnable )
{
}

// per texture unit stuff
void CShaderShadowVK::OverbrightValue( TextureStage_t stage, float value )
{
}

void CShaderShadowVK::EnableTexture( Sampler_t stage, bool bEnable )
{
}

void CShaderShadowVK::EnableCustomPixelPipe( bool bEnable )
{
}

void CShaderShadowVK::CustomTextureStages( int stageCount )
{
}

void CShaderShadowVK::CustomTextureOperation( TextureStage_t stage, ShaderTexChannel_t channel, 
	ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2 )
{
}

void CShaderShadowVK::EnableTexGen( TextureStage_t stage, bool bEnable )
{
}

void CShaderShadowVK::TexGen( TextureStage_t stage, ShaderTexGenParam_t param )
{
}

// Sets the vertex and pixel shaders
void CShaderShadowVK::SetVertexShader( const char *pShaderName, int vshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
}

void CShaderShadowVK::EnableBlendingSeparateAlpha( bool bEnable )
{
}
void CShaderShadowVK::SetPixelShader( const char *pShaderName, int pshIndex )
{
	m_bUsesVertexAndPixelShaders = ( pShaderName != NULL );
}

void CShaderShadowVK::BlendFuncSeparateAlpha( ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor )
{
}
// indicates what per-vertex data we're providing
void CShaderShadowVK::DrawFlags( unsigned int drawFlags )
{
}