struct PixelFragment
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

float4 main( PixelFragment pIn ) : SV_Target
{
	return pIn.Color;
}