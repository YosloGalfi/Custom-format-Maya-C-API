#pragma once

namespace awesomeFormat
{
	const int NAME_MAX_LENGTH = 50;
	const int FILEPATH_MAX_LENGTH = 100;

	enum materialType {lambert, phong, pbr};
	enum textureType { diffuse, normal, specular, opacity, displacement };

	struct float4
	{
		float x, y, z, w;
		float4() { x = 0; y = 0; z = 0; w = 0; }
	};

	struct float3
	{
		float x, y, z;
		float3() { x = 0; y = 0; z = 0; }
	};

	struct float2
	{
		float u, v;
		float2() { u = 0; v = 0; }
	};

	struct fileHeader
	{
		int nrOfMeshes;
		int nrOfCameras;
		int nrOfLights;
		int nrOfDeformers;
	};

	struct afMatrix
	{
		float matrix[16];
		afMatrix() { matrix[0] = 1.0f; matrix[1] = 0.0f; matrix[2] = 0.0f; matrix[3] = 0.0f;
			matrix[4] = 0.0f; matrix[5] = 1.0f; matrix[6] = 0.0f; matrix[7] = 0.0f;
			matrix[8] = 0.0f; matrix[9] = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
			matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f; }
	};

	struct afTransform
	{
		char transformName[NAME_MAX_LENGTH];
		char parentName[NAME_MAX_LENGTH];
		afMatrix transformation;
	};

	struct afMesh
	{
		char meshName[NAME_MAX_LENGTH];
		char parentName[NAME_MAX_LENGTH];
		int materialID;
		int nrOfVertices;
		int nrOfIndices;
		afTransform transform;
	};

	struct afVertex
	{
		float3 position;
		float3 normal;
		float2 uv;
		float3 tangent;
	};

	struct afIndex
	{
		size_t index;
	};

	struct afMaterial
	{
		char materialName[NAME_MAX_LENGTH];
		int materialId;
		int nrOfTextures = 0;
		float3 diffuse;
		float3 ambient;
		float3 specular;
		materialType type;
	};

	struct afTexture
	{
		char textureName[NAME_MAX_LENGTH];
		char filePath[FILEPATH_MAX_LENGTH];
		int materialID;
		textureType type;
	};
}