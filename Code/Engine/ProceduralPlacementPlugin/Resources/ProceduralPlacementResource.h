#pragma once

#include <ProceduralPlacementPlugin/Basics.h>
#include <Core/ResourceManager/Resource.h>

typedef ezTypedResourceHandle<class ezProceduralPlacementResource> ezProceduralPlacementResourceHandle;

struct EZ_PROCEDURALPLACEMENTPLUGIN_DLL ezProceduralPlacementResourceDescriptor
{
  // empty, these types of resources must be loaded from file
};

class EZ_PROCEDURALPLACEMENTPLUGIN_DLL ezProceduralPlacementResource : public ezResource<ezProceduralPlacementResource, ezProceduralPlacementResourceDescriptor>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezProceduralPlacementResource, ezResourceBase);

public:
  ezProceduralPlacementResource();
  ~ezProceduralPlacementResource();

  const ezDynamicArray<ezSharedPtr<const ezPPInternal::Layer>>& GetLayers() const;

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
  virtual ezResourceLoadDesc CreateResource(ezProceduralPlacementResourceDescriptor&& descriptor) override;

private:

  ezDynamicArray<ezExpressionByteCode> m_ByteCode;
  ezDynamicArray<ezSharedPtr<const ezPPInternal::Layer>> m_Layers;
};

