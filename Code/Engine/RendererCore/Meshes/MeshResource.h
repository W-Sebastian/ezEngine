#pragma once

#include <RendererCore/Meshes/MeshBufferResource.h>

/// \todo: proper implementation
struct ezMeshResourceDescriptor
{
  ezMeshBufferResourceHandle hMeshBuffer;
};

class EZ_RENDERERCORE_DLL ezMeshResource : public ezResource<ezMeshResource, ezMeshResourceDescriptor>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezMeshResource);

public:
  ezMeshResource();

  struct Part
  {
    ezUInt32 m_uiPrimitiveCount;
    ezUInt32 m_uiFirstPrimitive;
    ezUInt32 m_uiMaterialIndex;
    ezMeshBufferResourceHandle m_hMeshBuffer;
  };

  const ezDynamicArray<Part>& GetParts() const
  {
    return m_Parts;
  }

  ezUInt32 GetMaterialCount() const
  {
    return m_uiMaterialCount;
  }

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReaderBase* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
  virtual ezResourceLoadDesc CreateResource(const ezMeshResourceDescriptor& descriptor) override;

  ezDynamicArray<Part> m_Parts;
  ezUInt32 m_uiMaterialCount;
};

typedef ezResourceHandle<ezMeshResource> ezMeshResourceHandle;
