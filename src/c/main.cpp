#include <cstring>
#include "dropbox.h"

using namespace std;

int main(int argc, char const* argv[])
{
  if (argc < 2) {
    dropbox::usage(argv[0]);
    return 1;
  } else {
    int r;
    if (strcmp(argv[1], "d") == 0 || strcmp(argv[1], "download") == 0 || strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--download") == 0) {
      if (argc != 3) {
        dropbox::usage(argv[0]);
        return 1;
      }
      r = dropbox::download(argv[2]);
    } else if (strcmp(argv[1], "u") == 0 || strcmp(argv[1], "upload") == 0 || strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--upload") == 0) {
      if (argc == 3) {
        r = dropbox::upload(argv[2], "");
      } else if (argc == 4) {
        r = dropbox::upload(argv[2], argv[3]);
      } else {
        dropbox::usage(argv[0]);
        return 1;
      }
    } else if (strcmp(argv[1], "r") == 0 || strcmp(argv[1], "remove") == 0 || strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--remove") == 0) {
      if (argc < 3) {
        dropbox::usage(argv[0]);
        return 1;
      }
      for (int i = 2; i < argc - 1; ++i) {
        r = dropbox::delete_v2(argv[i]);
      }
    } else if (strcmp(argv[1], "m") == 0 || strcmp(argv[1], "move") == 0 || strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "--move") == 0) {
      if (argc < 4) {
        dropbox::usage(argv[0]);
        return 1;
      } else if (argc == 4) {
        r = dropbox::move_v2(argv[2], argv[3]);
      } else {
        dropbox::usage(argv[0]);
        return 1;
        // argv[2] ... argv[argc - 2] argv[argc - 1]
        // r = dropbox::move_batch_v2(argv + 2, argc - 3, argv[argc - 1]);
      }
    } else if (strcmp(argv[1], "f") == 0 || strcmp(argv[1], "mkdir") == 0 || strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--mkdir") == 0) {
      if (argc != 3) {
        dropbox::usage(argv[0]);
        return 1;
      }
      r = dropbox::create_folder_v2(argv[2]);
    } else if (strcmp(argv[1], "h") == 0 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      dropbox::usage(argv[0]);
      return 0;
    } else {
      dropbox::usage(argv[0]);
      return 1;
    }
    return r;
  }

  return 0;
}
