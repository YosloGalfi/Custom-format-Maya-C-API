
#include "maya_includes.h"
#include "awesomeFormat.h"
#include <maya/MTimer.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <string>
#include <fstream>

namespace af = awesomeFormat;

void printFile(std::string fileName, std::vector<af::afMesh> meshes, std::vector<af::afMaterial> materials, std::vector<af::afTexture> textures,
	std::vector<std::vector<af::afVertex>> vertices)
{
	std::ofstream output(fileName, std::ofstream::binary);

	af::fileHeader fileHeader;
	fileHeader.nrOfMeshes = meshes.size();

	output.write((const char*)&fileHeader, sizeof(af::fileHeader));

	for (int i = 0; i < meshes.size(); i++)
	{
		output.write((const char*)&meshes[i], sizeof(af::afMesh));

		for (int j = 0; j < vertices[i].size(); j++)
		{
			output.write((const char*)&vertices[i][j], sizeof(af::afVertex));
		}

		output.write((const char*)&materials[i], sizeof(af::afMaterial));

		if (materials[i].nrOfTextures > 0)
		{
			for (int j = 0; j < textures.size(); j++)
			{
				if (textures[j].materialID == materials[i].materialId)
				{
					output.write((const char*)&textures[j], sizeof(af::afTexture));
				}
			}
		}
	}

	output.close();
}

void testPrint(std::string fileName)
{
	std::ifstream reader(fileName, std::ios::binary);

	af::fileHeader newFile;

	reader.read((char*)&newFile, sizeof(af::fileHeader));

	std::vector<af::afMesh> tempMeshes;
	std::vector<std::vector<af::afVertex>> tempVertices;
	std::vector<af::afMaterial> tempMaterials;
	std::vector<af::afTexture> tempTextures;

	for (int i = 0; i < newFile.nrOfMeshes; i++)
	{
		af::afMesh tempMesh;
		reader.read((char*)&tempMesh, sizeof(af::afMesh));
		tempMeshes.push_back(tempMesh);

		std::vector<af::afVertex> temporary;

		for (int j = 0; j < tempMesh.nrOfVertices; j++)
		{
			af::afVertex tempVertex;
			reader.read((char*)&tempVertex, sizeof(af::afVertex));

			temporary.push_back(tempVertex);
		}
		tempVertices.push_back(temporary);

		af::afMaterial tempMaterial;
		reader.read((char*)&tempMaterial, sizeof(af::afMaterial));

		tempMaterials.push_back(tempMaterial);

		if (tempMaterial.nrOfTextures > 0)
		{
			af::afTexture tempTexture;
			reader.read((char*)&tempTexture, sizeof(af::afTexture));
			tempTextures.push_back(tempTexture);
		}
	}

	for (int i = 0; i < tempMeshes.size(); i++)
	{
		std::cout << "Mesh: " << i << " name: " << tempMeshes[i].meshName << std::endl;
		std::cout << "Mesh: " << i << " material: " << tempMeshes[i].materialID << std::endl;
		std::cout << "Mesh: " << i << " parentName: " << tempMeshes[i].parentName << std::endl;
		std::cout << "Mesh: " << i << " vertices: " << tempMeshes[i].nrOfVertices << std::endl;
		std::cout << "Mesh: " << i << " transform: " << tempMeshes[i].transform.transformName << std::endl;
	}

	for (int i = 0; i < tempMaterials.size(); i++)
	{
		std::cout << "Material: " << i << " name: " << tempMaterials[i].materialName << std::endl;
		std::cout << "Material: " << i << " material: " << tempMaterials[i].materialId << std::endl;
		std::cout << "Material: " << i << " textures: " << tempMaterials[i].nrOfTextures << std::endl;
		std::cout << "Material: " << i << " ambient: " << tempMaterials[i].ambient.x << " " << tempMaterials[i].ambient.y << " " << tempMaterials[i].ambient.z << std::endl;
		std::cout << "Material: " << i << " diffuse: " << tempMaterials[i].diffuse.x << " " << tempMaterials[i].diffuse.y << " " << tempMaterials[i].diffuse.z << std::endl;
	}

	for (int i = 0; i < tempTextures.size(); i++)
	{
		std::cout << "Texture: " << i << " name: " << tempTextures[i].textureName << std::endl;
		std::cout << "Texture: " << i << " material: " << tempTextures[i].materialID << std::endl;
		std::cout << "Texture: " << i << " filepath: " << tempTextures[i].filePath << std::endl;
		std::cout << "Texture: " << i << " type: " << tempTextures[i].type << std::endl;
	}

	for (int i = 0; i < tempVertices.size(); i++)
	{
		std::cout << "Vertices: " << tempVertices[i].size() << std::endl;
	}

	reader.close();
}

