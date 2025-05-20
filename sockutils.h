#pragma once
#include <netdb.h>

struct addrinfo* get_addresses(const char* ip, const char* port, int s);
int try_connect(struct addrinfo* servinfo, struct addrinfo** out);
int try_bind(struct addrinfo* servinfo);
