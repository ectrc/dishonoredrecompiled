#pragma once
// Engine/inc/arkrequestmanager.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (25):
//   0x576b60  public: int __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::GetRequestIdxByID(int)const
//   0x576bd0  public: int __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::AddRequest(void const * const, class FName, int, struct FArkComponentFaceTo::FFaceToRequestData const &, float)
//   0x576d80  public: void __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::UpdateRequestByIdx(int, struct FArkComponentFaceTo::FFaceToRequestData const &)
//   0x578ce0  public: void __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::RemoveRequestByIdx(int)
//   0x578d80  public: unsigned int __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::RemoveAllRequestsFromAsker(void const * const)
//   0x57a940  public: void __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::Initialize(class FArkComponentBase * const, void (__thiscall FArkComponentBase::*)(enum FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::EArkReqMgrEvent, int))
//   0x57a9b0  public: unsigned int __thiscall FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::Update(float)
//   0x583b80  public: int __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::GetRequestIdxByID(int)const
//   0x583bf0  public: int __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::AddRequest(void const * const, class FName, int, struct FArkComponentLocomotion::FLocoRequestData const &, float)
//   0x583da0  public: void __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::UpdateRequestByIdx(int, struct FArkComponentLocomotion::FLocoRequestData const &)
//   0x586ac0  public: void __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::RemoveRequestByIdx(int)
//   0x586b60  public: unsigned int __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::RemoveAllRequestsFromAsker(void const * const)
//   0x588690  public: void __thiscall FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::Initialize(class FArkComponentBase * const, void (__thiscall FArkComponentBase::*)(enum FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::EArkReqMgrEvent, int))
//   0x594600  public: int __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::GetRequestIdxByID(int)const
//   0x594670  public: int __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::AddRequest(void const * const, class FName, int, struct FArkComponentLookat::FLookatRequestData const &, float)
//   0x595e70  public: void __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::RemoveRequestByIdx(int)
//   0x595f10  public: unsigned int __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::RemoveAllRequestsFromAsker(void const * const)
//   0x595f80  private: void __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::Reset(void)
//   0x5963e0  public: unsigned int __thiscall FArkRequestManager<struct FArkComponentLookat::FLookatRequestData>::Update(float)
//   0x8bb700  public: int __thiscall FArkRequestManager<struct FLodRequest>::AddRequest(void const * const, class FName, int, struct FLodRequest const &, float)
//   0x8bb8d0  public: int __thiscall FArkRequestManager<struct FLodRequest>::GetRequestIdxByID(int)const
//   0x8bb940  public: void __thiscall FArkRequestManager<struct FLodRequest>::RemoveRequestByIdx(int)
//   0x8bf9c0  public: unsigned int __thiscall FArkRequestManager<struct FLodRequest>::Update(float)
//   0x8bfa70  public: unsigned int __thiscall FArkRequestManager<struct FLodRequest>::RemoveAllRequestsFromAsker(void const * const)
//   0x8bfc80  private: void __thiscall FArkRequestManager<struct FLodRequest>::Reset(void)
