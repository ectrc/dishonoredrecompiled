// FDisConvSaveData_DialogTree cpptext: included inside the generated struct body (DishonoredGameGlobalEnumsClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The SDK dump left m_ConvData as a 12-byte gap; FDisConvSaveData::FindConvData
// (2013 rva 0x8a30a0) reads it as the TArray of 28-byte FDisConvSaveData_Conversation records. Same 12 bytes, typed here.
	TArrayNoInit<struct FDisConvSaveData_Conversation>& ConvData() { return *(TArrayNoInit<struct FDisConvSaveData_Conversation>*)m_ConvData; }
