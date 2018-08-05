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

float4x4 worldMatrix : World;
float4x4 viewMatrix : View;
float4x4 projectionMatrix : Projection;

struct VS_INPUT 
{
   float4 Position : POSITION0;
   float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT 
{
   float4 Position : POSITION0;
   float2 texCoord : TEXCOORD0;
};

VS_OUTPUT TextureMapping_Pass_0_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;
   output.Position=mul(input.Position,worldMatrix);
   output.Position=mul(output.Position,viewMatrix);
   output.Position=mul(output.Position,projectionMatrix);
   output.texCoord=input.texCoord;
   
   return output;
   
}




texture DiffuseMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Earth.jpg";
>;
sampler2D DiffuseSampler = sampler_state
{
   Texture = (DiffuseMap_Tex);
};

struct PS_INPUT
{
   float2 texCoord : TEXCOORD0;
};

float4 TextureMapping_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR0
{   
   float4 albedo = tex2D(DiffuseSampler,input.texCoord);
   
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