void processMaterial(MObject node, std::vector<af::afMaterial>& materials, std::vector<af::afTexture>& textures, int meshID)
{
	MStatus status;

	MFnMesh mesh(node);

	MObjectArray shaders;
	MIntArray materialIndices;

	mesh.getConnectedShaders(0, shaders, materialIndices);

	for (int i = 0; i < shaders.length(); i++)
	{
		af::afMaterial tempMaterial;

		tempMaterial.materialId = meshID;

		MFnDependencyNode depNode(shaders[i]);
		MPlugArray connections;

		MPlug shaderPlug = depNode.findPlug("surfaceShader");
		shaderPlug.connectedTo(connections, true, false);

		for (int j = 0; j < connections.length(); j++)
		{
			MObject materialNode(connections[j].node());

			// For lambert materials
			if (materialNode.hasFn(MFn::kLambert))
			{
				tempMaterial.type = af::lambert;

				MFnDependencyNode lambertDependency = (materialNode);
				MFnLambertShader lambert(materialNode);
				
				strcpy_s(tempMaterial.materialName, lambert.name().asChar());

				tempMaterial.diffuse.x = lambert.color().r;
				tempMaterial.diffuse.y = lambert.color().g;
				tempMaterial.diffuse.z = lambert.color().b;

				tempMaterial.ambient.x = lambert.ambientColor().r;
				tempMaterial.ambient.y = lambert.ambientColor().g;
				tempMaterial.ambient.z = lambert.ambientColor().b;

				MPlugArray lambertConnections;
				MPlug colorPlug = lambertDependency.findPlug("color");
				colorPlug.connectedTo(lambertConnections, true, false);

				for (int k = 0; k < lambertConnections.length(); k++)
				{
					MObject textureNode(lambertConnections[k].node());
					if (textureNode.hasFn(MFn::kFileTexture))
					{
						MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", status);
						if (status)
						{
							af::afTexture tempTexture;
							MString filePath;
							filenamePlug.getValue(filePath);

							if (filePath != "")
							{
								std::string temp = filePath.asChar();
								size_t pos = temp.find_last_of("/\\");
								std::string name = temp.substr(pos + 1);

								strcpy_s(tempTexture.filePath, filePath.asChar());
								strcpy_s(tempTexture.textureName, name.c_str());
								tempTexture.type = af::textureType::diffuse;
								tempTexture.materialID = meshID;
								tempMaterial.nrOfTextures += 1;
								textures.push_back(tempTexture);
							}
						}
					}
				}

				MPlugArray normalConnections;
				MPlug normalPlug = lambertDependency.findPlug("Normal Camera");
				normalPlug.connectedTo(normalConnections, true, false);

				for (int k = 0; k < normalConnections.length(); k++)
				{
					MObject textureNode(normalConnections[k].node());
					if (textureNode.hasFn(MFn::kBump))
					{
						MPlugArray normalConnections;
						MPlug normalPlug = MFnDependencyNode(textureNode).findPlug("bumpValue");
						normalPlug.connectedTo(normalConnections, true, false);

						for (int l = 0; l < normalConnections.length(); l++)
						{
							MObject fileNode(normalConnections[l].node());

							if (fileNode.hasFn(MFn::kFileTexture))
							{
								MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", status);
								if (status)
								{
									af::afTexture tempTexture;
									MString filePath;
									filenamePlug.getValue(filePath);

									if (filePath != "")
									{
										std::string temp = filePath.asChar();
										size_t pos = temp.find_last_of("/\\");
										std::string name = temp.substr(pos + 1);

										strcpy_s(tempTexture.filePath, filePath.asChar());
										strcpy_s(tempTexture.textureName, name.c_str());
										tempTexture.type = af::textureType::normal;
										tempTexture.materialID = meshID;
										tempMaterial.nrOfTextures += 1;
										textures.push_back(tempTexture);
									}
								}
							}
						}
					}

				}
			}

			// For phong materials
			else if (materialNode.hasFn(MFn::kPhong))
			{
				tempMaterial.type = af::phong;

				MFnDependencyNode phongDependency = (materialNode);
				MFnPhongShader phong(materialNode);

				strcpy_s(tempMaterial.materialName, phong.name().asChar());

				tempMaterial.diffuse.x = phong.color().r;
				tempMaterial.diffuse.y = phong.color().g;
				tempMaterial.diffuse.z = phong.color().b;

				tempMaterial.ambient.x = phong.ambientColor().r;
				tempMaterial.ambient.y = phong.ambientColor().g;
				tempMaterial.ambient.z = phong.ambientColor().b;

				tempMaterial.specular.x = phong.specularColor().r;
				tempMaterial.specular.y = phong.specularColor().g;
				tempMaterial.specular.z = phong.specularColor().b;

				MPlugArray phongConnections;
				MPlug colorPlug = phongDependency.findPlug("color");
				colorPlug.connectedTo(phongConnections, true, false);

				for (int k = 0; k < phongConnections.length(); k++)
				{
					MObject textureNode(phongConnections[k].node());
					if (textureNode.hasFn(MFn::kFileTexture))
					{
						MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", status);
						if (status)
						{
							af::afTexture tempTexture;
							MString filePath;
							filenamePlug.getValue(filePath);

							if (filePath != "")
							{
								std::string temp = filePath.asChar();
								size_t pos = temp.find_last_of("/\\");
								std::string name = temp.substr(pos + 1);

								strcpy_s(tempTexture.filePath, filePath.asChar());
								strcpy_s(tempTexture.textureName, name.c_str());
								tempTexture.type = af::textureType::diffuse;
								tempTexture.materialID = meshID;
								tempMaterial.nrOfTextures += 1;
								textures.push_back(tempTexture);
							}
						}
					}
				}

				MPlugArray normalConnections;
				MPlug normalPlug = phongDependency.findPlug("Normal Camera");
				normalPlug.connectedTo(normalConnections, true, false);

				for (int k = 0; k < normalConnections.length(); k++)
				{
					MObject textureNode(normalConnections[k].node());
					if (textureNode.hasFn(MFn::kBump))
					{
						MPlugArray normalConnections;
						MPlug normalPlug = MFnDependencyNode(textureNode).findPlug("bumpValue");
						normalPlug.connectedTo(normalConnections, true, false);

						for (int l = 0; l < normalConnections.length(); l++)
						{
							MObject fileNode(normalConnections[l].node());

							if (fileNode.hasFn(MFn::kFileTexture))
							{
								MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName", status);
								if (status)
								{
									af::afTexture tempTexture;
									MString filePath;
									filenamePlug.getValue(filePath);

									if (filePath != "")
									{
										std::string temp = filePath.asChar();
										size_t pos = temp.find_last_of("/\\");
										std::string name = temp.substr(pos + 1);

										strcpy_s(tempTexture.filePath, filePath.asChar());
										strcpy_s(tempTexture.textureName, name.c_str());
										tempTexture.type = af::textureType::normal;
										tempTexture.materialID = meshID;
										tempMaterial.nrOfTextures += 1;
										textures.push_back(tempTexture);
									}
								}
							}
						}
					}
					
				}
				std::cout << "Inside phong" << std::endl;
			}
			// For PBS materials
			//else if(materialNode.hasFn(MFn::))
		}

		materials.push_back(tempMaterial);
	}
	// Process material here
}

