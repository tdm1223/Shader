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
// EnvironmentMapping
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string EnvironmentMapping_Pass_0_Model : ModelData = ".\\TeapotWithTangent.x";

float4x4 worldMatrix : World;
float4x4 worldViewProjectionMatrix : WorldViewProjection;

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

struct VS_INPUT 
{
   float4 position : POSITION;
   float3 normal: NORMAL;
   float3 tangent: TANGENT;
   float3 binormal: BINORMAL;
   float2 uv: TEXCOORD0;
};

struct VS_OUTPUT 
{
   float4 position : POSITION;
   float2 uv : TEXCOORD0;
   float3 lightDir : TEXCOORD1;
   float3 viewDir: TEXCOORD2;
   float3 t : TEXCOORD3;
   float3 b : TEXCOORD4;
   float3 n : TEXCOORD5;
};

VS_OUTPUT EnvironmentMapping_Pass_0_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT Output;

   Output.position = mul( input.position, worldViewProjectionMatrix );
   Output.uv = input.uv;

   float4 worldPosition = mul( input.position, worldMatrix );
   float3 lightDir = worldPosition.xyz - worldLightPosition.xyz;
   Output.lightDir = normalize(lightDir);
   
   float3 viewDir = normalize(worldPosition.xyz - worldCameraPosition.xyz);
   Output.viewDir = viewDir;
   
   float3 worldNormal = mul( input.normal, (float3x3)worldMatrix );
   Output.n = normalize(worldNormal);
   
   float3 worldTangent = mul(input.tangent, (float3x3)worldMatrix );
   Output.t = normalize(worldTangent);
   
   float3 worldBinormal = mul(input.binormal, (float3x3)worldMatrix );
   Output.b = normalize(worldBinormal);
   
   return Output;
}




struct PS_INPUT
{
   float2 uv : TEXCOORD0;
   float3 lightDir : TEXCOORD1;
   float3 viewDir: TEXCOORD2;
   float3 T : TEXCOORD3;
   float3 B : TEXCOORD4;
   float3 N : TEXCOORD5;
};

texture diffuseMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Fieldstone.tga";
>;
sampler2D diffuseSampler = sampler_state
{
   Texture = (diffuseMap_Tex);
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
};
texture specularMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\fieldstone_SM.tga";
>;
sampler2D specularSampler = sampler_state
{
   Texture = (specularMap_Tex);
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
};
sampler2D normalSampler;
texture environmentMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Snow.dds";
>;
samplerCUBE environmentSampler = sampler_state
{
   Texture = (environmentMap_Tex);
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
};

float3 lightColor
<
   string UIName = "lightColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 0.70, 0.70, 1.00 );

float4 EnvironmentMapping_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float3 tangentNormal = tex2D(normalSampler, input.uv).xyz;
   tangentNormal = normalize(tangentNormal * 2 - 1);
   tangentNormal = float3(0,0,1);
   
   float3x3 TBN = float3x3(normalize(input.T), normalize(input.B), normalize(input.N));
   TBN = transpose(TBN);
   float3 worldNormal = mul(TBN, tangentNormal);
   
   float4 albedo = tex2D(diffuseSampler, input.uv);
   float3 lightDir = normalize(input.lightDir);
   float3 diffuse = saturate(dot(worldNormal, -lightDir));
   diffuse = lightColor * albedo.rgb * diffuse;
   
   float3 viewDir = normalize(input.viewDir);
   float3 specular = 0;
   if ( diffuse.x > 0 )
   {
      float3 reflection = reflect(lightDir, worldNormal);

      specular = saturate(dot(reflection, -viewDir ));
      specular = pow(specular, 20.0f);
      
      float4 specularIntensity  = tex2D(specularSampler, input.uv);
      specular *= specularIntensity.rgb * lightColor;
   }

   float3 viewReflect = reflect(viewDir,worldNormal);
   float3 environment = texCUBE(environmentSampler,viewReflect).rgb;

   float3 ambient = float3(0.1f, 0.1f, 0.1f) * albedo;
   
   return float4(ambient + diffuse + specular + environment * 0.5f, 1);
}

//--------------------------------------------------------------//
// Technique Section for EnvironmentMapping
//--------------------------------------------------------------//
technique EnvironmentMapping
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 EnvironmentMapping_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 EnvironmentMapping_Pass_0_Pixel_Shader_ps_main();
   }

}

