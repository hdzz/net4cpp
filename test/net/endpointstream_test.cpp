#include <sstream>
#include <gtest/gtest.h>
#include "net/endpointstream.hpp"
#include "net/sender.hpp"
#include "net/receiver.hpp"
#include "net/connector.hpp"

using namespace net;
using namespace std;

TEST(NetEndpointStreamTest,Construct1)
{
    auto is = join("228.0.0.4","54321");
}

TEST(NetEndpointStreamTest,Construct2)
{
    auto os = distribute("228.0.0.4","54321");
}

TEST(NetEndpointStreamTest,HttpRequestAndResponse)
{
    auto s = connect("www.google.fi","http");

    s << "GET / HTTP/1.1"                << crlf
      << "Host: www.google.com"          << crlf
      << "Connection: close"             << crlf
      << "Accept: text/plain, text/html" << crlf
      << "Accept-Charset: utf-8"         << crlf
      << crlf
      << flush;

    auto os = ostringstream{};
    while(s)
    {
        char c;
        s >> noskipws >> c;
        os << c;
    }
    SUCCEED() << os.str();
    cout << os.str();;
}
