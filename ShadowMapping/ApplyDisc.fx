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
// ApplyDisc
//--------------------------------------------------------------//
//--------------------------------------------------------------//
// ApplyShadowDisc
//--------------------------------------------------------------//
string ApplyDisc_ApplyShadowDisc_Disc : ModelData = "..\\..\\..\\..\\..\\Program Files (x86)\\AMD\\RenderMonkey 1.82\\Examples\\Media\\Models\\Disc.3ds";

struct VS_INPUT 
{
   float4 position: POSITION;
   float3 normal: NORMAL;
};

struct VS_OUTPUT 
{
   float4 position: POSITION;
   float4 clipPosition: TEXCOORD1;
   float diffuse: TEXCOORD2; //�� ��
};

float4x4 worldMatrix
<
   string UIName = "worldMatrix";
   string UIWidget = "Numeric";
   bool UIVisible =  false;
> = float4x4( 2.00, 0.00, 0.00, 0.00, 0.00, 2.00, 0.00, -40.00, 0.00, 0.00, 2.00, 0.00, 0.00, 0.00, 0.00, 1.00 );
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

float4x4 viewProjectionMatrix : ViewProjection;

VS_OUTPUT ApplyDisc_ApplyShadowDisc_Vertex_Shader_vs_main( VS_INPUT input )
{
   VS_OUTPUT output;
   
   float4x4 lightViewMatrixTmp = lightViewMatrix;
   
   //<�| ��0 \ � �X 
   float4 worldPosition = mul(input.position, worldMatrix);
   output.position = mul(worldPosition, viewProjectionMatrix);

   //Jt| lX0 \ �<\ �X 
   output.clipPosition = mul(worldPosition, lightViewMatrixTmp);
   output.clipPosition = mul(output.clipPosition, lightProjectionMatrix);
   
   //��D lX� �
   float3 lightDir = normalize(worldPosition.xyz - worldLightPosition.xyz);
   float3 worldNormal = normalize(mul(input.normal, (float3x3)worldMatrix));
   output.diffuse = dot(-lightDir, worldNormal);
   
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
float4 objectColor
<
   string UIName = "objectColor";
   string UIWidget = "Color";
   bool UIVisible =  true;
> = float4( 0.00, 1.00, 1.00, 1.00 );

struct PS_INPUT
{
   float4 clipPosition: TEXCOORD1;
   float diffuse: TEXCOORD2;
};

float4 ApplyDisc_ApplyShadowDisc_Pixel_Shader_ps_main(PS_INPUT input) : COLOR
{
   float3 rgb = saturate(input.diffuse) * objectColor;
   
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
// Technique Section for ApplyDisc
//--------------------------------------------------------------//
technique ApplyDisc
{
   pass ApplyShadowDisc
   {
      VertexShader = compile vs_2_0 ApplyDisc_ApplyShadowDisc_Vertex_Shader_vs_main();
      PixelShader = compile ps_2_0 ApplyDisc_ApplyShadowDisc_Pixel_Shader_ps_main();
   }

}

