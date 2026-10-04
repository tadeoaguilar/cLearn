// Registers the REST endpoints on an httplib::Server.
//
//   GET    /health
//   GET    /tasks            ?status=&priority=&q=&sort=&order=&limit=&offset=
//   POST   /tasks
//   GET    /tasks/{id}
//   PUT    /tasks/{id}
//   PATCH  /tasks/{id}
//   DELETE /tasks/{id}
#pragma once

#include <httplib.h>

#include "app/logger.hpp"
#include "service/task_service.hpp"

namespace tasks::http {

void register_routes(httplib::Server& server, TaskService& service, Logger& logger);

} // namespace tasks::http
