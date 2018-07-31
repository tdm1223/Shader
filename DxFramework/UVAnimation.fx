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
// UVAnimation
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// Pass 0
//--------------------------------------------------------------//
string UVAnimation_Pass_0_Model : ModelData = ".\\torus.x";

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

float gTime : Time0_X;
float gWaveHeight
<
   string UIName = "gWaveHeight";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 3.00 );
float gSpeed
<
   string UIName = "gSpeed";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 2.00 );
float gWaveFrequency
<
   string UIName = "gWaveFrequency";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 10.00 );
float gUVSpeed
<
   string UIName = "gUVSpeed";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float( 0.25 );

struct VS_INPUT
{
   float4 mPosition : POSITION;
   float3 mNormal : NORMAL;
   float2 mUV : TEXCOORD0;
};

struct VS_OUTPUT
{
   float4 mPosition : POSITION;
   float2 mUV : TEXCOORD0;
   float3 mDiffuse : TEXCOORD1;   
   float3 mViewDir : TEXCOORD2;
   float3 mReflection : TEXCOORD3;
};


VS_OUTPUT UVAnimation_Pass_0_Vertex_Shader_vs_main(VS_INPUT input)
{
   VS_OUTPUT output;
   
   float cosTime = gWaveHeight * cos(gTime*gSpeed + input.mUV.x*gWaveFrequency);
   input.mPosition.y+=cosTime;
   
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
   
   output.mUV=input.mUV+float2(gTime*gUVSpeed,0);
   
   return output;
}


struct PS_INPUT
{
   float2 mUV : TEXCOORD0;
   float3 mDiffuse : TEXCOORD1;
   float3 mViewDir : TEXCOORD2;
   float3 mReflection: TEXCOORD3;
};

texture DiffuseMap_Tex
<
   string ResourceName = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Textures\\Fieldstone.tga";
>;
sampler2D DiffuseSampler = sampler_state
{
   Texture = (DiffuseMap_Tex);
};
texture SpecularMap_Tex
<
   string ResourceName = ".\\Fieldstone_SM.tga";
>;
sampler2D SpecularSampler = sampler_state
{
   Texture = (SpecularMap_Tex);
};

float3 gLightColor
<
   string UIName = "gLightColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 0.70, 0.70, 1.00 );

float gPow=20.0f;

float4 UVAnimation_Pass_0_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float4 albedo = tex2D(DiffuseSampler,input.mUV);
   float3 diffuse = gLightColor * albedo.rgb * saturate(input.mDiffuse);
   
   float3 reflection = normalize(input.mReflection);
   float3 viewDir = normalize(input.mViewDir);
   float3 specular = 0;
   if(diffuse.x>0)
   {
      specular = saturate(dot(reflection,-viewDir));
      specular = pow(specular,gPow);
      
      float4 specularIntensity = tex2D(SpecularSampler,input.mUV);
      specular *= specularIntensity * gLightColor;
   }
   
   float3 ambient = float3(0.1f,0.1f,0.1f) * albedo;
   
   return float4(ambient+diffuse+specular,1);
}
//--------------------------------------------------------------//
// Technique Section for UVAnimation
//--------------------------------------------------------------//
technique UVAnimation
{
   pass Pass_0
   {
      VertexShader = compile vs_2_0 UVAnimation_Pass_0_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 UVAnimation_Pass_0_Pixel_Shader_ps_main();
   }

}

