# -*- coding: utf-8 -*-
'''
  Filename: client.py
  Creator: Hemajun
  Description: Python socket client
'''
import socket

PORT = 12345
MAX_DATA_SIZE = 1024

if __name__ == '__main__':
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockfd.connect(('127.0.0.1', PORT))