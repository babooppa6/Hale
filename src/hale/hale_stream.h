#ifndef HALE_STREAM_H
#define HALE_STREAM_H

#if HALE_INCLUDES
#include "hale.h"
#endif

namespace hale {

inline err
open(File *file, const ch8 *path, u32 flags)
{
    return platform.open_file(file, path, flags);
}

inline err
open(File *file, const ch16 *path, u32 flags)
{
    return platform.open_file(file, path, flags);
}

inline void
close(File *file)
{
    platform.close_file(file);
}

inline memi
size(File *file)
{
    return platform.get_file_size_64(file);
}

inline memi
seek(File *file, memi pos)
{
    return platform.seek_file(file, pos);
}

inline memi
read(File *file, const void *data, memi size)
{
    return platform.read_file(file, data, size);
}

inline memi
write(File *file, const void *data, memi size)
{
    return platform.write_file(file, data, size);
}

} // namespace hale

#endif // HALE_STREAM_H
