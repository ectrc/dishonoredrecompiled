/*=============================================================================
	FxFile.cpp - FaceFX 1.7.3.1 stdio file wrapper (fxsdk\src\fxfile.cpp, compiland FxFile.obj).

	DISHONORED(port): agent FX1 (wave 26). The mode string is "r"/"w" + "b"/"t" (fopen_s); the length is only
	measured for FM_Read, and the (filename, mode) constructor leaves it unset otherwise, as retail (0xb2afa0).
=============================================================================*/
#include "FxFile.h"
#include "FxMemory.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb2af70. */
FxFile::FxFile()
	: _file(NULL)
	, _length(0)
{
}

/** 2012 0xb2afa0. */
FxFile::FxFile(const FxChar* filename, FxInt32 mode)
	: _file(NULL)
{
	Open(filename, mode);
}

/** 2012 0xb2afd0. */
FxFile::~FxFile()
{
	Close();
}

/** 2012 0xb2aff0. */
FxBool FxFile::Exists(const FxChar* filename)
{
	FxFile file(filename, FM_Read);
	return file.IsValid();
}

/** 2012 0xb2b060. */
FxByte* FxFile::ReadBinaryFile(const FxChar* filename, FxSize& numBytes)
{
	FxFile file(filename, FM_Read | FM_Binary);
	FxSize fileLength = 0;
	FxByte* retval = NULL;
	if( file.IsValid() )
	{
		fileLength = static_cast<FxSize>(file.Length());
		retval = static_cast<FxByte*>(FxAllocate(fileLength));
		file.Read(retval, fileLength);
	}
	numBytes = fileLength;
	return retval;
}

/** 2012 0xb2b110. */
FxBool FxFile::IsValid() const
{
	return _file != NULL;
}

/** 2012 0xb2b130. */
FxBool FxFile::Open(const FxChar* filename, FxInt32 mode)
{
	FxChar filemode[4];
	filemode[0] = (mode & FM_Write) ? 'w' : 'r';
	filemode[1] = (mode & FM_Binary) ? 'b' : 't';
	filemode[2] = 0;
	FILE* file = NULL;
	fopen_s(&file, filename, filemode);
	_file = file;
	if( IsValid() && (mode & FM_Read) )
	{
		fseek(static_cast<FILE*>(_file), 0, SEEK_END);
		_length = ftell(static_cast<FILE*>(_file));
		fseek(static_cast<FILE*>(_file), 0, SEEK_SET);
	}
	return IsValid();
}

/** 2012 0xb2b1e0. */
void FxFile::Close()
{
	if( _file )
	{
		fclose(static_cast<FILE*>(_file));
	}
	_file = NULL;
}

/** 2012 0xb2b210. */
FxSize FxFile::Read(FxByte* data, FxSize numBytes)
{
	if( _file )
	{
		return static_cast<FxSize>(fread(data, 1, numBytes, static_cast<FILE*>(_file)));
	}
	return 0;
}

/** 2012 0xb2b250. */
FxSize FxFile::Write(const FxByte* data, FxSize numBytes)
{
	if( _file )
	{
		return static_cast<FxSize>(fwrite(data, 1, numBytes, static_cast<FILE*>(_file)));
	}
	return 0;
}

/** 2012 0xb2b290. */
FxInt32 FxFile::Tell() const
{
	if( _file )
	{
		return ftell(static_cast<FILE*>(_file));
	}
	return 0;
}

/** 2012 0xb2b2c0. */
void FxFile::Seek(FxInt32 pos)
{
	if( _file )
	{
		fseek(static_cast<FILE*>(_file), pos, SEEK_SET);
	}
}

/** 2012 0xb2b2f0. */
FxInt32 FxFile::Length() const
{
	return _length;
}

} // namespace Face
} // namespace OC3Ent
