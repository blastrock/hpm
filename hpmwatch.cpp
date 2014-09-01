/*
 * Copyright (C) 2014 Philippe Daouadi <p.daouadi@free.fr>
 * Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: installwatch.c,v 0.6.3.2 2001/12/22 23:01:20 izto Exp $
 *
 * april-15-2001 - Modifications by Felipe Eduardo Sanchez Diaz Duran
 *                                  <izto@asic-linux.com.mx>
 * Added backup() and make_path() functions.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include <mutex>

#define BUFSIZE 4096

#define error(X) (X < 0 ? strerror(errno) : "success")

std::once_flag initialized;

static int(*true_creat)(const char *, mode_t);
static FILE *(*true_fopen)(const char *,const char*);
static int(*true_link)(const char *, const char *);
static int(*true_open)(const char *, int, ...);
static int(*true_rename)(const char *, const char *);
static int(*true_symlink)(const char *, const char *);

static int(*true_creat64)(const char *, mode_t);
static FILE *(*true_fopen64)(const char *,const char *);
static int(*true_open64)(const char *, int, ...);

static void addfile(const char *filename);

static int log_fd = -1;

int initialize_watch(void) {
  void *libc_handle;

  libc_handle = RTLD_NEXT;


  true_creat = (int(*)(const char*, mode_t))dlsym(libc_handle, "creat");
  true_fopen = (FILE*(*)(const char*, const char*))dlsym(libc_handle, "fopen");
  true_link = (int(*)(const char*, const char*))dlsym(libc_handle, "link");
  true_open = (int(*)(const char*, int, ...))dlsym(libc_handle, "open");
  true_rename = (int(*)(const char*, const char*))dlsym(libc_handle, "rename");
  true_symlink = (int(*)(const char*, const char*))dlsym(libc_handle, "symlink");

  true_creat64 = (int(*)(const char*, __mode_t))dlsym(libc_handle, "creat64");
  true_fopen64 = (FILE*(*)(const char*, const char*))dlsym(libc_handle, "fopen64");
  true_open64 = (int(*)(const char*, int, ...))dlsym(libc_handle, "open64");

  if (!true_creat ||
      !true_fopen ||
      !true_link ||
      !true_open ||
      !true_rename ||
      !true_symlink ||
      !true_creat64 ||
      !true_fopen64 ||
      !true_open64)
  {
    puts("Error: cannot load one of the symbols");
    exit(-1);
  }

  char* logname = getenv("INSTALLWATCHFILE");
  if (!logname)
  {
    puts("Error: INSTALLWATCHFILE not defined");
    exit(-1);
  }
  log_fd = true_open(logname, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (log_fd == -1)
  {
    perror("cannot open logfile");
    exit(-1);
  }
}

static int forceinit = initialize_watch();

static void addfile(const char *filename) {
  char buffer[BUFSIZE];
  int count;

  count = snprintf(buffer, BUFSIZE, "%s\n", filename);
  if(count == -1) {
    /* The buffer was not big enough */
    strcpy(&(buffer[BUFSIZE - 5]), "...\n");
    count = BUFSIZE - 1;
  }
  if(write(log_fd, buffer, count) != count)
    perror("could not write in logs");
}

static void canonicalize(const char *path, char *resolved_path) {
  if(!realpath(path, resolved_path) && (path[0] != '/')) {
    /* The path could not be canonicalized, append it
     * to the current working directory if it was not
     * an absolute path                               */
    getcwd(resolved_path, MAXPATHLEN - 2);
    strcat(resolved_path, "/");
    strncat(resolved_path, path, MAXPATHLEN - 1);
  }
}

int creat(const char *pathname, mode_t mode) {
  call_once(initialized, initialize_watch);

  /* Is it a system call? */
  int result;
  char canonic[MAXPATHLEN];

  canonicalize(pathname, canonic);

  result = true_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
  if (result != -1)
    addfile(canonic);
  return result;
}

FILE *fopen(const char *pathname, const char *mode) {
  call_once(initialized, initialize_watch);

  FILE *result;
  char canonic[MAXPATHLEN];

  canonicalize(pathname, canonic);

  result = true_fopen(pathname,mode);
  if ((mode[0] == 'w' ||
        mode[0] == 'a' ||
        (mode[0] != '\0' && mode[1]=='+')) &&
      result)
    addfile(canonic);
  return result;
}

int link(const char *oldpath, const char *newpath) {
  call_once(initialized, initialize_watch);

  int result;
  char old_canonic[MAXPATHLEN], new_canonic[MAXPATHLEN];

  canonicalize(oldpath, old_canonic);
  canonicalize(newpath, new_canonic);
  result = true_link(oldpath, newpath);
  if (result != -1)
    addfile(new_canonic);
  return result;
}

int open(const char *pathname, int flags, ...) {
  call_once(initialized, initialize_watch);

  /* Eventually, there is a third parameter: it's mode_t mode */
  va_list ap;
  mode_t mode;
  int result;
  char canonic[MAXPATHLEN];

  va_start(ap, flags);
  mode = va_arg(ap, mode_t);
  va_end(ap);
  canonicalize(pathname, canonic);

  result = true_open(pathname, flags, mode);
  if ((flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC)) && result != -1)
    addfile(canonic);
  return result;
}

int rename(const char *oldpath, const char *newpath) {
  call_once(initialized, initialize_watch);

  int result;
  char old_canonic[MAXPATHLEN], new_canonic[MAXPATHLEN];

  canonicalize(oldpath, old_canonic);

  canonicalize(newpath, new_canonic);
  result = true_rename(oldpath, newpath);
  if (result != -1)
    addfile(new_canonic);
  return result;
}

int symlink(const char *oldpath, const char *newpath) {
  call_once(initialized, initialize_watch);

  int result;
  char old_canonic[MAXPATHLEN], new_canonic[MAXPATHLEN];

  canonicalize(oldpath, old_canonic);
  canonicalize(newpath, new_canonic);
  result = true_symlink(oldpath, newpath);
  if (result != -1)
    addfile(new_canonic);
  return result;
}

int creat64(const char *pathname, mode_t mode) {
  call_once(initialized, initialize_watch);

  /* Is it a system call? */
  int result;
  char canonic[MAXPATHLEN];

  canonicalize(pathname, canonic);

  result = true_open64(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
  if (result != -1)
    addfile(canonic);
  return result;
}

FILE *fopen64(const char *pathname, const char *mode) {
  call_once(initialized, initialize_watch);

  FILE *result;
  char canonic[MAXPATHLEN];

  canonicalize(pathname, canonic);

  result = true_fopen(pathname,mode);
  if ((mode[0] == 'w' ||
        mode[0] == 'a' ||
        (mode[0] != '\0' && mode[1]=='+')) &&
      result)
    addfile(canonic);
  return result;
}

int open64(const char *pathname, int flags, ...) {
  call_once(initialized, initialize_watch);

  /* Eventually, there is a third parameter: it's mode_t mode */
  va_list ap;
  mode_t mode;
  int result;
  char canonic[MAXPATHLEN];

  va_start(ap, flags);
  mode = va_arg(ap, mode_t);
  va_end(ap);
  canonicalize(pathname, canonic);
  result = true_open64(pathname, flags, mode);
  if ((flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC)) && result != -1)
    addfile(canonic);
  return result;
}