void processObject(MObject object, std::vector<af::afMesh>& meshes, std::vector<af::afMaterial>& materials, std::vector<af::afTexture>& textures, 
	std::vector<std::vector<af::afVertex>>& vertices, std::vector<std::vector<af::afIndex>>& indices, int& meshID)
{
	if (object.hasFn(MFn::kMesh))
	{
		meshID++;

		MStatus status;
		af::afMesh tempMesh;

		MFnDagNode dagNode(object);
		MFnDagNode parentNode = dagNode.parent(0);

		strcpy_s(tempMesh.meshName, dagNode.name().asChar());
		strcpy_s(tempMesh.parentName, parentNode.name().asChar());

		// Process mesh for vertices
		MFnMesh mfnMesh(object, &status);
		if (status)
		{
			std::vector<af::afVertex> tempVertices;

			MPointArray vertexArray;
			MFloatVectorArray normalArray;
			MFloatArray u, v;
			MIntArray count, UVid;
			MIntArray triangleCount, triangleIndex;
			MIntArray vertexCounts, vertexID;
			MIntArray normalCount, normalList;

			// Vertex information
			mfnMesh.getPoints(vertexArray, MSpace::kObject);
			mfnMesh.getNormals(normalArray);
			mfnMesh.getUVs(u, v);

			// Index information
			mfnMesh.getVertices(vertexCounts, vertexID);
			mfnMesh.getTriangleOffsets(triangleCount, triangleIndex);
			mfnMesh.getNormalIds(normalCount, normalList);
			mfnMesh.getAssignedUVs(count, UVid);

			tempVertices.resize(triangleIndex.length());
			tempMesh.nrOfVertices = triangleIndex.length();

			for (int i = 0; i < triangleIndex.length(); i++)
			{
				af::afVertex tempVertex;

				tempVertex.position.x = vertexArray[vertexID[triangleIndex[i]]].x;
				tempVertex.position.y = vertexArray[vertexID[triangleIndex[i]]].y;
				tempVertex.position.z = vertexArray[vertexID[triangleIndex[i]]].z;

				tempVertex.normal.x = normalArray[normalList[triangleIndex[i]]].x;
				tempVertex.normal.y = normalArray[normalList[triangleIndex[i]]].y;
				tempVertex.normal.z = normalArray[normalList[triangleIndex[i]]].z;

				tempVertex.uv.u = u[UVid[triangleIndex[i]]];
				tempVertex.uv.v = v[UVid[triangleIndex[i]]];

				tempVertices[i] = tempVertex;
			}

			vertices.push_back(tempVertices);

			processMaterial(object, materials, textures, meshID);

		}

		// Process the children of the mesh recursively
		MObject parent(dagNode.parent(0));
		MFnDagNode parentDag(parent);

		if (parent.hasFn(MFn::kTransform))
		{
			af::afTransform tempTransform;
			strcpy_s(tempTransform.transformName, parentDag.name().asChar());
			strcpy_s(tempTransform.parentName, MFnDagNode(parentDag.parent(0)).name().asChar());

			MMatrix worldMatrix;
			MDagPath dagPath = MDagPath::getAPathTo(parent);
			worldMatrix = dagPath.inclusiveMatrix();

			int index = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					tempTransform.transformation.matrix[index] = worldMatrix[i][j];
					index++;
				}
			}

			tempMesh.transform = tempTransform;
		}

		tempMesh.materialID = meshID;

		meshes.push_back(tempMesh);

		// If there are any children
		if (dagNode.childCount() > 0)
		{
			// Repeat recursively
			for (int i = 0; i < dagNode.childCount(); i++)
			{
				processObject(dagNode.child(i), meshes, materials, textures, vertices, indices, meshID);
			}
		}
	}
	else if (object.hasFn(MFn::kTransform))
	{
		MFnDagNode dagNode(object);
		if (dagNode.childCount() > 0)
		{
			for (int i = 0; i < dagNode.childCount(); i++)
			{
				processObject(dagNode.child(i), meshes, materials, textures, vertices, indices, meshID);
			}
		}
	}
}

