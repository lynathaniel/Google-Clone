/*
 * Copyright ©2023 Chris Thachuk.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

using std::cerr;
using std::endl;
using std::string;

namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int* const listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // STEP 1:
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_flags |= AI_V4MAPPED;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  const char* port_string = std::to_string(port_).c_str();
  struct addrinfo* result;
  int res = getaddrinfo(nullptr, port_string, &hints, &result);

  if (res != 0) {
    cerr << "getaddrinfo() failed: ";
    cerr << gai_strerror(res) << endl;
    return false;
  }

  for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
    *listen_fd = socket(ai_family,
                        rp->ai_socktype,
                        rp->ai_protocol);
    if (*listen_fd == -1) {
      // Failed to create socket, repeat
      cerr << "socket() failed: " << strerror(errno) << endl;
      *listen_fd = -1;
      continue;
    }

    int optval = 1;
    setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));

    // Try binding the socket to returned address and port number
    if (bind(*listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      sock_family_ = rp->ai_family;
      break;
    }

    // Binding failed, repeat
    close(*listen_fd);
    *listen_fd = -1;
  }

  freeaddrinfo(result);

  // Return false if failed to bind
  if (*listen_fd <= 0)
    return false;

  // Set socket to a listening socket
  if (listen(*listen_fd, SOMAXCONN) != 0) {
    cerr << "Failed to mark socket as listening: ";
    cerr << strerror(errno) << endl;
    close(*listen_fd);
    return false;
  }
  listen_sock_fd_ = *listen_fd;
  return true;
}

bool ServerSocket::Accept(int* const accepted_fd,
                          std::string* const client_addr,
                          uint16_t* const client_port,
                          std::string* const client_dns_name,
                          std::string* const server_addr,
                          std::string* const server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:

  // Wait for a client to arrive, then set up socket.
  int client_fd;
  struct sockaddr_storage caddr;
  struct sockaddr* saddr = reinterpret_cast<struct sockaddr*>(&caddr);
  socklen_t caddr_len = sizeof(caddr);
  while (1) {
    client_fd = accept(listen_sock_fd_,
                        saddr,
                        &caddr_len);
    if (client_fd < 0) {
      if ((errno == EAGAIN) || (errno == EINTR)) {
        continue;
      }
      cerr << "Failure on accept: " << strerror(errno) << endl;
      return false;
    }
    break;
  }

  // Set file descriptor output param for client connection.
  *accepted_fd = client_fd;

  // If the client is an IPv4 address:
  if (saddr->sa_family == AF_INET) {
    struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(saddr);
    char astring[INET_ADDRSTRLEN];

    // Convert address + port to appropriate formats, store in output params.
    inet_ntop(AF_INET, &(sa->sin_addr), astring, INET_ADDRSTRLEN);
    *client_addr = astring;
    *client_port = htons(sa->sin_port);
  
  // If the client is an IPv6 address:
  } else if (saddr->sa_family == AF_INET6) {
    struct sockaddr_in6* sa = reinterpret_cast<struct sockaddr_in6*>(saddr);
    char astring[INET6_ADDRSTRLEN];

    // Convert address + port to appropriate formats, store in output params.
    inet_ntop(AF_INET6, &(sa->sin6_addr), astring, INET6_ADDRSTRLEN);
    *client_addr = astring;
    *client_port = htons(sa->sin6_port);

  // If the client is unspecified:
  } else {
    cerr << "Could not determine address and port." << endl;
    return false;
  }

  // Perform reverse DNS lookup on client, store in output param.
  char client_dns_buf[NI_MAXHOST];
  if (getnameinfo(saddr, caddr_len, client_dns_buf, 
        sizeof(client_dns_buf), nullptr, 0, 0) != 0) {
    cerr << "Failed to get client DNS name." << endl;
    return false;
  }
  *client_dns_name = client_dns_buf;

  // Obtain the server address and perform reverse DNS lookup.
  char server_dns_buf[NI_MAXHOST];
  server_dns_buf[0] = '\0';

  // If the server is an IPv4 address:
  if (sock_family_ == AF_INET) {
    struct sockaddr_in srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET_ADDRSTRLEN];
    getsockname(client_fd,
                reinterpret_cast<struct sockaddr*>(&srvr),
                &srvrlen);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo(reinterpret_cast<struct sockaddr*>(&srvr),
                srvrlen, server_dns_buf, 1024, nullptr, 0, 0);
    *server_addr = addrbuf;
    *server_dns_name = server_dns_buf;

  // If the server is an IPv6 address:
  } else {
    struct sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET6_ADDRSTRLEN];
    getsockname(client_fd,
                reinterpret_cast<struct sockaddr*>(&srvr),
                &srvrlen);
    inet_ntop(AF_INET, &srvr.sin6_addr, addrbuf, INET_ADDRSTRLEN);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo(reinterpret_cast<struct sockaddr*>(&srvr),
                srvrlen, server_dns_buf, 1024, nullptr, 0, 0);
    *server_addr = addrbuf;
    *server_dns_name = server_dns_buf;
  }

  return true;
}

}  // namespace hw4
