﻿#pragma once

#include <RendererCore/Basics.h>
#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Math/Rect.h>

typedef ezTypedResourceHandle<class ezDecalAtlasResource> ezDecalAtlasResourceHandle;
typedef ezTypedResourceHandle<class ezTexture2DResource> ezTexture2DResourceHandle;

class ezImage;

struct ezDecalAtlasResourceDescriptor
{
};

class EZ_RENDERERCORE_DLL ezDecalAtlasResource : public ezResource<ezDecalAtlasResource, ezDecalAtlasResourceDescriptor>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezDecalAtlasResource, ezResourceBase);

public:
  ezDecalAtlasResource();

  struct DecalInfo
  {
    ezString m_sIdentifier;
    ezRectU32 m_Rect;
  };

  const ezTexture2DResourceHandle& GetDiffuseTexture() const { return m_hDiffuse; }
  const ezTexture2DResourceHandle& GetNormalTexture() const { return m_hNormal; }
  const ezMap<ezUInt32, DecalInfo>& GetAllDecals() const { return m_Decals; }

private:
  virtual ezResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual ezResourceLoadDesc UpdateContent(ezStreamReader* Stream) override;

  void ReadDecalInfo(ezStreamReader* Stream);

  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
  virtual ezResourceLoadDesc CreateResource(const ezDecalAtlasResourceDescriptor& descriptor) override;

  void CreateLayerTexture(const ezImage& img, bool bSRGB, ezTexture2DResourceHandle& out_hTexture);

  ezMap<ezUInt32, DecalInfo> m_Decals;
  static ezUInt32 s_uiDecalAtlasResources;
  ezTexture2DResourceHandle m_hDiffuse;
  ezTexture2DResourceHandle m_hNormal;
};