# -*- coding: utf-8 -*-
'''
  Filename: server.py
  Creator: Hemajun
  Description: --
'''
from __future__ import print_function
import socket
import select
import Queue
import time

class Server(object):
    """Server class of lan chat."""
    def __init__(self, host='', port=20146, timeout=2, client_nums=10):
        super(Server, self).__init__()
        self.__buf_size = 1024

        self.__sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__sockfd.setblocking(False)
        self.__sockfd.settimeout(timeout)
        self.__sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        self.__sockfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.__sockfd.bind((host, port))
        self.__sockfd.listen(client_nums)

        self.__ins = [self.__sockfd]
        self.__outs = []
        self.__message_queues = {}
        self.__client_info = {}

    def run(self, select_timeout=10):
        while True:
            readable, writable, exceptional = select.select(self.__ins, self.__outs, self.__ins, select_timeout)
            if not (readable or writable or exceptional):
                continue
            for s in readable:
                if s is self.__sockfd:
                    client, addr_client = s.accept()
                    print('%s connected.' % str(addr_client))
                    client.setblocking(False)
                    self.__ins.append(client)
                    self.__client_info[client] = str(addr_client)
                    self.__message_queues[client] = Queue.Queue()
                else:
                    mess = s.recv(self.__buf_size)
                    if mess:
                        mess = '%s %s | %s' % (time.strftime("%Y-%m-%d %H:%M:%S"), self.__client_info[s], mess)
                        self.__message_queues[s].put(mess)
                        if not s in self.__outs:
                            self.__outs.append(s)
                    else:
                        print('%s disconnected.' % str(self.__client_info[s]))
                        if s in self.__outs:
                            self.__outs.remove(s)
                        self.__ins.remove(s)
                        s.close()
                        del self.__message_queues[s]
                        del self.__client_info[s]

            for s in writable:
                try:
                    mess = self.__message_queues[s].get_nowait()
                except Queue.Empty:
                    self.__outs.remove(s)
                except Exception:
                    if s in self.__outs:
                        self.__outs.remove(s)
                else:
                    for client in self.__client_info:
                        try:
                            client.sendall(mess if client is not s else '\033[32m%s\033[0m' % mess)
                        except Exception:
                            if client in self.__ins:
                                self.__ins.remove(client)
                            if client in self.__outs:
                                self.__outs.remove(s)
                            if client in self.__message_queues:
                                del self.__message_queues[s]
                            del self.__client_info[client]

            for s in exceptional:
                if s in self.__ins:
                    self.__ins.remove(s)
                    s.close()
                if s in self.__outs:
                    self.__outs.remove(s)
                if s in self.__message_queues:
                    del self.__message_queues[s]
                del self.__client_info[s]

if __name__ == '__main__':
    Server().run()
        