#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define BLOCKSIZE 4096
int id = 0;

struct inode * create_file(struct inode * parent, char * name, char readonly, int size_in_bytes)
{
  if (find_inode_by_name(parent, name) != NULL)
  {
    return NULL;
  }
  struct inode * new_file = malloc(sizeof(struct inode));
  if (new_file == NULL)
  {
    fprintf(stderr, "Malloc failed");
    exit(EXIT_FAILURE);
  }
  new_file -> id = id++;
  new_file -> name = strdup(name);
  new_file -> is_directory = 0;
  new_file -> is_readonly = readonly;
  new_file -> filesize = size_in_bytes;
  double numberOfBlocks = (double) size_in_bytes / (double) BLOCKSIZE;
  new_file -> num_entries = ceil(numberOfBlocks);
  new_file -> entries = malloc(sizeof(uintptr_t) * new_file -> num_entries);

  for (int i = 0; i < new_file -> num_entries; i++)
  {
    new_file -> entries[i] = allocate_block();
  }

  if (parent != NULL)
  {
    parent -> entries = realloc(parent -> entries,
      (parent -> num_entries + 1) * sizeof(uintptr_t));

    if (new_file -> entries == NULL) {
      fprintf(stderr, "Malloc failed");
      exit(EXIT_FAILURE);
    }

    parent -> entries[parent -> num_entries] = (uintptr_t) new_file;
    parent -> num_entries++;
  }
  return parent;
}

struct inode * create_dir(struct inode * parent, char * name)
{
  if (find_inode_by_name(parent, name) != NULL)
  {
    return NULL;
  }
  struct inode * new_dir = malloc(sizeof(struct inode));
  if (new_dir == NULL)
  {
    fprintf(stderr, "Malloc failed");
    exit(EXIT_FAILURE);
  }

  new_dir -> id = id++;
  new_dir -> name = strdup(name);
  new_dir -> is_directory = 1;
  new_dir -> is_readonly = 0;
  new_dir -> filesize = 0;
  new_dir -> num_entries = 0;
  new_dir -> entries = malloc(64);

  if (parent != NULL)
  {
    parent -> entries = realloc(parent -> entries,
      (parent -> num_entries + 1) * sizeof(uintptr_t));

    if (new_dir -> entries == NULL) {
      fprintf(stderr, "Malloc failed");
      exit(EXIT_FAILURE);
    }

    parent -> entries[parent -> num_entries] = (uintptr_t) new_dir;
    parent -> num_entries++;
  }
  return new_dir;
}


struct inode *find_inode_by_name(struct inode *parent, char *name)
{
    if (parent == NULL)
        return NULL;
    if (!strcmp(parent->name, name))
    {
        return parent;
    }
    if (parent->is_directory)
    {
        for (int i = 0; i < parent->num_entries; i++)
        {
            struct inode *child = (struct inode *)parent->entries[i];
            if (strcmp(child->name, name) == 0)
            {
                return child;
            }
        }
    }
    return NULL;
}

struct inode *loadNodesHelper(FILE *fp)
{
    struct inode *node = malloc(sizeof(struct inode));
    if (node == NULL)
    {
        fprintf(stderr, "Malloc failed");
        exit(EXIT_FAILURE);
    }

    char name_len[sizeof(int)];
    fread(&node->id, sizeof(int), 1, fp);
    fread(name_len, sizeof(int), 1, fp);
    node->name = malloc((int)*name_len);
    fread(node->name, *name_len, 1, fp);
    fread(&node->is_directory, sizeof(char), 1, fp);
    fread(&node->is_readonly, sizeof(char), 1, fp);
    fread(&node->filesize, sizeof(int), 1, fp);
    fread(&node->num_entries, sizeof(int), 1, fp);
    uintptr_t *oppforing = malloc(sizeof(uintptr_t) * node->num_entries);
    node->entries = malloc(64 * node->num_entries);
    fread(oppforing, node->num_entries * sizeof(uintptr_t), 1, fp);


    for (int i = 0; i < node->num_entries; i++) 
    {
        if (node->is_directory) {
            struct inode *child = loadNodesHelper(fp);
            node->entries[i] = (uintptr_t)child;
    }
        else {
        node->entries[i] = (uintptr_t)oppforing[i];
        }
    }


    free(oppforing);
    return node;
}

struct inode *load_inodes()
{
    FILE *fp;
    fp = fopen("master_file_table", "rb");
    if (fp == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    struct inode *root = loadNodesHelper(fp);
    fclose(fp);

    return root;
}

void fs_shutdown(struct inode *node)
{
    if (!node)
        return;
    if (node->is_directory)
    {
        for (int i = 0; i < node->num_entries; i++)
        {
            fs_shutdown((struct inode *)node->entries[i]);
        }
    }
    free(node->name);
    free(node->entries);
    free(node);
}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

void debug_fs(struct inode *node)
{
    if (node == NULL)
        return;
    for (int i = 0; i < indent; i++)
        printf("  ");
    if (node->is_directory)
    {
        printf("%s (id %d)\n", node->name, node->id);
        indent++;
        for (int i = 0; i < node->num_entries; i++)
        {
            struct inode *child = (struct inode *)node->entries[i];
            debug_fs(child);
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize);
        for (int i = 0; i < node->num_entries; i++)
        {
            printf("%d ", (int)node->entries[i]);
        }
        printf(")\n");
    }
}