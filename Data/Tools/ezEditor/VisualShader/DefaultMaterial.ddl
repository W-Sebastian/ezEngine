Node %MaterialOutput
{
  string %Category { "Output" }
  unsigned_int8 %Color { 127, 0, 110 }
  string %NodeType { "Main" }
  string %CodePermutations { "
BLEND_MODE
RENDER_PASS
SHADING_MODE
TWO_SIDED
INSTANCING
" }

  string %CheckPermutations
  {"
INSTANCING=FALSE
TWO_SIDED=FALSE
BLEND_MODE=OPAQUE
RENDER_PASS=FORWARD
SHADING_MODE=LIT
"}

  string %CodeRenderStates { "#include <Shaders/Materials/MaterialState.h>" }
  string %CodeVertexShader { "
#define USE_NORMAL
#define USE_TANGENT
#define USE_TEXCOORD0
#define USE_OBJECT_POSITION_OFFSET
#define USE_WORLD_POSITION_OFFSET

#include <Shaders/Materials/DefaultMaterialCB.h>
#include <Shaders/Materials/MaterialVertexShader.h>
#include <Shaders/Common/VisualShaderUtil.h>

VS_OUT main(VS_IN Input)
{
  return FillVertexData(Input);
}

float3 GetObjectPositionOffset(VS_IN Input, ezPerInstanceData data)
{
  return ToFloat3($in8);
}

float3 GetWorldPositionOffset(VS_IN Input, ezPerInstanceData data)
{
  return ToFloat3($in9);
}

" }

  string %CodeMaterialParams { "
Permutation BLEND_MODE;
Permutation SHADING_MODE;
Permutation TWO_SIDED;
float MaskThreshold @Default($prop0);
" }

  string %CodePixelDefines { "
#define USE_NORMAL
#define USE_TANGENT
#define USE_TEXCOORD0
#define USE_SIMPLE_MATERIAL_MODEL
#define USE_MATERIAL_EMISSIVE
#define USE_MATERIAL_OCCLUSION
" }

  string %CodePixelIncludes { "
#include <Shaders/Materials/DefaultMaterialCB.h>
#include <Shaders/Materials/MaterialPixelShader.h>
#include <Shaders/Common/VisualShaderUtil.h>
" }

  string %CodePixelSamplers { "" }
  string %CodePixelConstants { "" }
  string %CodePixelBody { "

float3 GetBaseColor(PS_IN Input)
{
  return ToColor3($in0);
}

float3 GetNormal(PS_IN Input)
{
  return TangentToWorldSpace(ToFloat3($in1), Input);
}

float GetMetallic(PS_IN Input)
{
  return saturate(ToFloat1($in2));
}

float GetReflectance(PS_IN Input)
{
  return saturate(ToFloat1($in3));
}

float GetRoughness(PS_IN Input)
{
  return saturate(ToFloat1($in4));
}

float GetOpacity(PS_IN Input)
{
  #if BLEND_MODE == BLEND_MODE_MASKED
    return ToFloat1($in5) - MaskThreshold;
  #else
    return ToFloat1($in5);
  #endif
}

float3 GetEmissiveColor(PS_IN Input)
{
  return ToColor3($in6);
}

float GetOcclusion(PS_IN Input)
{
  return saturate(ToFloat1($in7));
}
" }

  Property %MaskThreshold
  {
    string %Type { "float" }
    float %Value { 0.25 }
  }

  // Pin 0
  InputPin %BaseColor
  {
    string %Type { "float3" }
    unsigned_int8 %Color { 255, 255, 255 }
    bool %Expose { true }
    string %Fallback { "1" }
  }

  // Pin 1
  InputPin %Normal
  {
    string %Type { "float3" }
    unsigned_int8 %Color { 128, 128, 255 }
    string %Fallback { "float3(0, 0, 1)", }
    string %Tooltip { "Surface normal in tangent space." }
  }

  // Pin 2
  InputPin %Metallic
  {
    string %Type { "float" }
    unsigned_int8 %Color { 128, 128, 128 }
    bool %Expose { true }
    string %Fallback { "0" }
  }

  // Pin 3
  InputPin %Reflectance
  {
    string %Type { "float" }
    unsigned_int8 %Color { 210, 255, 100 }
    bool %Expose { true }
    string %Fallback { "0.5" }
  }

  // Pin 4
  InputPin %Roughness
  {
    string %Type { "float" }
    unsigned_int8 %Color { 150, 64, 64 }
    bool %Expose { true }
    string %Fallback { "0.5" }
  }

  // Pin 5
  InputPin %Opacity
  {
    string %Type { "float" }
    unsigned_int8 %Color { 255, 0, 110 }
    bool %Expose { true }
    string %Fallback { "1" }
  }

  // Pin 6
  InputPin %Emissive
  {
    string %Type { "float3" }
    unsigned_int8 %Color { 255, 106, 0 }
    bool %Expose { true }
    string %Fallback { "0" }
  }

  // Pin 7
  InputPin %Occlusion
  {
    string %Type { "float" }
    unsigned_int8 %Color { 127, 115, 63 }
    bool %Expose { true }
    string %Fallback { "1" }
  }

  // Pin 8
  InputPin %LocalPosOffset
  {
    string %Type { "float3" }
    unsigned_int8 %Color { 75, 145, 112 }
    string %Fallback { "0" }
  }

  // Pin 9
  InputPin %GlobalPosOffset
  {
    string %Type { "float3" }
    unsigned_int8 %Color { 226, 96, 93 }
    string %Fallback { "0" }
  }
}
