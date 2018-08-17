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
// TextureMapping
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string TextureMapping_Pass_0_Model : ModelData = ".\\sphere.x";

float4x4 worldViewProjectionMatrix : WorldViewProjection;

struct VS_INPUT 
{
   float4 position : POSITION0;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION0;
   float2 uv : TEXCOORD0;
};

VS_OUTPUT TextureMapping_Pass_0_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;
   output.position=mul(input.position,worldViewProjectionMatrix);
   output.uv=input.uv;
   
   return output;
   
}




texture diffuseSampler_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Earth.jpg";
>;
sampler2D diffuseSampler = sampler_state
{
   Texture = (diffuseSampler_Tex);
};

struct PS_INPUT
{
   float2 uv : TEXCOORD0;
};

float4 TextureMapping_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR0
{   
   float4 albedo = tex2D(diffuseSampler,input.uv);
   
   return albedo;
}




//--------------------------------------------------------------//
// Technique Section for TextureMapping
//--------------------------------------------------------------//
technique TextureMapping
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 TextureMapping_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 TextureMapping_Pass_0_Pixel_Shader_ps_main();
   }

}

