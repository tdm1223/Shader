//**************************************************************//
//  Effect File exported by RenderMonkey 1.6
//
//  - Although many improvements were made to RenderMonkey FX  
//    file export, there are still situations that may cause   
//    compilation problems once the file is exported, such as  
//    occasional naming conflicts for methods, since FX format 
//    does not support any notions of name spaces. You need to 
//    try to create workspaces in such a way as to minimize    
//    potential naming conflicts on export.                    
//    
//  - Note that to minimize resulting name collisions in the FX 
//    file, RenderMonkey will mangle names for passes, shaders  
//    and function names as necessary to reduce name conflicts. 
//**************************************************************//

//--------------------------------------------------------------//
// ApplyShadow
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// ApplyShadowTorus
//--------------------------------------------------------------//
string ApplyShadow_ApplyShadowTorus_Torus : ModelData = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Torus.3ds";

struct VS_INPUT 
{
   float4 mPosition: POSITION;
   float3 mNormal: NORMAL;
};

struct VS_OUTPUT 
{
   float4 mPosition: POSITION;
   float4 mClipPosition: TEXCOORD1;
   float mDiffuse: TEXCOORD2;
};

float4x4 gWorldMatrix : World;
float4x4 gLightViewMatrix;
float4x4 gLightProjectionMatrix : Projection;

float4 gWorldLightPosition
<
   string UIName = "gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );

float4x4 gViewProjectionMatrix : ViewProjection;

VS_OUTPUT ApplyShadow_ApplyShadowTorus_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;
   
   float4x4 lightViewMatrix = gLightViewMatrix;
   
   //<´| ø¬0 \ õ ÀX 
   float4 worldPosition = mul(input.mPosition, gWorldMatrix);
   output.mPosition = mul(worldPosition, gViewProjectionMatrix);

   //Jt| lX0 \ õ<\ ÀX 
   output.mClipPosition = mul(worldPosition, lightViewMatrix);
   output.mClipPosition = mul(output.mClipPosition, gLightProjectionMatrix);
   
   //œ¬D lX” ü
   float3 lightDir = normalize(worldPosition.xyz - gWorldLightPosition.xyz);
   float3 worldNormal = normalize(mul(input.mNormal, (float3x3)gWorldMatrix));
   output.mDiffuse = dot(-lightDir, worldNormal);
   
   return output;
}
texture ShadowMap_Tex
<
   string ResourceName = "..\\RenderMonkey\\";
>;
sampler2D ShadowSampler = sampler_state
{
   Texture = (ShadowMap_Tex);
};
float4 gObjectColor
<
   string UIName = "gObjectColor";
   string UIWidget = "Color";
   bool UIVisible =  true;
> = float4( 1.00, 1.00, 0.00, 1.00 );

struct PS_INPUT
{
   float4 mClipPosition: TEXCOORD1;
   float mDiffuse: TEXCOORD2;
};

float4 ApplyShadow_ApplyShadowTorus_Pixel_Shader_ps_main(PS_INPUT Input) : COLOR
{
   float3 rgb = saturate(Input.mDiffuse) * gObjectColor;
   
   float currentDepth = Input.mClipPosition.z / Input.mClipPosition.w;
   
   float2 uv = Input.mClipPosition.xy / Input.mClipPosition.w;
   uv.y = -uv.y;
   uv = uv * 0.5 + 0.5;
   
   float shadowDepth = tex2D(ShadowSampler, uv).r;
   
   if (currentDepth > shadowDepth + 0.0000125f)
   {
      rgb *= 0.5f;
   }
   
   return( float4( rgb, 1.0f ) );
}




//--------------------------------------------------------------//
// Technique Section for ApplyShadow
//--------------------------------------------------------------//
technique ApplyShadow
{
   pass ApplyShadowTorus
   {
      VertexShader = compile vs_2_0 ApplyShadow_ApplyShadowTorus_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ApplyShadow_ApplyShadowTorus_Pixel_Shader_ps_main();
   }

}

