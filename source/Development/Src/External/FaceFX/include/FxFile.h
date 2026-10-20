/*=============================================================================
	FxFile.h - FaceFX 1.7.3.1 stdio file wrapper (fxfile.h). 8 bytes: the FILE* and the length measured
	when the file is opened for reading.

	DISHONORED(port): agent FX1 (wave 26).
=============================================================================*/
#ifndef FxFile_H__
#define FxFile_H__

#include "FxPlatform.h"

namespace OC3Ent
{
namespace Face
{

class FxFile
{
public:
	enum FxFileMode
	{
		FM_Read = 1,
		FM_Write = 2,
		FM_Text = 4,
		FM_Binary = 8
	};

	FxFile();
	FxFile(const FxChar* filename, FxInt32 mode);
	~FxFile();

	static FxBool Exists(const FxChar* filename);
	static FxByte* ReadBinaryFile(const FxChar* filename, FxSize& numBytes);

	FxBool IsValid() const;
	FxBool Open(const FxChar* filename, FxInt32 mode);
	void Close();
	FxSize Read(FxByte* data, FxSize numBytes);
	FxSize Write(const FxByte* data, FxSize numBytes);
	FxInt32 Tell() const;
	void Seek(FxInt32 pos);
	FxInt32 Length() const;

private:
	FxFile(const FxFile&);
	FxFile& operator=(const FxFile&);

	void* _file;
	FxInt32 _length;
};

} // namespace Face
} // namespace OC3Ent

#endif
