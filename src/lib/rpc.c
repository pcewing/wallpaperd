#include "rpc.h"
#include "ipc.h"
#include "log.h"

#include "rpc/rpc_builder.h"
#include "rpc/rpc_reader.h"
#include "rpc/rpc_verifier.h"

struct rpc_server_t {
    struct ipc_server_t *ipc_server;
    rpc_handler rpc_handlers[2];
};

// clang-format off

/* Type Conversion Tables */

I3BG_Rpc_RequestType_enum_t request_types[] = {
    [RPC_TYPE_PAUSE] = I3BG_Rpc_RequestType_Pause,
    [RPC_TYPE_DISCOVER] = I3BG_Rpc_RequestType_Discover
};

I3BG_Rpc_ResponseType_enum_t response_types[] = {
    [I3BG_Rpc_RequestType_Pause] = I3BG_Rpc_ResponseType_Pause,
    [I3BG_Rpc_RequestType_Discover] = I3BG_Rpc_ResponseType_Discover
};

// clang-format on

/* Domain Model Serializers */

uint8_t *pause_request_serializer(void *msg, size_t *size) {
    assert(msg);
    struct pause_request_t *pr_model = (struct pause_request_t *)msg;

    flatcc_builder_t builder, *B;
    B = &builder;

    flatcc_builder_init(B);

    // Create PauseRequest
    I3BG_Rpc_PauseRequest_ref_t pr_dto =
        I3BG_Rpc_PauseRequest_create(B, pr_model->duration_ms);

    // Create RequestContent union
    I3BG_Rpc_RequestContent_union_ref_t request_content =
        I3BG_Rpc_RequestContent_as_PauseRequest(pr_dto);

    // Create Request
    I3BG_Rpc_Request_ref_t r_dto =
        I3BG_Rpc_Request_create(B, I3BG_Rpc_RequestType_Pause, request_content);

    // Create MessageContent union
    I3BG_Rpc_MessageContent_union_ref_t message_content =
        I3BG_Rpc_MessageContent_as_Request(r_dto);

    // Create Message
    I3BG_Rpc_Message_create_as_root(B, I3BG_Rpc_MessageType_Request,
                                    message_content);

    uint8_t *buf = flatcc_builder_finalize_buffer(B, size);

    flatcc_builder_clear(B);

    return buf;
}

uint8_t *pause_response_serializer(void *msg, size_t *size) {
    assert(msg);
    struct pause_response_t *pr_model = (struct pause_response_t *)msg;

    flatcc_builder_t builder, *B;
    B = &builder;

    flatcc_builder_init(B);

    // Create PauseResponse
    I3BG_Rpc_PauseResponse_ref_t pr_dto =
        I3BG_Rpc_PauseResponse_create(B, pr_model->success);

    // Create ResponseContent union
    I3BG_Rpc_ResponseContent_union_ref_t response_content =
        I3BG_Rpc_ResponseContent_as_PauseResponse(pr_dto);

    // Create Response
    I3BG_Rpc_Response_ref_t r_dto = I3BG_Rpc_Response_create(
        B, I3BG_Rpc_ResponseType_Pause, response_content);

    // Create MessageContent union
    I3BG_Rpc_MessageContent_union_ref_t message_content =
        I3BG_Rpc_MessageContent_as_Response(r_dto);

    // Create Message
    I3BG_Rpc_Message_create_as_root(B, I3BG_Rpc_MessageType_Response,
                                    message_content);

    uint8_t *buf = flatcc_builder_finalize_buffer(B, size);

    flatcc_builder_clear(B);

    return buf;
}

uint8_t *discover_request_serializer(void *msg, size_t *size) {
    assert(msg);
    struct discover_request_t *dr_model = (struct discover_request_t *)msg;

    flatcc_builder_t builder, *B;
    B = &builder;

    flatcc_builder_init(B);

    // Create DiscoverRequest
    I3BG_Rpc_DiscoverRequest_ref_t dr_dto = I3BG_Rpc_DiscoverRequest_create(B);

    // Create RequestContent union
    I3BG_Rpc_RequestContent_union_ref_t request_content =
        I3BG_Rpc_RequestContent_as_DiscoverRequest(dr_dto);

    // Create Request
    I3BG_Rpc_Request_ref_t r_dto = I3BG_Rpc_Request_create(
        B, I3BG_Rpc_RequestType_Discover, request_content);

    // Create MessageContent union
    I3BG_Rpc_MessageContent_union_ref_t message_content =
        I3BG_Rpc_MessageContent_as_Request(r_dto);

    // Create Message
    I3BG_Rpc_Message_create_as_root(B, I3BG_Rpc_MessageType_Request,
                                    message_content);

    uint8_t *buf = flatcc_builder_finalize_buffer(B, size);

    flatcc_builder_clear(B);

    return buf;
}

