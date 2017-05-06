﻿#include <PCH.h>
#include <FmodPlugin/Components/FmodEventComponent.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <FmodPlugin/FmodSingleton.h>
#include <FmodPlugin/Resources/FmodSoundEventResource.h>
#include <Core/ResourceManager/ResourceBase.h>
#include <GameEngine/VisualScript/VisualScriptInstance.h>

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_MESSAGE_TYPE(ezFmodEventComponent_RestartSoundMsg);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFmodEventComponent_RestartSoundMsg, 1, ezRTTIDefaultAllocator<ezFmodEventComponent_RestartSoundMsg>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("OneShot", m_bOneShotInstance)->AddAttributes(new ezDefaultValueAttribute(true)),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_MESSAGE_TYPE(ezFmodEventComponent_StopSoundMsg);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFmodEventComponent_StopSoundMsg, 1, ezRTTIDefaultAllocator<ezFmodEventComponent_StopSoundMsg>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Immediate", m_bImmediate),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_MESSAGE_TYPE(ezFmodSoundFinishedEventMessage);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFmodSoundFinishedEventMessage, 1, ezRTTIDefaultAllocator<ezFmodSoundFinishedEventMessage>)
EZ_END_DYNAMIC_REFLECTED_TYPE

//////////////////////////////////////////////////////////////////////////

EZ_IMPLEMENT_MESSAGE_TYPE(ezFmodEventComponent_SoundCueMsg);
EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezFmodEventComponent_SoundCueMsg, 1, ezRTTIDefaultAllocator<ezFmodEventComponent_SoundCueMsg>)
EZ_END_DYNAMIC_REFLECTED_TYPE

//////////////////////////////////////////////////////////////////////////

EZ_BEGIN_COMPONENT_TYPE(ezFmodEventComponent, 1)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Paused", GetPaused, SetPaused),
    EZ_ACCESSOR_PROPERTY("Volume", GetVolume, SetVolume)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.0f, 1.0f)),
    EZ_ACCESSOR_PROPERTY("Pitch", GetPitch, SetPitch)->AddAttributes(new ezDefaultValueAttribute(1.0f), new ezClampValueAttribute(0.01f, 100.0f)),
    EZ_ACCESSOR_PROPERTY("EnvironmentReverb", GetApplyEnvironmentReverb, SetApplyEnvironmentReverb)->AddAttributes(new ezDefaultValueAttribute(true)),
    EZ_ACCESSOR_PROPERTY("SoundEvent", GetSoundEventFile, SetSoundEventFile)->AddAttributes(new ezAssetBrowserAttribute("Sound Event")),
    EZ_ENUM_MEMBER_PROPERTY("OnFinishedAction", ezOnComponentFinishedAction, m_OnFinishedAction),
    EZ_FUNCTION_PROPERTY("Preview", StartOneShot), // This doesn't seem to be working anymore, and I cannot find code for exposing it in the UI either
  }
  EZ_END_PROPERTIES
    EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezFmodEventComponent_RestartSoundMsg, RestartSound),
    EZ_MESSAGE_HANDLER(ezFmodEventComponent_StopSoundMsg, StopSound),
    EZ_MESSAGE_HANDLER(ezFmodEventComponent_SoundCueMsg, SoundCue),
  }
  EZ_END_MESSAGEHANDLERS
    EZ_BEGIN_MESSAGESENDERS
  {
    EZ_MESSAGE_SENDER(m_SoundFinishedEventSender),
  }
  EZ_END_MESSAGESENDERS
}
EZ_END_DYNAMIC_REFLECTED_TYPE

ezFmodEventComponent::ezFmodEventComponent()
{
  m_pEventDesc = nullptr;
  m_pEventInstance = nullptr;
  m_bPaused = false;
  m_fPitch = 1.0f;
  m_fVolume = 1.0f;
}

ezFmodEventComponent::~ezFmodEventComponent()
{

}

void ezFmodEventComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s << m_bPaused;
  s << m_fPitch;
  s << m_fVolume;

  s << m_hSoundEvent;

  ezOnComponentFinishedAction::StorageType type = m_OnFinishedAction;
  s << type;

  /// \todo store and restore current playback position
}

void ezFmodEventComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  s >> m_bPaused;
  s >> m_fPitch;
  s >> m_fVolume;
  s >> m_hSoundEvent;

  ezOnComponentFinishedAction::StorageType type;
  s >> type;
  m_OnFinishedAction = (ezOnComponentFinishedAction::Enum) type;
}

void ezFmodEventComponent::SetPaused(bool b)
{
  if (b == m_bPaused)
    return;

  m_bPaused = b;

  if (m_pEventInstance != nullptr)
  {
    EZ_FMOD_ASSERT(m_pEventInstance->setPaused(m_bPaused));
  }
  else if (!m_bPaused)
  {
    Restart();
  }
}

void ezFmodEventComponent::SetApplyEnvironmentReverb(bool b)
{
  if (m_bApplyEnvironmentReverb == b)
    return;

  m_bApplyEnvironmentReverb = b;

  if (m_pEventInstance != nullptr)
  {
    SetReverbParameters(m_pEventInstance);
  }
}

void ezFmodEventComponent::SetPitch(float f)
{
  if (f == m_fPitch)
    return;

  m_fPitch = f;

  if (m_pEventInstance != nullptr)
  {
    EZ_FMOD_ASSERT(m_pEventInstance->setPitch(m_fPitch * (float)GetWorld()->GetClock().GetSpeed()));
  }
}

void ezFmodEventComponent::SetVolume(float f)
{
  if (f == m_fVolume)
    return;

  m_fVolume = f;

  if (m_pEventInstance != nullptr)
  {
    EZ_FMOD_ASSERT(m_pEventInstance->setVolume(m_fVolume));
  }
}

void ezFmodEventComponent::SetSoundEventFile(const char* szFile)
{
  ezFmodSoundEventResourceHandle hRes;

  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    hRes = ezResourceManager::LoadResource<ezFmodSoundEventResource>(szFile);
  }

  SetSoundEvent(hRes);
}

const char* ezFmodEventComponent::GetSoundEventFile() const
{
  if (!m_hSoundEvent.IsValid())
    return "";

  return m_hSoundEvent.GetResourceID();
}

void ezFmodEventComponent::SetSoundEvent(const ezFmodSoundEventResourceHandle& hSoundEvent)
{
  if (m_pEventInstance)
  {
    ezFmodEventComponent_StopSoundMsg msg;
    msg.m_bImmediate = false;
    StopSound(msg);

    EZ_FMOD_ASSERT(m_pEventInstance->release());
    m_pEventInstance = nullptr;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    if (m_pSubscripedTo)
    {
      m_pSubscripedTo->m_ResourceEvents.RemoveEventHandler(ezMakeDelegate(&ezFmodEventComponent::ResourceEventHandler, this));
      m_pSubscripedTo = nullptr;
    }
#endif
  }

  m_hSoundEvent = hSoundEvent;
}

