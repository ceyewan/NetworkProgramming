#include "sqlconnRAII.h"

SqlConnPool *SqlConnPool::Instance() {
  static SqlConnPool connPool;
  return &connPool;
}

void SqlConnPool::Init(const char *host, int port, const char *user,
                       const char *pwd, const char *dbName, int connSize = 10) {
  assert(connSize > 0);
  for (int i = 0; i < connSize; i++) {
    MYSQL *sql = nullptr;
    sql = mysql_init(sql);
    assert(sql);
    sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
    assert(sql);
    connn_queue_.push(sql);
  }
  MAX_CONN_ = connSize;
  sem_init(&sem_, 0, MAX_CONN_);
}

MYSQL *SqlConnPool::GetConn() {
  MYSQL *sql = nullptr;
  if (connn_queue_.empty())
    return nullptr;
  sem_wait(&sem_);
  std::lock_guard<std::mutex> locker(mutex_);
  sql = connn_queue_.front();
  connn_queue_.pop();
  return sql;
}

void SqlConnPool::FreeConn(MYSQL *sql) {
  assert(sql);
  std::lock_guard<std::mutex> locker(mutex_);
  connn_queue_.push(sql);
  sem_post(&sem_);
}

void SqlConnPool::ClosePool() {
  std::lock_guard<std::mutex> locker(mutex_);
  while (!connn_queue_.empty()) {
    auto it = connn_queue_.front();
    connn_queue_.pop();
    mysql_close(it);
  }
  mysql_library_end();
}

int SqlConnPool::GetFreeConnCount() {
  std::lock_guard<std::mutex> locker(mutex_);
  return connn_queue_.size();
}