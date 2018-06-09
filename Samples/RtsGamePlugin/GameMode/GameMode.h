#pragma once

#include <PCH.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <Core/Input/Declarations.h>

class ezWorld;
class ezCamera;
class RtsGameState;

class RtsGameMode
{
public:
  RtsGameMode();
  virtual ~RtsGameMode();

  void ActivateMode(ezWorld* pMainWorld, ezViewHandle hView, ezCamera* pMainCamera);
  void DeactivateMode();
  void ProcessInput(ezUInt32 uiMousePosX, ezUInt32 uiMousePosY, ezKeyState::Enum LeftClickState, ezKeyState::Enum RightClickState);
  void BeforeWorldUpdate();

  //////////////////////////////////////////////////////////////////////////
  // Game Mode Interface
public:
  virtual void AfterProcessInput() {}

protected:
  virtual void OnActivateMode() {}
  virtual void OnDeactivateMode() {}
  virtual void RegisterInputActions() {}
  virtual void OnProcessInput() {}
  virtual void OnBeforeWorldUpdate() {}

  RtsGameState* m_pGameState = nullptr;
  ezWorld* m_pMainWorld = nullptr;
  ezViewHandle m_hMainView;

private:
  bool m_bInitialized = false;

  //////////////////////////////////////////////////////////////////////////
  // Camera
protected:
  void DoDefaultCameraInput();

  ezCamera* m_pMainCamera = nullptr;

  //////////////////////////////////////////////////////////////////////////
  // Input

protected:
  ezUInt32 m_uiMousePosX;
  ezUInt32 m_uiMousePosY;
  ezKeyState::Enum m_LeftClickState;
  ezKeyState::Enum m_RightClickState;

};
