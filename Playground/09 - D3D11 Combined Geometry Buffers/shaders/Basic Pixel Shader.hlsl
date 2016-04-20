struct PixelFragment
{
	float4 PosH  : POSITION;
	float4 Color : COLOR;
};

float4 main(PixelFragment pIn) : SV_TARGET
{
	return pIn.Color;
}