# Archive version comparisons in serialization code (2012 Shipping)

Scanned 1723 serialization-like functions (0 failed to decompile). 231 comparisons: 14 licensee, 217 engine version.

## Licensee thresholds seen

0, 10, 24, 25, 26, 27, 28, 29, 30

## Licensee comparisons per module

| Module | Comparisons |
|---|---:|
| engine | 8 |
| dishonoredgame | 5 |
| core | 1 |

## Functions with licensee branches (13) — port verbatim before milestone 3

- `core` `public: virtual void __thiscall UClass::Serialize(class FArchive &)`
- `dishonoredgame` `public: virtual void __thiscall UDisTweaks_Attributes::Serialize(class FArchive &)`
- `dishonoredgame` `public: virtual void __thiscall UDisTweaks_SkeletalBreakable::Serialize(class FArchive &)`
- `dishonoredgame` `public: virtual void __thiscall UDisTweaks_StaticBreakable::Serialize(class FArchive &)`
- `dishonoredgame` `public: virtual void __thiscall UDisTweaks_UsableObject::Serialize(class FArchive &)`
- `engine` `class FArchive & __cdecl operator<<(class FArchive &, struct FNavMeshPolyBase &)`
- `engine` `private: virtual void __thiscall USeqEvent_Touch::Serialize(class FArchive &)`
- `engine` `public: virtual void __thiscall AActor::Serialize(class FArchive &)`
- `engine` `public: virtual void __thiscall UDecalMaterial::Serialize(class FArchive &)`
- `engine` `public: virtual void __thiscall ULevel::Serialize(class FArchive &)`
- `engine` `public: virtual void __thiscall USkeletalMeshComponent::Serialize(class FArchive &)`
- `engine` `public: virtual void __thiscall UWorld::Serialize(class FArchive &)`
- `engine` `public: void __thiscall FMaterial::Serialize(class FArchive &)`