void ezFmodEventComponent::OnDeactivated()
{
  if (m_pEventInstance != nullptr)
  {
    EZ_FMOD_ASSERT(m_pEventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
    EZ_FMOD_ASSERT(m_pEventInstance->release());
    m_pEventInstance = nullptr;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    if (m_pSubscripedTo)
    {
      m_pSubscripedTo->m_ResourceEvents.RemoveEventHandler(ezMakeDelegate(&ezFmodEventComponent::ResourceEventHandler, this));
      m_pSubscripedTo = nullptr;
    }
#endif
  }
}

void ezFmodEventComponent::OnSimulationStarted()
{
  if (!m_bPaused)
  {
    Restart();
  }
}

void ezFmodEventComponent::Restart()
{
  if (!m_hSoundEvent.IsValid() || !IsActiveAndSimulating())
    return;

  if (m_pEventInstance == nullptr)
  {
#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    EZ_ASSERT_DEV(m_pSubscripedTo == nullptr, "Cannot be subscribed already at this time");
#endif

    ezResourceLock<ezFmodSoundEventResource> pEvent(m_hSoundEvent, ezResourceAcquireMode::NoFallback);

    if (pEvent->IsMissingResource())
    {
      ezLog::Debug("Cannot start sound event, resource is missing.");
      return;
    }

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
    m_pSubscripedTo = pEvent.operator->();
    m_pSubscripedTo->m_ResourceEvents.AddEventHandler(ezMakeDelegate(&ezFmodEventComponent::ResourceEventHandler, this));
#endif

    m_pEventInstance = pEvent->CreateInstance();
    if (m_pEventInstance == nullptr)
    {
      ezLog::Debug("Cannot start sound event, instance could not be created.");
      return;
    }
  }

  m_bPaused = false;

  SetReverbParameters(m_pEventInstance);
  SetParameters3d(m_pEventInstance);

  EZ_FMOD_ASSERT(m_pEventInstance->setPaused(false));
  EZ_FMOD_ASSERT(m_pEventInstance->setVolume(m_fVolume));

  EZ_FMOD_ASSERT(m_pEventInstance->start());
}

void ezFmodEventComponent::StartOneShot()
{
  if (!m_hSoundEvent.IsValid())
    return;

  ezResourceLock<ezFmodSoundEventResource> pEvent(m_hSoundEvent, ezResourceAcquireMode::NoFallback);

  if (pEvent->IsMissingResource())
  {
    ezLog::Debug("Cannot start one-shot sound event, resource is missing.");
    return;
  }

  if (pEvent->GetDescriptor() == nullptr)
  {
    ezLog::Debug("Cannot start one-shot sound event, descriptor is null.");
    return;
  }

  bool bIsOneShot = false;
  pEvent->GetDescriptor()->isOneshot(&bIsOneShot);

  // do not start sounds that will not terminate
  if (!bIsOneShot)
  {
    ezLog::Warning("ezFmodEventComponent::StartOneShot: Request ignored, because sound event '{0}' ('{0}') is not a one-shot event.", pEvent->GetResourceID(), pEvent->GetResourceDescription());
    return;
  }

  FMOD::Studio::EventInstance* pEventInstance = pEvent->CreateInstance();

  if (pEventInstance == nullptr)
  {
    ezLog::Debug("Cannot start one-shot sound event, instance could not be created.");
    return;
  }

  SetParameters3d(pEventInstance);
  SetReverbParameters(pEventInstance);

  EZ_FMOD_ASSERT(pEventInstance->setVolume(m_fVolume));

  EZ_FMOD_ASSERT(pEventInstance->start());
  EZ_FMOD_ASSERT(pEventInstance->release());
}

void ezFmodEventComponent::RestartSound(ezFmodEventComponent_RestartSoundMsg& msg)
{
  if (msg.m_bOneShotInstance)
    StartOneShot();
  else
    Restart();
}
void ezFmodEventComponent::StopSound(ezFmodEventComponent_StopSoundMsg& msg)
{
  if (m_pEventInstance != nullptr)
  {
    EZ_FMOD_ASSERT(m_pEventInstance->stop(msg.m_bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT));
  }
}

void ezFmodEventComponent::SoundCue(ezFmodEventComponent_SoundCueMsg& msg)
{
  if (m_pEventInstance != nullptr && m_pEventInstance->isValid())
  {
    EZ_FMOD_ASSERT(m_pEventInstance->triggerCue());
  }
}

ezInt32 ezFmodEventComponent::FindParameter(const char* szName) const
{
  if (!m_hSoundEvent.IsValid())
    return -1;

  ezResourceLock<ezFmodSoundEventResource> pEvent(m_hSoundEvent, ezResourceAcquireMode::NoFallback);

  FMOD::Studio::EventDescription* pEventDesc = pEvent->GetDescriptor();
  if (pEventDesc == nullptr || !pEventDesc->isValid())
    return -1;

  FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
  if (pEventDesc->getParameter(szName, &paramDesc) != FMOD_OK)
    return -1;

  return paramDesc.index;
}

void ezFmodEventComponent::SetParameter(ezInt32 iParamIndex, float fValue)
{
  if (m_pEventInstance == nullptr || iParamIndex < 0 || !m_pEventInstance->isValid())
    return;

  m_pEventInstance->setParameterValueByIndex(iParamIndex, fValue);
}

float ezFmodEventComponent::GetParameter(ezInt32 iParamIndex) const
{
  if (m_pEventInstance == nullptr || iParamIndex < 0 || !m_pEventInstance->isValid())
    return 0.0f;

  float value = 0;
  m_pEventInstance->getParameterValueByIndex(iParamIndex, &value, nullptr);
  return value;
}

void ezFmodEventComponent::Update()
{
  if (m_pEventInstance)
  {
    if (!m_pEventInstance->isValid())
    {
      ezLog::Debug("Fmod instance pointer has been invalidated.");
      m_pEventInstance = nullptr;

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)
      if (m_pSubscripedTo)
      {
        m_pSubscripedTo->m_ResourceEvents.RemoveEventHandler(ezMakeDelegate(&ezFmodEventComponent::ResourceEventHandler, this));
        m_pSubscripedTo = nullptr;
      }
#endif
      return;
    }

    SetParameters3d(m_pEventInstance);

    FMOD_STUDIO_PLAYBACK_STATE state;
    EZ_FMOD_ASSERT(m_pEventInstance->getPlaybackState(&state));

    if (state == FMOD_STUDIO_PLAYBACK_STOPPED)
    {
      ezFmodSoundFinishedEventMessage msg;
      m_SoundFinishedEventSender.SendMessage(msg, this, GetOwner());

      if (m_OnFinishedAction == ezOnComponentFinishedAction::DeleteEntity)
      {
        GetWorld()->DeleteObjectDelayed(GetOwner()->GetHandle());
      }
      else if (m_OnFinishedAction == ezOnComponentFinishedAction::DeleteComponent)
      {
        GetManager()->DeleteComponent(GetHandle());
      }
    }
  }
}

