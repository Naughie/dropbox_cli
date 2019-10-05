#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <libgen.h>
#include <cstdlib>

using namespace std;

namespace dropbox {
  static const char ACCESS_TOKEN[] = "DROPBOX_ACCESS_TOKEN";
  static const char TOKEN_TYPE[]   = "Bearer";

  static const char RPC_URL[]     = "https://api.dropboxapi.com";
  static const char CONTENT_URL[] = "https://content.dropboxapi.com";
  static const char API_PREFIX[]  = "/2";

  static const char AUTHORIZATION[]   = "Authorization: ";
  static const char CONTENT_TYPE[]    = "Content-Type: ";
  static const char DROPBOX_API_ARG[] = "Dropbox-API-Arg: ";

  static const char TEXT_PLAIN[]   = "text/plain; charset=utf-8";
  static const char OCTET_STREAM[] = "application/octet-stream";
  static const char JSON[]         = "application/json";

  using CALLBACK = size_t (*)(char*, size_t, size_t, void*);
  using HEADERS = vector<string>;

  size_t callback(char*, size_t, size_t, void*);

  int perform(string&, HEADERS*, string&, ostream&, CALLBACK);

  int download(const char*);
  int upload(const char*, const char*);
  int delete_v2(const char*);

  void usage(const char*);
}

size_t dropbox::callback(char *ptr, size_t size, size_t nmemb, void *stream) {
  size_t block = size * nmemb;
  vector<char> *buf = (vector<char>*)stream;
  for (char *p = ptr; p < ptr + block; buf->push_back(*(p++))) {}
  return block;
}

int dropbox::perform(string &url, dropbox::HEADERS *headers, string &post_body, ostream &ofs, CALLBACK callback) {
  // Setup

  CURL *curl = curl_easy_init();
  if (curl == nullptr) {
    cerr << "Could not initialize curl" << endl;
    curl_easy_cleanup(curl);
    return 1;
  }

  vector<char> buf;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropbox::callback);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

  // Headers

  const char *token = getenv(dropbox::ACCESS_TOKEN);
  if (token == NULL) {
    cerr << "Please set the environment variable " << dropbox::ACCESS_TOKEN << "." << endl;
    return 1;
  }
  string header_auth(dropbox::AUTHORIZATION);
  header_auth += dropbox::TOKEN_TYPE;
  header_auth += " ";
  header_auth += token;
  headers->push_back(header_auth);

  curl_slist *chunk = NULL;

  for (auto s : *headers) {
    chunk = curl_slist_append(chunk, s.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  // Post data

  curl_easy_setopt(curl, CURLOPT_POST, 1);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_body.size());

  // Execute

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    cerr << curl_easy_strerror(res);
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
    return 1;
  }

  // Output

  curl_easy_cleanup(curl);

  curl_slist_free_all(chunk);

  for (auto c : buf) {
    ofs << c;
  }

  return 0;
}

int dropbox::download(const char *fname) {
  const char *basefname = basename((char *)fname);
  ofstream ofs(basefname);

  string url(dropbox::CONTENT_URL);
  url += dropbox::API_PREFIX;
  url += "/files/download";

  string post_body;

  dropbox::HEADERS headers;

  string dropbox_api_arg(dropbox::DROPBOX_API_ARG);
  dropbox_api_arg += "{\"path\": \"/";
  dropbox_api_arg += fname;
  dropbox_api_arg += "\"}";
  headers.push_back(dropbox_api_arg);

  string content_type(dropbox::CONTENT_TYPE);
  content_type += dropbox::TEXT_PLAIN;
  headers.push_back(content_type);

  return dropbox::perform(url, &headers, post_body, ofs, dropbox::callback);
}

int dropbox::upload(const char *fname, const char *dir) {
  string url(dropbox::CONTENT_URL);
  url += dropbox::API_PREFIX;
  url += "/files/upload";

  ifstream ifs(fname);
  string post_body(istreambuf_iterator<char>(ifs), {});

  dropbox::HEADERS headers;

  string dropbox_api_arg(dropbox::DROPBOX_API_ARG);
  dropbox_api_arg += "{\"path\": \"/";
  if (dir[0]) {
    dropbox_api_arg += dir;
    dropbox_api_arg += "/";
  }
  dropbox_api_arg += basename((char *)fname);
  dropbox_api_arg += "\", \"mode\": \"overwrite\", \"autorename\": false, \"mute\": true}";
  headers.push_back(dropbox_api_arg);

  string content_type(dropbox::CONTENT_TYPE);
  content_type += dropbox::OCTET_STREAM;
  headers.push_back(content_type);

  return dropbox::perform(url, &headers, post_body, cout, dropbox::callback);
}

int dropbox::delete_v2(const char *path) {
  string url(dropbox::RPC_URL);
  url += dropbox::API_PREFIX;
  url += "/files/delete_v2";

  string post_body("{\"path\": \"/");
  post_body += path;
  post_body += "\"}";

  dropbox::HEADERS headers;

  string content_type(dropbox::CONTENT_TYPE);
  content_type += dropbox::JSON;
  headers.push_back(content_type);

  return dropbox::perform(url, &headers, post_body, cout, dropbox::callback);
}

void dropbox::usage(const char *cmd) {
  cout << "Usage: " << endl;
  cout << "  " << cmd << " [d|download|-d|--download] <path>             download a file <path>." << endl;
  cout << "  " << cmd << " [u|upload|-u|--upload] <path1> <path2>        upload a file <path1> to the folder <path2>." << endl;
  cout << "  " << cmd << " [u|upload|-u|--upload] <path1>                download a file <path> to the root folder." << endl;
  cout << "  " << cmd << " [r|remove|-r|--remove] <path>                 remove a file <path>." << endl;
  cout << "  " << cmd << " [h|help|-h|--help]                            print this help." << endl;
  cout << endl;
  cout << "Note: <path> does *not* include the root \"/\". For example," << endl;
  cout << "        " << cmd << " upload foo.pdf Bar" << endl;
  cout << "      tries to upload foo.pdf to /Bar." << endl;
  cout << endl;
}
