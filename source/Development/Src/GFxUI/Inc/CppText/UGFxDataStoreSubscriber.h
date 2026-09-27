// UGFxDataStoreSubscriber cpptext: included inside the generated class body (GFxUIUIPrivateClasses.h).
// Bodies in Src/gfxuinatives.cpp; retail has them in gfxui/src/gfxuidatastore.cpp. Declaring the
// IUIDataStoreSubscriber / IUIDataStorePublisher methods here is what stops gen_classes_header.py --sdk
// emitting its appErrorf pure-virtual stubs for them.
public:
	virtual UBOOL SaveSubscriberValue( TArray<class UUIDataStore*>& out_BoundDataStores, INT BindingIndex = INDEX_NONE ); // 2012 0x5c8df0
	virtual void SetDataStoreBinding( const FString& MarkupText, INT BindingIndex = INDEX_NONE );                         // 2012 gfxuidatastore.cpp
	virtual FString GetDataStoreBinding( INT BindingIndex = INDEX_NONE ) const;                                           // 2012 0x5bf530
	virtual UBOOL RefreshSubscriberValue( INT BindingIndex = INDEX_NONE );                                                // 2012 0x5ce310
	virtual void NotifyDataStoreValueUpdated( class UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, class UUIDataProvider* SourceProvider, INT ArrayIndex ); // 2012 0x5c88a0
	virtual void GetBoundDataStores( TArray<class UUIDataStore*>& out_BoundDataStores );                                  // 2012 0x5c8b30
	virtual void ClearBoundDataStores();                                                                                  // 2012 0x5c8c00
	void PublishValues();                                                                                                 // 2012 0x5c8f80
