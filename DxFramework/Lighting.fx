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
// Lighting
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string Lighting_Pass_0_Model : ModelData = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Sphere.3ds";

struct VS_INPUT
{
   float4 mPosition : POSITION;
   float3 mNormal : NORMAL;
};

struct VS_OUTPUT
{
   float4 mPosition : POSITION;
   float3 mDiffuse : TEXCOORD1;   
   float3 mViewDir : TEXCOORD2;
   float3 mReflection : TEXCOORD3;
};

float4x4 gWorldMatrix : World;
float4x4 gViewMatrix : View;
float4x4 gProjectionMatrix : Projection;

float4 gWorldLightPosition
<
   string UIName = "gWorldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );
float4 gWorldCameraPosition : ViewPosition;

VS_OUTPUT Lighting_Pass_0_Vertex_Shader_vs_main(VS_INPUT input)
{
   VS_OUTPUT output;
   output.mPosition = mul(input.mPosition,gWorldMatrix);
   
   //ÐX XÐ ¬ XLÀ  D ø´ …¬ ¡0| Ìæ
   float3 lightDir = output.mPosition.xyz-gWorldLightPosition.xyz;
   lightDir = normalize(lightDir);
   
   float3 viewDir = normalize(output.mPosition.xyz-gWorldCameraPosition.xyz);
   output.mViewDir = viewDir;
   
   output.mPosition=mul(output.mPosition,gViewMatrix);
   output.mPosition=mul(output.mPosition,gProjectionMatrix);
   
   float3 worldNormal = mul(input.mNormal,(float3x3)gWorldMatrix);
   worldNormal = normalize(worldNormal);
   
   output.mDiffuse=dot(-lightDir,worldNormal);
   output.mReflection = reflect(lightDir,worldNormal);
   
   return output;
}


struct PS_INPUT
{
   float3 mDiffuse : TEXCOORD1;
   float3 mViewDir : TEXCOORD2;
   float3 mReflection: TEXCOORD3;
};

float gPow=20.0f;
float4 Lighting_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float3 diffuse = saturate(input.mDiffuse);
   
   float3 reflection = normalize(input.mReflection);
   float3 viewDir = normalize(input.mViewDir);
   float3 specular = 0;
   if(diffuse.x>0)
   {
      specular = saturate(dot(reflection,-viewDir));
      specular = pow(specular,gPow);
   }
   
   float3 ambient = float3(0.1f,0.1f,0.1f);
   
   return float4(ambient+diffuse+specular,1);
}
//--------------------------------------------------------------//
// Technique Section for Lighting
//--------------------------------------------------------------//
technique Lighting
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 Lighting_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 Lighting_Pass_0_Pixel_Shader_ps_main();
   }

}

