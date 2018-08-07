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
// SpecularMapping
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string SpecularMapping_Pass_0_Model : ModelData = "..\\Lighting\\sphere.x";

struct VS_INPUT
{
   float4 position : POSITION;
   float3 normal : NORMAL;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 position : POSITION;
   float2 uv : TEXCOORD0;
   float3 diffuse : TEXCOORD1;   
   float3 viewDir : TEXCOORD2;
   float3 reflection : TEXCOORD3;
};

float4x4 worldMatrix : World;
float4x4 viewMatrix : View;
float4x4 projectionMatrix : Projection;

float4 worldLightPosition
<
   string UIName = "worldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );
float4 worldCameraPosition : ViewPosition;

VS_OUTPUT SpecularMapping_Pass_0_Vertex_Shader_vs_main(VS_INPUT input)
{
   VS_OUTPUT output;
   output.position = mul(input.position,worldMatrix);
   
   //ÐX XÐ ¬ XLÀ  D ø´ …¬ ¡0| Ìæ
   float3 lightDir = output.position.xyz-worldLightPosition.xyz;
   lightDir = normalize(lightDir);
   
   float3 viewDir = normalize(output.position.xyz-worldCameraPosition.xyz);
   output.viewDir = viewDir;
   
   output.position=mul(output.position,viewMatrix);
   output.position=mul(output.position,projectionMatrix);
   
   //• D ÔÜõ<\ ÀXXà ÜT
   float3 worldNormal = mul(input.normal,(float3x3)worldMatrix);
   worldNormal = normalize(worldNormal);
   
   output.diffuse=dot(-lightDir,worldNormal);
   output.reflection = reflect(lightDir,worldNormal);
   
   output.uv=input.uv;
   return output;
}


struct PS_INPUT
{
   float2 uv : TEXCOORD0;
   float3 diffuse : TEXCOORD1;
   float3 viewDir : TEXCOORD2;
   float3 reflection: TEXCOORD3;
};

float power
<
   string UIName = "power";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 20.00 );

texture diffuseMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Fieldstone.tga";
>;
sampler2D diffuseSampler = sampler_state
{
   Texture = (diffuseMap_Tex);
};
texture specularMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\fieldstone_SM.tga";
>;
sampler2D specularSampler = sampler_state
{
   Texture = (specularMap_Tex);
};

float3 lightColor
<
   string UIName = "lightColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 0.70, 0.70, 1.00 );

float4 SpecularMapping_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float4 albedo = tex2D(diffuseSampler,input.uv); //èˆõ ØÁ
   float3 diffuse = lightColor * albedo.rgb * saturate(input.diffuse);
   
   float3 reflection = normalize(input.reflection);
   float3 viewDir = normalize(input.viewDir);
   float3 specular = 0;
   if(diffuse.x>0)
   {
      specular = saturate(dot(reflection,-viewDir));
      specular = pow(specular,power);
      
      float4 specularIntensity = tex2D(specularSampler,input.uv); // ¤˜Xìõ ØÁ
      specular = lightColor * specularIntensity.rgb * specular;
   
   }
   
   float3 ambient = float3(0.1f,0.1f,0.1f) * albedo;

   return float4(ambient+diffuse+specular,1);
}
//--------------------------------------------------------------//
// Technique Section for SpecularMapping
//--------------------------------------------------------------//
technique SpecularMapping
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 SpecularMapping_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 SpecularMapping_Pass_0_Pixel_Shader_ps_main();
   }

}

