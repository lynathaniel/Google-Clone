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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

#define BUFSIZE 1024

namespace hw4 {

static const char* kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest* const request) {
  // Use WrappedRead from HttpUtils.cc to read bytes from the files into
  // private buffer_ variable. Keep reading until:
  // 1. The connection drops
  // 2. You see a "\r\n\r\n" indicating the end of the request header.
  //
  // Hint: Try and read in a large amount of bytes each time you call
  // WrappedRead.
  //
  // After reading complete request header, use ParseRequest() to parse into
  // an HttpRequest and save to the output parameter request.
  //
  // Important note: Clients may send back-to-back requests on the same socket.
  // This means WrappedRead may also end up reading more than one request.
  // Make sure to save anything you read after "\r\n\r\n" in buffer_ for the
  // next time the caller invokes GetNextRequest()!

  // STEP 1:
  unsigned char readbuf[BUFSIZE];
  size_t found;

  // Keep reading until connection drops or end of request is seen.
  while ((found = buffer_.find(kHeaderEnd)) == string::npos) {
    int res = WrappedRead(fd_, readbuf, BUFSIZE);

    // There was an error reading or we didn't find end of request header
    // before the connection dropped.
    if (res <= 0) {
      return false;
    }
    buffer_ += string(reinterpret_cast<char*>(readbuf), res);
  }

  // Get first request read, parse it, and store it in output param.
  *request = ParseRequest(buffer_.substr(0, found + kHeaderEndLen));

  // If multiple requests were read, store everything after the first
  // into buffer_ for the next GetNextRequest() call.
  buffer_ = buffer_.substr(found + kHeaderEndLen);
  
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse& response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string& request) const {
  HttpRequest req("/");  // by default, get "/".

  // Plan for STEP 2:
  // 1. Split the request into different lines (split on "\r\n").
  // 2. Extract the URI from the first line and store it in req.URI.
  // 3. For the rest of the lines in the request, track the header name and
  //    value and store them in req.headers_ (e.g. HttpRequest::AddHeader).
  //
  // Hint: Take a look at HttpRequest.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for:
  // - Splitting a string into lines on a "\r\n" delimiter
  // - Trimming whitespace from the end of a string
  // - Converting a string to lowercase.
  //
  // Note: If a header is malformed, skip that line.

  // STEP 2:

  // Split the request into lines.
  vector<string> lines;
  boost::split(lines, request, boost::is_any_of("\r\n"),
               boost::token_compress_on);

  // Set the URI of the request.
  vector<string> components;
  string first_line = lines[0];
  boost::split(components, first_line, boost::is_any_of(" "),
               boost::token_compress_on);
  req.set_uri(components[1]);

  // Add each header to the request.
  for (unsigned int i = 1; i < lines.size(); i++) {
    vector<string> header;
    boost::split(header, lines[i], boost::is_any_of(":"),
                 boost::token_compress_on);

    // Only add the header if it isn't malformed.
    if (header.size() == 2) {
      boost::algorithm::to_lower(header[0]);
      boost::trim(header[1]);
      req.AddHeader(header[0], header[1]);
    }
  }

  return req;
}

}  // namespace hw4
