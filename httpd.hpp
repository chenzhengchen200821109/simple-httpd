#ifndef _HTTPD_HPP
#define _HTTPD_HPP

#include "httpd.h"
#include <string>
#include <map>

class StartLine
{
    public:
        StartLine() 
        { 
            status_code_.insert(std::pair<int, std::string>(500, "HTTP/1.0 501 Internal Server Error\r\n"));
            status_code_.insert(std::pair<int, std::string>(400, "HTTP/1.0 400 BAD REQUEST\r\n"));
            status_code_.insert(std::pair<int, std::string>(200, "HTTP/1.0 200 OK\r\n"));
            status_code_.insert(std::pair<int, std::string>(404, "HTTP/1.0 404 NOT FOUND\r\n"));
            status_code_.insert(std::pair<int, std::string>(501, "HTTP/1.0 501 Method Not Implemented\r\n"));
        };
        const std::string getStartLine(int code) const;
    private:
        std::map<int, std::string> status_code_;
};

const std::string StartLine::getStartLine(int code) const
{
    std::string s(status_code_.at(code));
    return s;
}

class MesgBody
{
    public:
        explicit MesgBody(const std::string& mesg) : mesg_(mesg) 
        {
            mesg_.append("\r\n");
        }
        size_t size() const { return mesg_.size() - 2; }
        void appendMesg(const std::string& mesg) { mesg_.append(mesg); }
        const std::string getMesg() 
        { 
            mesg_.append("\r\n");
            return mesg_; 
        }
    private:
        std::string mesg_;
};

typedef enum { content_type = 0, content_length = 1, server_name = 2 } header_key;

class Headers
{
    public:
        Headers() {};
        void setContentType(const std::string& type);
        const std::string getContentType() const;
        void setContentLength(size_t length);
        const std::string getContentLength() const;
        void setServer(const std::string& server);
        const std::string getServer() const;
    private:
        std::map<int, std::string> headers_;
};

void Headers::setContentType(const std::string& type)
{
    std::string value("Content-Type: ");
    value.append(type);
    value.append("\r\n");
    headers_.insert(std::pair<int, std::string>(content_type, value)); 
}

const std::string Headers::getContentType() const
{
    std::string s(headers_.at(content_type));
    return s;
}

void Headers::setContentLength(size_t length)
{
    std::string value("Content-Length: ");
    value.append(std::to_string(length));
    value.append("\r\n");
    headers_.insert(std::pair<int, std::string>(content_length, value));
}

const std::string Headers::getContentLength() const
{
    std::string s(headers_.at(content_length));
    return s;
}

void Headers::setServer(const std::string& server)
{
    std::string value("Server: ");
    value.append(server);
    value.append("\r\n");
    headers_.insert(std::pair<int, std::string>(server_name, value));
}

const std::string Headers::getServer() const
{
    std::string s(headers_.at(server_name));
    return s;
}

#endif