void start()
{	
	// Open or create the file
	// Change to GUI in the future
	std::string fileName = "C:/Users/Galfi/Documents/test.aF";
	//std::ofstream output(fileName, std::ofstream::binary);

	MSelectionList selectedObjects;
	MGlobal::getActiveSelectionList(selectedObjects);

	std::vector<af::afMesh> meshes;
	std::vector<af::afMaterial> materials;
	std::vector<af::afTexture> textures;
	std::vector<std::vector<af::afVertex>> vertices;
	std::vector<std::vector<af::afIndex>> indices;
	int meshID = -1;

	// TO DO
	// PBR
	// NORMALMAP, DISPLACEMENT E.T.C...
	// EMBEDDED TEXTURE IN FILE
	// LIGHTS
	// CAMERAS
	// DEFORMERS
	// ANIMATIONS
	// SKELETONS
	// PRINT

	for (int i = 0; i < selectedObjects.length(); i++)
	{
		MObject selectedObject;

		selectedObjects.getDependNode(i, selectedObject);

		processObject(selectedObject, meshes, materials, textures, vertices, indices, meshID);
	}

	
	printFile(fileName, meshes, materials, textures, vertices);

	// FOR READ TESTING
	testPrint(fileName);

}

EXPORT MStatus initializePlugin(MObject obj) 
{		
	MStatus status;

	MFnPlugin myPlugin(obj, "Format transfer", "1.0", "Any", &status);
	if (MFAIL(status)) 
	{
		CHECK_MSTATUS(status);
		return status;
	}  	

	/* Redirects outputs to mayas output window instead of scripting output */
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());

	/* iterate through whole maya scene and check name changing */
	start();

	return status;
}
	
EXPORT MStatus uninitializePlugin(MObject obj) {
	MFnPlugin plugin(obj);

	MTimer gTimer;

	gTimer.endTimer();
	gTimer.clear();

	return MS::kSuccess;
}