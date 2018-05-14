# -*- coding: utf-8 -*-
'''
  Filename: client.py
  Creator: Hemajun
  Description: Python socket client
'''
import argparse
import socket

if __name__ == '__main__':
    parse = argparse.ArgumentParser()
    parse.add_argument('--server_ip', default='127.0.0.1', help='The ip of server.')
    parse.add_argument('--server_port', default=12345, help='The port of server.')
    args = parse.parse_args()
    sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockfd.connect((args.server_ip, args.server_port))
    data = raw_input('Input your data: ')
    sockfd.send(data)
    sockfd.close()