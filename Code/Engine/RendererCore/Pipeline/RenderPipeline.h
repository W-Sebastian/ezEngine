#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <RendererFoundation/Context/Context.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Pipeline/View.h>

class ezRendererCore;

class EZ_RENDERERCORE_DLL ezRenderPipeline
{
public:
  enum Mode
  {
    Synchronous,
    Asynchronous
  };

  ezRenderPipeline(Mode mode);
  ~ezRenderPipeline();

  void ExtractData(const ezView& view);
  void Render(const ezView& view, ezRendererCore* pRenderer);

  void AddPass(ezRenderPipelinePass* pPass);
  void RemovePass(ezRenderPipelinePass* pPass);

  template <typename T>
  T* CreateRenderData(ezRenderPassType passType, ezGameObject* pOwner)
  {
    EZ_CHECK_AT_COMPILETIME(EZ_IS_DERIVED_FROM_STATIC(ezRenderData, T));

    T* pRenderData = EZ_DEFAULT_NEW(T);
    pRenderData->m_uiSortingKey = 0; /// \todo implement sorting

  #if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    pRenderData->m_pOwner = pOwner;
  #endif
    
    if (passType >= m_pExtractedData->m_PassData.GetCount())
    {
      m_pExtractedData->m_PassData.SetCount(passType + 1);
    }

    m_pExtractedData->m_PassData[passType].m_RenderData.PushBack(pRenderData);

    return pRenderData;
  }

  EZ_FORCE_INLINE ezArrayPtr<const ezRenderData*> GetRenderDataWithPassType(ezRenderPassType passType)
  {
    ezArrayPtr<const ezRenderData*> renderData;

    if (m_pRenderData->m_PassData.GetCount() > passType)
    {
      renderData = m_pRenderData->m_PassData[passType].m_RenderData;
    }

    return renderData;
  }

  static ezRenderPassType FindOrRegisterPassType(const char* szPassTypeName);

  EZ_FORCE_INLINE static const char* GetPassTypeName(ezRenderPassType passType)
  {
    return s_PassTypeData[passType].m_sName.GetString().GetData();
  }

  EZ_FORCE_INLINE static ezProfilingId& GetPassTypeProfilingID(ezRenderPassType passType)
  {
    return s_PassTypeData[passType].m_ProfilingID;
  }

private:
  struct PassData
  {
    void SortRenderData();

    ezDynamicArray<const ezRenderData*> m_RenderData;
  };

  struct PipelineData
  {
    ezHybridArray<PassData, 8> m_PassData;
  };

  Mode m_Mode;

  PipelineData m_Data[2];

  PipelineData* m_pExtractedData;
  PipelineData* m_pRenderData;

  static void ClearPipelineData(PipelineData* pPipeLineData);

  ezDynamicArray<ezRenderPipelinePass*> m_Passes;

  static ezRenderPassType s_uiNextPassType;

  struct PassTypeData
  {
    ezHashedString m_sName;
    ezProfilingId m_ProfilingID;
  };

  enum
  {
    MAX_PASS_TYPES = sizeof(ezRenderPassType) * 8
  };

  static PassTypeData s_PassTypeData[MAX_PASS_TYPES];
};

class EZ_RENDERERCORE_DLL ezDefaultPassTypes
{
public:
  static ezRenderPassType Opaque;
  static ezRenderPassType Masked;
  static ezRenderPassType Transparent;
};

