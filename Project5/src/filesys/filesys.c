#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#ifdef FILESYS
#include "cache.h"
#endif

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);
static bool approach_leaf(const char *, struct dir **, char *);

/* Parses 'path' and populates last filename and cotaining dir. */
static bool approach_leaf(const char *path, 
                          struct dir **containing_dir, 
                          char *filename){
  int path_len = strnlen(path, MAX_PATH_LEN + 1);
  if (path_len == 0 || path_len > MAX_PATH_LEN)  
    return false; 
  
  /* Local copy to feed strtok which messes up provided string */
  char path_copy[MAX_PATH_LEN + 1];
  strlcpy(path_copy, path, MAX_PATH_LEN + 1);

  char *cur_token = NULL;
  char last_identifier[MAX_PATH_LEN + 1];
  struct dir *cur_dir = NULL;
  /* Decide if path is in absolute or relative form. */
  if (path[0] == PATH_DELIM_CHAR) 
    cur_dir = dir_open_root();
  else 
    cur_dir = dir_reopen(thread_current()->pwd);
  
  char *state; // internal state of strtok_r
  cur_token = strtok_r(path_copy, PATH_DELIM_STRING, &state);
  /* Should not end on directory */
  if (cur_token == NULL)
    return false;
  strlcpy(last_identifier, cur_token, MAX_PATH_LEN + 1);

  while (true){
    cur_token = strtok_r(NULL, PATH_DELIM_STRING, &state);
    if (cur_token == NULL) 
      break;
    struct inode *inode = NULL;
    bool is_in_dir = dir_lookup(cur_dir, last_identifier, &inode);
    dir_close(cur_dir);
    if (!is_in_dir)
      return false;
    cur_dir = dir_open(inode);
    if (cur_dir == NULL)
      return false;
    strlcpy(last_identifier, cur_token, MAX_PATH_LEN + 1);
  }

  /* Set last dir and filename variables for caller. */
  *containing_dir = cur_dir;
  strlcpy(filename, last_identifier, MAX_PATH_LEN + 1);
  return true;
}

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  sector_cache_flush(true);
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *path, off_t initial_size, bool is_dir) 
{
  char filename[NAME_MAX + 1];
  struct dir *containing_dir;
  if (!approach_leaf(path, &containing_dir, filename))
    return false;

  ASSERT (containing_dir != NULL);

  block_sector_t inode_sector = 0;
  bool success = (containing_dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && ( is_dir ? dir_create   (inode_sector, inode_get_inumber(dir_get_inode(containing_dir)),  initial_size) :
                                inode_create (inode_sector, initial_size, is_dir) )
                  && dir_add (containing_dir, filename, inode_sector));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (containing_dir);
  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *path)
{
  /* Special case if path is root dir. */
  if (strcmp(path, PATH_DELIM_STRING) == 0)
    return (struct file *)dir_open_root();

  char filename[NAME_MAX + 1];
  struct dir *containing_dir;
  if (!approach_leaf(path, &containing_dir, filename))
    return NULL;

  ASSERT (containing_dir != NULL);
  struct inode *inode = NULL;
  dir_lookup(containing_dir, filename, &inode);
  dir_close (containing_dir);

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *path) 
{
  
  char filename[NAME_MAX + 1];
  struct dir *containing_dir;
  if (!approach_leaf(path, &containing_dir, filename))
    return false;

  ASSERT (containing_dir != NULL);

  // Don't touch directories yet
  struct inode *inode = NULL;
  dir_lookup(containing_dir, filename, &inode);
  if (inode_is_dir(inode)){
    dir_close(containing_dir);
    return false;
  }

  bool success = dir_remove(containing_dir, filename);
  dir_close (containing_dir);

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}