uint8_t *discover_response_serializer(void *msg, size_t *size) {
    assert(msg);
    struct discover_response_t *dr_model = (struct discover_response_t *)msg;

    flatcc_builder_t builder, *B;
    B = &builder;

    flatcc_builder_init(B);

    // Create DiscoverResponse
    I3BG_Rpc_DiscoverResponse_ref_t dr_dto =
        I3BG_Rpc_DiscoverResponse_create(B, dr_model->pid);

    // Create ResponseContent union
    I3BG_Rpc_ResponseContent_union_ref_t response_content =
        I3BG_Rpc_ResponseContent_as_DiscoverResponse(dr_dto);

    // Create Response
    I3BG_Rpc_Response_ref_t r_dto = I3BG_Rpc_Response_create(
        B, I3BG_Rpc_ResponseType_Discover, response_content);

    // Create MessageContent union
    I3BG_Rpc_MessageContent_union_ref_t message_content =
        I3BG_Rpc_MessageContent_as_Response(r_dto);

    // Create Message
    I3BG_Rpc_Message_create_as_root(B, I3BG_Rpc_MessageType_Response,
                                    message_content);

    uint8_t *buf = flatcc_builder_finalize_buffer(B, size);

    flatcc_builder_clear(B);

    return buf;
}

typedef uint8_t *(*serializer)(void *msg, size_t *size);

// clang-format off

serializer request_serializers[] = {
    [I3BG_Rpc_RequestType_Pause] = pause_request_serializer,
    [I3BG_Rpc_RequestType_Discover] = discover_request_serializer
};

serializer response_serializers[] = {
    [I3BG_Rpc_ResponseType_Pause] = pause_response_serializer,
    [I3BG_Rpc_ResponseType_Discover] = discover_response_serializer
};

// clang-format on

/* DTO to Domain Model Conversion */

void pause_request_from_dto(const void *dto, void **model) {
    I3BG_Rpc_PauseRequest_table_t pr_dto = (I3BG_Rpc_PauseRequest_table_t)dto;
    struct pause_request_t *pr = malloc(sizeof(struct pause_request_t));

    pr->duration_ms = I3BG_Rpc_PauseRequest_duration_ms(pr_dto);

    *model = pr;
}

void pause_response_from_dto(const void *dto, void **model) {
    I3BG_Rpc_PauseResponse_table_t pr_dto = (I3BG_Rpc_PauseResponse_table_t)dto;
    struct pause_response_t *pr = malloc(sizeof(struct pause_response_t));

    pr->success = I3BG_Rpc_PauseResponse_success(pr_dto);

    *model = pr;
}

void discover_request_from_dto(const void *dto, void **model) {
    I3BG_Rpc_DiscoverRequest_table_t dr_dto =
        (I3BG_Rpc_DiscoverRequest_table_t)dto;
    struct discover_request_t *dr = malloc(sizeof(struct discover_request_t));

    *model = dr;
}

void discover_response_from_dto(const void *dto, void **model) {
    I3BG_Rpc_DiscoverResponse_table_t dr_dto =
        (I3BG_Rpc_DiscoverResponse_table_t)dto;
    struct discover_response_t *dr = malloc(sizeof(struct discover_response_t));

    dr->pid = I3BG_Rpc_DiscoverResponse_pid(dr_dto);

    *model = dr;
}

typedef void (*dto_converter)(const void *dto, void **model);

// clang-format off

dto_converter request_dto_converters[] = {
    [I3BG_Rpc_RequestType_Pause] = pause_request_from_dto,
    [I3BG_Rpc_RequestType_Discover] = discover_request_from_dto
};

dto_converter response_dto_converters[] = {
    [I3BG_Rpc_ResponseType_Pause] = pause_response_from_dto,
    [I3BG_Rpc_ResponseType_Discover] = discover_response_from_dto
};

