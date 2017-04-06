#ifndef INCLUDED_ESHADERTYPES_H
#define INCLUDED_ESHADERTYPES_H

namespace lux
{
namespace video
{

enum EVertexShaderType
{
	EVST_VS_1_1 = 0,
	EVST_VS_2_0,
	EVST_VS_2_a,
	EVST_VS_3_0,
	EVST_VS_4_0,
	EVST_VS_4_1,
	EVST_VS_5_0,
	EVST_COUNT,
};

enum EPixelShaderType
{
	EPST_PS_1_1 = 0,
	EPST_PS_1_2,
	EPST_PS_1_3,
	EPST_PS_1_4,
	EPST_PS_2_0,
	EPST_PS_2_a,
	EPST_PS_2_b,
	EPST_PS_3_0,
	EPST_PS_4_0,
	EPST_PS_4_1,
	EPST_PS_5_0,
	EPST_COUNT,
};

}
}
#endif // !INCLUDED_ESHADERTYPES_H
