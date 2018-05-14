# -*- coding: utf-8 -*-
'''
  Filename: server.py
  Creator: Hemajun
  Description: Python socket server.
'''
import argparse
import socket

if __name__ == '__main__':
    parse = argparse.ArgumentParser()
    parse.add_argument('--server_port', default=12345, help='The port of server.')
    parse.add_argument('--backlog', default=8, help='The max number of connections.')
    parse.add_argument('--max_buffer_size', default=1024, help='The max size of buffer.')
    args = parse.parse_args()
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockfd.bind((str(socket.INADDR_ANY), args.server_port))
    sockfd.listen(args.backlog)
    connfd, addr_client = sockfd.accept()
    data = connfd.recv(args.max_buffer_size)
    print 'Got data from', addr_client, ':', data
    connfd.close()
    sockfd.close()