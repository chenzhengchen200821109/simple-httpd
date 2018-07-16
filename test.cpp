#include <iostream>
#include "httpd.hpp"

int main()
{
    MesgBody m("hello world\r\n");
    Headers h;
    StartLine sl;

    std::cout << sl.getStartLine(501) << std::endl;

    m.appendMesg("a long line\r\n");
    std::cout << "size is " << m.size() << std::endl;
    std::cout << m.getMesg() << std::endl;

    h.setContentType("text/html");
    std::cout << h.getContentType() << std::endl;

    size_t size = m.size();
    h.setContentLength(size);
    std::cout << h.getContentLength() << std::endl;

    return 0;
}
