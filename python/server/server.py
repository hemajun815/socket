# -*- coding: utf-8 -*-
'''
  Filename: server.py
  Creator: Hemajun
  Description: Python socket server.
'''
import socket

PORT = 12345
MAX_DATA_SIZE = 1024

if __name__ == '__main__':
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockfd.bind((str(socket.INADDR_ANY), PORT))
    sockfd.listen(8)
    while True:
        connfd, addr_client = sockfd.accept()
        print connfd, addr_client
        connfd.close()