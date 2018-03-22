# -*- coding: utf-8 -*-
'''
  Filename: client.py
  Creator: Hemajun
  Description: --
'''
from __future__ import print_function
import socket
import threading
import sys

class Client(object):
    """Client class of lan chat."""
    def __init__(self, server_host, server_port=20146, timeout=1):
        super(Client, self).__init__()
        self.__buf_size = 1024

        self.__sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__sockfd.setblocking(True)
        self.__sockfd.settimeout(timeout)
        self.__sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.__sockfd.connect((server_host, server_port))

        self.__is_work = True
        self.__lock = threading.Lock()

    def __send_msg(self):
        while True:
            mess = sys.stdin.readline().strip()
            if mess.lower() == 'exit':
                with self.__lock:
                    self.__is_work = False
                break
            self.__sockfd.sendall(mess)

    def __recv_msg(self):
        while True:
            with self.__lock:
                if not self.__is_work:
                    break
            try:
                mess = self.__sockfd.recv(self.__buf_size)
                print(mess)
            except socket.timeout:
                continue

    def run(self):
        send_proc = threading.Thread(target=self.__send_msg)
        recv_proc = threading.Thread(target=self.__recv_msg)
        recv_proc.start()
        send_proc.start()
        recv_proc.join()
        send_proc.join()
        self.__sockfd.close()

if __name__ == '__main__':
    Client('192.168.10.146').run()