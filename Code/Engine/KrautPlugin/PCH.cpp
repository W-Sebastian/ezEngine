#include <PCH.h>

#include <KrautPlugin/KrautDeclarations.h>

#include <Foundation/Configuration/Startup.h>
#include <KrautPlugin/Resources/KrautTreeResource.h>

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(Kraut, KrautPlugin)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    ezResourceManager::RegisterResourceForAssetType("Kraut Tree", ezGetStaticRTTI<ezKrautTreeResource>());

    ezKrautTreeResourceDescriptor desc;
    desc.m_Details.m_Bounds.SetInvalid();
    ezKrautTreeResourceHandle hResource = ezResourceManager::CreateResource<ezKrautTreeResource>("Missing Kraut Tree", std::move(desc), "Empty Kraut tree");
    ezKrautTreeResource::SetTypeMissingResource(hResource);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    ezKrautTreeResource::SetTypeMissingResource(ezKrautTreeResourceHandle());

    ezKrautTreeResource::CleanupDynamicPluginReferences();
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

void OnLoadPlugin(bool bReloading) {}
void OnUnloadPlugin(bool bReloading) {}

ezPlugin g_Plugin(false, OnLoadPlugin, OnUnloadPlugin);

EZ_DYNAMIC_PLUGIN_IMPLEMENTATION(EZ_KRAUTPLUGIN_DLL, ezKrautPlugin);