// clang-format on

/* Domain model allocators */

#define ALLOCATOR(name)                                                        \
    void name##_allocator(void **model) {                                      \
        *model = malloc(sizeof(struct name##_t));                              \
        memset(*model, 0, sizeof(struct name##_t));                            \
    }

ALLOCATOR(pause_request)
ALLOCATOR(pause_response)
ALLOCATOR(discover_request)
ALLOCATOR(discover_response)

typedef void (*allocator)(void **model);

// clang-format off

allocator request_allocators[] = {
    [I3BG_Rpc_RequestType_Pause] = pause_request_allocator,
    [I3BG_Rpc_RequestType_Discover] = discover_request_allocator
};

allocator response_allocators[] = {
    [I3BG_Rpc_ResponseType_Pause] = pause_response_allocator,
    [I3BG_Rpc_ResponseType_Discover] = discover_response_allocator
};

// clang-format on

wpd_error_t handle_rpc(struct rpc_server_t *server,
                       I3BG_Rpc_Message_table_t rpc) {
    wpd_error_t error;

    // Extract type from the RPC message and ensure it's a request
    I3BG_Rpc_MessageType_enum_t message_type = I3BG_Rpc_Message_type(rpc);
    if (message_type != I3BG_Rpc_MessageType_Request) {
        error = WPD_ERROR_RPC_UNEXPECTED_MESSAGE_TYPE;
        goto done;
    }

    // Extract the request content
    I3BG_Rpc_Request_table_t request_dto = I3BG_Rpc_Message_content(rpc);
    I3BG_Rpc_RequestType_enum_t request_type =
        I3BG_Rpc_Request_type(request_dto);
    if (!I3BG_Rpc_RequestType_is_known_value(request_type)) {
        error = WPD_ERROR_RPC_REQUEST_TYPE_UNKNOWN;
        goto done;
    }
    const void *request_content = I3BG_Rpc_Request_content(request_dto);

    // Convert the request DTO to the domain model
    void *request = NULL;
    request_dto_converters[request_type](request_content, &request);

    // Allocate the response object
    I3BG_Rpc_ResponseType_enum_t response_type = response_types[request_type];
    void *response = NULL;
    if (!response_allocators[response_type]) {
        error = WPD_ERROR_RPC_MISSING_REPONSE_ALLOCATOR;
        goto free_request;
    }
    response_allocators[response_type](&response);

    // Execute the handler
    if (server->rpc_handlers[request_type]) {
        error = server->rpc_handlers[request_type](request, response);
        if (error != WPD_ERROR_GLOBAL_SUCCESS)
            goto free_response;

        // Serialize the response object
        size_t size;
        uint8_t *buf = response_serializers[response_type](response, &size);

        // Send the response
        error = IpcServer.send(server->ipc_server, size, buf);

        // free_buf:
        free(buf);
    }

free_response:
    free(response);
free_request:
    free(request);
done:
    return error;
}

/* RpcServer */

wpd_error_t rpc_server_create(struct rpc_server_t **server) {
    assert(server);
    assert(!(*server));

    *server = malloc(sizeof(struct rpc_server_t));
    memset(*server, 0, sizeof(struct rpc_server_t));

    IpcServer.create(&(*server)->ipc_server);

    memset((*server)->rpc_handlers, 0, sizeof((*server)->rpc_handlers));

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t rpc_server_destroy(struct rpc_server_t **server) {
    assert(server);
    assert(*server);

    IpcServer.destroy(&(*server)->ipc_server);

    free(*server);
    *server = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t rpc_server_register_handler(struct rpc_server_t *server,
                                        rpc_type_t type, rpc_handler handler) {
    assert(server);

    I3BG_Rpc_RequestType_enum_t t = request_types[type];
    server->rpc_handlers[t] = handler;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t rpc_server_start(struct rpc_server_t *server) {
    assert(server);

    wpd_error_t error = IpcServer.start(server->ipc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t rpc_server_poll(struct rpc_server_t *server) {
    assert(server);

    // Initialize the message object
    struct ipc_msg_t *msg = NULL;
    IpcMessage.create(&msg);

    // Poll for messages over the IPC channel
    bool accepted;
    wpd_error_t error = IpcServer.poll(server->ipc_server, &accepted, msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    // Handle the message
    if (accepted) {

        int ret = I3BG_Rpc_Message_verify_as_root(IpcMessage.data(msg),
                                                  IpcMessage.size(msg));
        if (ret) {
            const char *s = flatcc_verify_error_string(ret);
            LOGWARN("Failed to deserialize rpc message: %s\n", s);
            error = WPD_ERROR_RPC_INVALID_MESSAGE;
        } else {
            // Deserialize into the RPC DTO and handle the RPC
            I3BG_Rpc_Message_table_t rpc =
                I3BG_Rpc_Message_as_root(IpcMessage.data(msg));
            error = handle_rpc(server, rpc);
        }

        IpcServer.close(server->ipc_server);
    }

    IpcMessage.destroy(&msg);

    return error;
}

wpd_error_t rpc_server_stop(struct rpc_server_t *server) {
    assert(server);

    wpd_error_t error = IpcServer.stop(server->ipc_server);
    if (error != WPD_ERROR_GLOBAL_SUCCESS)
        return error;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t execute_rpc(I3BG_Rpc_RequestType_enum_t request_type, void *req,
                        void *res) {
    wpd_error_t error;
    int sfd;

    // Serialize the request object
    size_t size;
    uint8_t *buf = request_serializers[request_type](req, &size);

    // Create the IPC client
    struct ipc_client_t *ipc_client = NULL;
    error = IpcClient.create(&ipc_client);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        goto free_buf;
    }

    // Connect to the IPC channel
    error = IpcClient.connect(ipc_client);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        goto destroy_ipc_client;
    }

    // Send the request
    error = IpcClient.send(ipc_client, size, buf);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        goto ipc_disconnect;
    }

    // Initialize the message object
    struct ipc_msg_t *msg = NULL;
    IpcMessage.create(&msg);

    // Wait for a response
    error = IpcClient.recv(ipc_client, msg);
    if (error != WPD_ERROR_GLOBAL_SUCCESS) {
        goto destroy_msg;
    }

    // Deserialize into a RPC message
    I3BG_Rpc_Message_table_t rpc =
        I3BG_Rpc_Message_as_root(IpcMessage.data(msg));

    // Extract type and ensure the message is a response
    I3BG_Rpc_MessageType_enum_t message_type = I3BG_Rpc_Message_type(rpc);
    if (message_type != I3BG_Rpc_MessageType_Response) {
        error = WPD_ERROR_RPC_UNEXPECTED_MESSAGE_TYPE;
        goto destroy_msg;
    }

    // Extract the content
    I3BG_Rpc_Response_table_t response_dto = I3BG_Rpc_Message_content(rpc);
    I3BG_Rpc_ResponseType_enum_t response_type =
        I3BG_Rpc_Response_type(response_dto);
    const void *response_content = I3BG_Rpc_Response_content(response_dto);

    // Verify it is the expected response type
    if (response_type != response_types[request_type]) {
        error = WPD_ERROR_RPC_UNEXPECTED_RESPONSE_TYPE;
        goto destroy_msg;
    }

    // Convert the DTO to the response model
    response_dto_converters[response_type](response_content, res);

destroy_msg:
    IpcMessage.destroy(&msg);
ipc_disconnect:
    IpcClient.disconnect(ipc_client);
destroy_ipc_client:
    IpcClient.destroy(&ipc_client);
free_buf:
    free(buf);

    return error;
}

/* RpcClient */

static wpd_error_t rpc_client_pause(struct pause_request_t *req,
                                    struct pause_response_t **res) {
    assert(req);
    assert(*res);

    return execute_rpc(I3BG_Rpc_RequestType_Pause, req, res);
}

static wpd_error_t rpc_client_discover(struct discover_request_t *req,
                                       struct discover_response_t **res) {
    assert(req);
    assert(*res);

    return execute_rpc(I3BG_Rpc_RequestType_Discover, req, res);
}

// clang-format off

_RpcServer const RpcServer = {
    .create = rpc_server_create,
    .destroy = rpc_server_destroy,
    .register_handler = rpc_server_register_handler,
    .start = rpc_server_start,
    .poll = rpc_server_poll,
    .stop = rpc_server_stop
};

_RpcClient const RpcClient = {
    .pause = rpc_client_pause,
    .discover = rpc_client_discover
};

// clang-format on
