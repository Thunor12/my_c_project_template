// Optional teapot example — requires: ./scripts/enable-teapot.sh
// Build via: ./nob examples  (only when teapot/ is present)
#include <stdio.h>

#include "stb_teapot.h"

static teapot_response hello_handler(const teapot_request *req)
{
    (void)req;
    teapot_response resp;
    teapot_response_init(&resp, 200);
    tp_sb_appendf(&resp.body, "Hello from teapot (optional submodule)\n");
    tp_sb_append_null(&resp.body);
    return resp;
}

int main(void)
{
    teapot_route routes[] = {
        {TEAPOT_GET, "/hello", hello_handler},
    };

    teapot_server server = {
        .port = 8080,
        .routes = routes,
        .route_count = sizeof(routes) / sizeof(routes[0]),
    };

    printf("Listening on http://127.0.0.1:8080/hello\n");
    return teapot_listen(&server);
}
