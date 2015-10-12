#if 0

// https://github.com/libuv/libuv/blob/v1.x/src/unix/darwin.c

int uv_exepath(char* buffer, size_t* size) {
  /* realpath(exepath) may be > PATH_MAX so double it to be on the safe side. */
  char abspath[PATH_MAX * 2 + 1];
  char exepath[PATH_MAX + 1];
  uint32_t exepath_size;
  size_t abspath_size;

  if (buffer == NULL || size == NULL || *size == 0)
    return -EINVAL;

  exepath_size = sizeof(exepath);
  if (_NSGetExecutablePath(exepath, &exepath_size))
    return -EIO;

  if (realpath(exepath, abspath) != abspath)
    return -errno;

  abspath_size = strlen(abspath);
  if (abspath_size == 0)
    return -EIO;

  *size -= 1;
  if (*size > abspath_size)
    *size = abspath_size;

  memcpy(buffer, abspath, *size);
  buffer[*size] = '\0';

  return 0;
}

#endif