void ezFmodEventComponent::SetParameters3d(FMOD::Studio::EventInstance* pEventInstance)
{
  const auto pos = GetOwner()->GetGlobalPosition();
  const auto vel = GetOwner()->GetVelocity();
  const auto fwd = (GetOwner()->GetGlobalRotation() * ezVec3(1, 0, 0)).GetNormalized();
  const auto up = (GetOwner()->GetGlobalRotation() * ezVec3(0, 0, 1)).GetNormalized();

  FMOD_3D_ATTRIBUTES attr;
  attr.position.x = pos.x;
  attr.position.y = pos.y;
  attr.position.z = pos.z;
  attr.forward.x = fwd.x;
  attr.forward.y = fwd.y;
  attr.forward.z = fwd.z;
  attr.up.x = up.x;
  attr.up.y = up.y;
  attr.up.z = up.z;
  attr.velocity.x = vel.x;
  attr.velocity.y = vel.y;
  attr.velocity.z = vel.z;

  // have to update pitch every time, in case the clock speed changes
  EZ_FMOD_ASSERT(pEventInstance->setPitch(m_fPitch * (float)GetWorld()->GetClock().GetSpeed()));
  EZ_FMOD_ASSERT(pEventInstance->set3DAttributes(&attr));
}

void ezFmodEventComponent::SetReverbParameters(FMOD::Studio::EventInstance* pEventInstance)
{
  if (m_bApplyEnvironmentReverb)
  {
    const ezUInt8 uiNumReverbs = ezFmod::GetSingleton()->GetNumBlendedReverbVolumes();

    for (ezUInt8 i = 0; i < uiNumReverbs; ++i)
    {
      EZ_FMOD_ASSERT(pEventInstance->setReverbLevel(i, 1.0f));
    }

    for (ezUInt8 i = uiNumReverbs; i < 4; ++i)
    {
      EZ_FMOD_ASSERT(pEventInstance->setReverbLevel(i, 0.0f));
    }
  }
  else
  {
    for (ezUInt8 i = 0; i < 4; ++i)
    {
      EZ_FMOD_ASSERT(pEventInstance->setReverbLevel(0, 0.0f));
    }
  }
}

