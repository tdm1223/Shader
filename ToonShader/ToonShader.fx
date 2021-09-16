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
// ToonShader
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string ToonShader_Pass_0_Model : ModelData = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Teapot.3ds";

struct VS_INPUT 
{
   float4 position : POSITION;
   float3 normal : NORMAL;
};

struct VS_OUTPUT 
{
   float4 position : POSITION;
   float3 diffuse : TEXCOORD1;
};

float4x4 worldViewProjectionMatrix : WorldViewProjection;
float4x4 inverseWorldMatrix : WorldInverse;
float4 worldLightPosition
<
   string UIName = "worldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );

VS_OUTPUT ToonShader_Pass_0_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;

   output.position=mul(input.position,worldViewProjectionMatrix);
   
   float3 objectLightPosition = mul(worldLightPosition,inverseWorldMatrix);
   float3 lightDir = normalize(input.position.xyz-objectLightPosition);
   
   output.diffuse=dot(-lightDir,normalize(input.normal));
   
   return output;
   
}




struct PS_INPUT
{
   float3 diffuse : TEXCOORD1;
};

float3 surfaceColor
<
   string UIName = "surfaceColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 0.00, 1.00, 0.00 );
float toonShaderParam
<
   string UIName = "toonShaderParam";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 5.00 );

float4 ToonShader_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{   
   float3 diffuse = saturate(input.diffuse);
   
   diffuse = ceil(diffuse * toonShaderParam) / toonShaderParam;
   
   return float4(surfaceColor * diffuse.xyz,1);
   
}




//--------------------------------------------------------------//
// Technique Section for ToonShader
//--------------------------------------------------------------//
technique ToonShader
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 ToonShader_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ToonShader_Pass_0_Pixel_Shader_ps_main();
   }

}

