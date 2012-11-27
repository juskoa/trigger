#!/usr/bin/python
from ctpbusy import application as hey
from fileserver import FileServer

def factory(static_path):
    return FileServer(hey, static_path)

def test_server():
    from wsgiref.simple_server import make_server
    application = factory("static-files")
    server = make_server('', 8080, application)
    server.serve_forever()
    
if __name__ == '__main__':
    test_server()
    
