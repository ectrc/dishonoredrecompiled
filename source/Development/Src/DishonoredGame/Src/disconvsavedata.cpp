// DishonoredGame/src/disconvsavedata.cpp
// DISHONORED(port): agent FR (PHASE17 FR). FDisConvSaveData's finders, keyed by object path name: FindDialogData 2013 rva
// 0x8a2f30 (2012 0x8f2210, 357 B), FindConvData 0x8a30a0 (0x8f2390, 375 B), FindLabelData (0x8f2520, 282 B). Retail compiles
// them beside the conversation manager; they get their own unit here because nothing else of that file is built.
#include "DishonoredGame.h"

FDisConvSaveData_DialogTree* FDisConvSaveData::FindDialogData( const UDisDialogTree* _pTree, UBOOL _bAddIfNeeded )
{
	if( !_pTree )
	{
		return NULL;
	}
	const FString TreeName = _pTree->GetPathName();
	for( INT i = 0; i < m_DialogData.Num(); i++ )
	{
		if( m_DialogData(i).m_DialogTree == TreeName )
		{
			return &m_DialogData(i);
		}
	}
	if( !_bAddIfNeeded )
	{
		return NULL;
	}
	FDisConvSaveData_DialogTree& Data = m_DialogData( m_DialogData.AddZeroed() );
	Data.m_DialogTree = TreeName;
	return &Data;
}

FDisConvSaveData_Conversation* FDisConvSaveData::FindConvData( const UDisDialogTree* _pTree, const UDisConversation* _pConv, UBOOL _bAddIfNeeded )
{
	FDisConvSaveData_DialogTree* DialogData = FindDialogData( _pTree, _bAddIfNeeded );
	if( !DialogData || !_pConv )
	{
		return NULL;
	}
	const FString ConvName = _pConv->GetPathName();
	TArrayNoInit<FDisConvSaveData_Conversation>& ConvData = DialogData->ConvData();
	for( INT i = 0; i < ConvData.Num(); i++ )
	{
		if( ConvData(i).m_Conversation == ConvName )
		{
			return &ConvData(i);
		}
	}
	if( !_bAddIfNeeded )
	{
		return NULL;
	}
	FDisConvSaveData_Conversation& Data = ConvData( ConvData.AddZeroed() );
	Data.m_Conversation = ConvName;
	return &Data;
}

FDisConvSaveData_Label* FDisConvSaveData::FindLabelData( const UDisDialogTree* _pTree, const UDisConversation* _pConv, const FName& _rLabel, UBOOL _bAddIfNeeded )
{
	FDisConvSaveData_Conversation* ConvData = FindConvData( _pTree, _pConv, _bAddIfNeeded );
	if( !ConvData )
	{
		return NULL;
	}
	for( INT i = 0; i < ConvData->m_LabelData.Num(); i++ )
	{
		if( ConvData->m_LabelData(i).m_Label == _rLabel )
		{
			return &ConvData->m_LabelData(i);
		}
	}
	if( !_bAddIfNeeded )
	{
		return NULL;
	}
	FDisConvSaveData_Label& Data = ConvData->m_LabelData( ConvData->m_LabelData.AddZeroed() );
	Data.m_Label = _rLabel;
	return &Data;
}
