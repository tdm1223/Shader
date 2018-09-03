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
string ApplyShadow_ApplyShadowTorus_Torus : ModelData = ".\\Torus.x";

struct VS_INPUT 
{
   float4 position: POSITION;
   float3 normal: NORMAL;
   float2 uv : TEXCOORD0;
};

struct VS_OUTPUT 
{
   float4 position: POSITION;
   float2 uv : TEXCOORD0;
   float4 clipPosition: TEXCOORD1;
   float3 diffuse: TEXCOORD2; //œ¬ °ü
   float3 viewDir : TEXCOORD3;
   float3 reflection:TEXCOORD4;
};

float4x4 worldMatrix : World;
float4x4 lightViewMatrix
<
   string UIName = "lightViewMatrix";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
> = float4x4( 1.00, 0.00, 0.00, 0.00, 0.00, 1.00, 0.00, 0.00, 0.00, 0.00, 1.00, 0.00, 0.00, 0.00, 0.00, 1.00 );
float4x4 lightProjectionMatrix : Projection;

float4 worldLightPosition
<
   string UIName = "worldLightPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 500.00, 500.00, -500.00, 1.00 );
float4 worldCameraPosition
<
   string UIName = "worldCameraPosition";
   string UIWidget = "Direction";
   bool UIVisible =  false;
   float4 UIMin = float4( -10.00, -10.00, -10.00, -10.00 );
   float4 UIMax = float4( 10.00, 10.00, 10.00, 10.00 );
   bool Normalize =  false;
> = float4( 0.00, 0.00, 0.00, 1.00 );

float4x4 viewProjectionMatrix : ViewProjection;

VS_OUTPUT ApplyShadow_ApplyShadowTorus_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;
   
   float4x4 lightViewMatrixTmp = lightViewMatrix;
   
   //<´| ø¬0 \ õ ÀX 
   output.position = mul(input.position, worldMatrix);

   float3 lightDir = normalize(output.position.xyz-worldLightPosition.xyz);
   
   float3 viewDir = normalize(output.position.xyz-worldCameraPosition.xyz);
   output.viewDir = viewDir;
   
   //Jt| lX0 \ õ<\ ÀX 
   output.clipPosition = mul(output.position, lightViewMatrix);
   output.clipPosition = mul(output.clipPosition, lightProjectionMatrix);
  
   output.position = mul(output.position, viewProjectionMatrix);
   
   float3 worldNormal = normalize(mul(input.normal,(float3x3)worldMatrix));
   
   output.diffuse=dot(-lightDir,worldNormal);
   output.reflection = reflect(lightDir,worldNormal);
   
   output.uv=input.uv;
   return output;
}
texture ShadowMap_Tex
<
   string ResourceName = ".\\";
>;
sampler2D shadowSampler = sampler_state
{
   Texture = (ShadowMap_Tex);
};
float3 lightColor
<
   string UIName = "lightColor";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
   float UIMin = -1.00;
   float UIMax = 1.00;
> = float3( 0.70, 0.70, 1.00 );

struct PS_INPUT
{
   float4 clipPosition: TEXCOORD1;
   float2 uv : TEXCOORD0;
   float3 diffuse : TEXCOORD2;
   float3 viewDir : TEXCOORD3;
   float3 reflection : TEXCOORD4;
};

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
   string ResourceName = ".\\Fieldstone_SM.tga";
>;
sampler2D specularSampler = sampler_state
{
   Texture = (specularMap_Tex);
};

float4 ApplyShadow_ApplyShadowTorus_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float4 albedo = tex2D(diffuseSampler,input.uv);
   float3 diffuse = lightColor * albedo.rgb * saturate(input.diffuse);
   
   float3 reflection = normalize(input.reflection);
   float3 viewDir = normalize(input.viewDir);
   float3 specular = 0;
   if(diffuse.x)
   {
      specular = saturate(dot(reflection,-viewDir));
      specular = pow(specular,20.0f);
      
      float4 specularIntensity = tex2D(specularSampler, input.uv);
      specular *= specularIntensity.rgb;
   }
   
   float3 ambient = float3(0.1f,0.1f,0.1f) * albedo;
   
   float3 rgb = ambient + diffuse + specular;
   
   float currentDepth = input.clipPosition.z / input.clipPosition.w;
   
   float2 uv = input.clipPosition.xy / input.clipPosition.w;
   uv.y = -uv.y;
   uv = uv * 0.5 + 0.5;
   
   float shadowDepth = tex2D(shadowSampler, uv).r;
   
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

