#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <mysql/mysql.h> //mysql
#include <regex>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

#include "buffer.h"
#include "sqlconnRAII.h"

enum PARSE_STATE {
  REQUEST_LINE,
  HEADERS,
  BODY,
  FINISH,
};

enum HTTP_CODE {
  NO_REQUEST = 0,
  GET_REQUEST,
  BAD_REQUEST,
  NO_RESOURSE,
  FORBIDDENT_REQUEST,
  FILE_REQUEST,
  INTERNAL_ERROR,
  CLOSED_CONNECTION,
};

class HTTPRequest {
public:
  HTTPRequest() { state_ = REQUEST_LINE; }
  ~HTTPRequest() = default;
  void Init();
  bool Parse(Buffer &buffer);
  std::string &Path() { return path_; };
  std::string Method() { return method_; };
  std::string Version() { return version_; };
  std::string GetPost(const std::string &key);
  std::string GetPost(const char *key);
  bool IsKeepAlive() const;

private:
  bool ParseRequestLine(const std::string &line);
  void ParseHeader(const std::string &line);
  void ParseBody(const std::string &line);
  void ParsePath();
  void ParsePost();
  void ParseFromUrlencoded();
  static bool UserVerify(const std::string &name, const std::string &pwd,
                         bool isLogin);
  static int ConverHex(char ch);

  PARSE_STATE state_;                          // 处理的阶段
  std::string method_, path_, version_, body_; // 请求方法、路径、版本和请求体
  std::unordered_map<std::string, std::string> header_; // header kv 对
  std::unordered_map<std::string, std::string> post_;   // post 请求的 kv 对
  static const std::unordered_set<std::string> DEFAULT_HTML;
  static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};

#endif // HTTP_REQUEST_H