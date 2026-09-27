#pragma once
/*===========================================================================
    arkcommonvertexdeclaration.h - the vertex declarations Arkane's full-screen passes draw with.

    DISHONORED(port): Engine/Src/arkcommonvertexdeclaration.cpp (2013 rva 0x41a3f0 InitRHI, 0x410900 the getter).
    The DisFog pass and every FArkPp node draw one clip-space triangle of two-float positions with
    ARK_COMMON_VD_FLOAT2 and ArkFullScreenTriangleFloat2Vertices, stride 8.
===========================================================================*/

enum EArkCommonVertexDeclaration
{
	ARK_COMMON_VD_FLOAT2 = 0,
	ARK_COMMON_VD_FLOAT4 = 1,
	ARK_COMMON_VD_FLOAT2_UV = 2,
	ARK_COMMON_VD_MAX
};

extern const FVertexDeclarationRHIRef& ArkGetCommonVertexDeclaration(EArkCommonVertexDeclaration iValue);

/** The three vertices of the full-screen triangle (2012 data 0xe32ddc). */
extern const FVector2D ArkFullScreenTriangleFloat2Vertices[3];
