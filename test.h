#ifndef _TEST_H
#define _TEST_H

#include <string>
#include <map>

class MesgBody
{
    public:
        explicit MesgBody(const std::string& mesg) : mesg_(mesg) {}
        size_t size() const { return mesg_.size(); }
        void appendMesg(const std::string& mesg) { mesg_.append(mesg); }
        const std::string& getMesg() const { return mesg_; }
    private:
        std::string mesg_;
};

typedef enum { content_type = 0, content_length = 1 } header_key;

class Headers
{
    public:
        Headers() {};
        void setContentType(const std::string& type);
        const std::string getContentType() const;
        void setContentLength(size_t length);
        const std::string getContentLength() const;
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

#endif
