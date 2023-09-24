#include <stdlib.h>
#pragma once
#define RESPONSE_HEADER_ATTRIBUTES_MAX (30)

enum Method
{
    GET,
    POST
};

enum StatusCode
{
    OK             = 200,
    NOT_FOUND      = 404,
    INTERNAL_ERROR = 500
};

struct Request
{
    char* header;
    char* body;
    enum Method method;
    char* path;
};

struct Response
{
    enum StatusCode status_code;
    char* header_attribute[RESPONSE_HEADER_ATTRIBUTES_MAX];
    char* header;
    char* body;
};

struct Request request_new(char* request, size_t request_len);
void request_cleanup(struct Request* request);

struct Response response_new();
void response_cleanup(struct Response* response);
void response_body_from_file(struct Response* response, char* filepath);
char* response_to_str(struct Response* response);
