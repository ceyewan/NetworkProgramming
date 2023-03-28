#include "epoller.h"

Epoller::Epoller(int maxEvent) : epollfd_(epoll_create(8)), events_(maxEvent) {
  assert(epollfd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() { close(epollfd_); }

bool Epoller::AddFd(int fd, u_int32_t events) {
  if (fd < 0)
    return false;
  epoll_event ev = {0};
  ev.events = events;
  ev.data.fd = fd;
  bool ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
  return ret;
}

bool Epoller::ModFd(int fd, uint32_t events) {
  if (fd < 0)
    return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
  if (fd < 0)
    return false;
  epoll_event ev = {0};
  return 0 == epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs) {
  return epoll_wait(epollfd_, &events_[0], static_cast<int>(events_.size()),
                    timeoutMs);
}

int Epoller::GetEventFd(size_t i) const {
  assert(i < events_.size() && i >= 0);
  return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) const {
  assert(i < events_.size() && i >= 0);
  return events_[i].events;
}