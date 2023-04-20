#!/usr/bin/env python3

import os, sys

len = os.getenv('CONTENT_LENGTH')
query = sys.stdin.read(len)

print("Content-type: text/html\r\n\r\n")
print("<html><head><title>CGI Test</title></head><body>")
print("<h1>CGI Test</h1>")
print("<p>Query: %s</p>" % query)
print("</body></html>")
# Path: src/cgi.py
