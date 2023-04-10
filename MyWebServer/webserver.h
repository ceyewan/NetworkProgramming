#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <fcntl.h> // fcntl()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close()
#include <unordered_map>

#include "epoller.h"
#include "httpconn.h"
#include "threadpool.h"
#include "timer.h"

class WebServer {
public:
  WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort,
            const char *sqlUser, const char *sqlPwd, const char *dbName,
            int connPoolNum, int threadNum);

  ~WebServer();
  void Start();

private:
  bool InitSocket();
  void InitEventMode(int trigMode);
  void AddClient(int fd, sockaddr_in addr);

  void DealListen();
  void DealWrite(HTTPConn *client);
  void DealRead(HTTPConn *client);

  static void SendError(int fd, const char *info);
  void ExtentTime(HTTPConn *client);
  void CloseConn(HTTPConn *client);

  void OnRead(HTTPConn *client);
  void OnWrite(HTTPConn *client);
  void OnProcess(HTTPConn *client);

  static const int MAX_FD = 65536;

  static int SetFdNonblock(int fd);

  int port_;
  bool openLinger_;
  int timeoutMS_; /* 毫秒MS */
  bool isClose_;
  int listenFd_{};
  char *srcDir_;

  uint32_t listenEvent_{};
  uint32_t connEvent_{};

  std::unique_ptr<TimerManager> timer_;
  std::unique_ptr<ThreadPool> threadpool_;
  std::unique_ptr<Epoller> epoller_;
  std::unordered_map<int, HTTPConn> users_;
};