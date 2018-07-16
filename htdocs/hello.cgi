#!/usr/bin/python

s1 = "<html>"
s2 = "<head>"
s3 = "<title>Hello World - My first CGI program</title>"
s4 = "</head>"
s5 = "<body>"
s6 = "<h2>Hello World! I am the first CGI program from httpd server</h2>"
s7 = "</body>"
s8 = "</html>"

n = len(s1) + len(s2) + len(s3) + len(s4) + len(s5) + len(s6) + len(s7) + len(s8)

print "Content-type:text/html"
print "Content-length:%d" % n
print
print "<html>"
print "<head>"
print "<title>Hello World - My first CGI program</title>"
print "</head>"
print "<body>"
print "<h2>Hellow World! I am the first CGI program from httpd server</h2>"
print "</body>"
print "</html>"
