#include "webserver.h"

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
                     int sqlPort, const char *sqlUser, const char *sqlPwd,
                     const char *dbName, int connPoolNum, int threadNum)
    : port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS),
      isClose_(false), timer_(new TimerManager()),
      threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
  srcDir_ = getcwd(nullptr, 256);
  assert(srcDir_);
  strncat(srcDir_, "/resources/", 16);
  HTTPConn::userCount = 0;
  HTTPConn::srcDir = srcDir_;
  SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName,
                                connPoolNum);

  InitEventMode(trigMode);
  if (!InitSocket()) {
    isClose_ = true;
  }
}

WebServer::~WebServer() {
  close(listenFd_);
  isClose_ = true;
  free(srcDir_);
  SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode(int trigMode) {
  listenEvent_ = EPOLLRDHUP;
  connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
  switch (trigMode) {
  case 0:
    break;
  case 1:
    connEvent_ |= EPOLLET;
    break;
  case 2:
    listenEvent_ |= EPOLLET;
    break;
  case 3:
    listenEvent_ |= EPOLLET;
    connEvent_ |= EPOLLET;
    break;
  default:
    listenEvent_ |= EPOLLET;
    connEvent_ |= EPOLLET;
    break;
  }
  HTTPConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::Start() {
  int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
  while (!isClose_) {
    if (timeoutMS_ > 0) {
      timeMS = timer_->GetNextTick();
    }
    int eventCnt = epoller_->Wait(timeMS);
    for (int i = 0; i < eventCnt; i++) {
      int fd = epoller_->GetEventFd(i);
      uint32_t events = epoller_->GetEvents(i);
      if (fd == listenFd_) {
        DealListen();
      } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        assert(users_.count(fd) > 0);
        CloseConn(&users_[fd]);
      } else if (events & EPOLLIN) {
        assert(users_.count(fd) > 0);
        DealRead(&users_[fd]);
      } else if (events & EPOLLOUT) {
        assert(users_.count(fd) > 0);
        DealWrite(&users_[fd]);
      }
    }
  }
}

void WebServer::CloseConn(HTTPConn *client) {
  assert(client);
  epoller_->DelFd(client->GetFd());
  client->Close();
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
  assert(fd > 0);
  users_[fd].Init(fd, addr);
  if (timeoutMS_ > 0) {
    timer_->Add(fd, timeoutMS_,
                std::bind(&WebServer::CloseConn, this, &users_[fd]));
  }
  epoller_->AddFd(fd, EPOLLIN | connEvent_);
  SetFdNonblock(fd);
}

int WebServer::SetFdNonblock(int fd) {
  assert(fd > 0);
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

bool WebServer::InitSocket() {
  int ret;
  struct sockaddr_in addr;
  if (port_ > 65535 || port_ < 1024) {
    return false;
  }
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port_);
  struct linger optLinger = {0};
  if (openLinger_) {
    optLinger.l_onoff = 1;
    optLinger.l_linger = 1;
  }
  listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listenFd_ < 0) {
    return false;
  }
  ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger,
                   sizeof(optLinger));
  if (ret < 0) {
    close(listenFd_);
    return false;
  }

  int optval = 1;
  ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                   sizeof(int));
  if (ret == -1) {
    close(listenFd_);
    return false;
  }

  ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    close(listenFd_);
    return false;
  }

  ret = listen(listenFd_, 6);
  if (ret < 0) {
    close(listenFd_);
    return false;
  }
  ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
  if (ret == 0) {
    close(listenFd_);
    return false;
  }
  SetFdNonblock(listenFd_);
  return true;
}