#if EZ_ENABLED(EZ_COMPILE_FOR_DEVELOPMENT)

void ezFmodEventComponent::ResourceEventHandler(const ezResourceEvent& e)
{
  if (m_pEventInstance != nullptr && e.m_EventType == ezResourceEventType::ResourceContentUnloaded)
  {
    m_pEventInstance = nullptr; // pointer is no longer valid!

    m_pSubscripedTo->m_ResourceEvents.RemoveEventHandler(ezMakeDelegate(&ezFmodEventComponent::ResourceEventHandler, this));
    m_pSubscripedTo = nullptr;
  }
}

#endif

//////////////////////////////////////////////////////////////////////////


EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(ezVisualScriptNode_SetFmodEventParameter, 1, ezRTTIDefaultAllocator<ezVisualScriptNode_SetFmodEventParameter>)
{
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Sound")
  }
    EZ_END_ATTRIBUTES
    EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Parameter", GetParameterName, SetParameterName),
    // Execution Pins (Input)
    EZ_INPUT_EXECUTION_PIN("run", 0),
    // Execution Pins (Output)
    EZ_OUTPUT_EXECUTION_PIN("then", 0),
    // Data Pins (Input)
    EZ_INPUT_DATA_PIN("Component", 0, ezVisualScriptDataPinType::ComponentHandle),
    EZ_INPUT_DATA_PIN("Value", 1, ezVisualScriptDataPinType::Number),
  }
  EZ_END_PROPERTIES
}
EZ_END_DYNAMIC_REFLECTED_TYPE

ezVisualScriptNode_SetFmodEventParameter::ezVisualScriptNode_SetFmodEventParameter() { }

void ezVisualScriptNode_SetFmodEventParameter::Execute(ezVisualScriptInstance* pInstance, ezUInt8 uiExecPin)
{
  if (m_iParameterIndex != -2)
  {
    ezFmodEventComponent* pEvent = nullptr;
    if (!pInstance->GetWorld()->TryGetComponent(m_hComponent, pEvent))
      goto failure;

    // index not yet initialized
    if (m_iParameterIndex < 0)
    {
      m_iParameterIndex = pEvent->FindParameter(m_sParameterName.GetData());

      // parameter not found
      if (m_iParameterIndex < 0)
        goto failure;
    }

    pEvent->SetParameter(m_iParameterIndex, (float)m_fValue);
  }

  pInstance->ExecuteConnectedNodes(this, 0);
  return;

failure:
  ezLog::Warning("Script: Fmod Event Parameter '{0}' could not be found. Note that event parameters are not available for one-shot events and events that are not playing.", m_sParameterName.GetString());

  m_iParameterIndex = -2; // make sure we don't try this again
  pInstance->ExecuteConnectedNodes(this, 0);
}

void* ezVisualScriptNode_SetFmodEventParameter::GetInputPinDataPointer(ezUInt8 uiPin)
{
  switch (uiPin)
  {
  case 0:
    return &m_hComponent;

  case 1:
    return &m_fValue;
  }

  return nullptr;
}




EZ_STATICLINK_FILE(FmodPlugin, FmodPlugin_Components_FmodEventComponent);

