import socket
import sys
import json # for request/response parsing
import base64 # for file decoding

class Client:
    def __init__(self, addr = 'localhost', port = 8888, id = 0):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect( (addr,port) ) # connect to server
        self.response_q = []
        self.res = [] # char list for buffering responses
        self.username = ""
        self.id = id
        self.dwl_fname = "stress_dwl_"+str(id) # unique filename for DWL test

    def login(self, username, pwd):
        """Send AUTH request to sock"""
        req = json.dumps(
            {'type':'REQUEST',
            'command':'AUTH',
            'username':username,
            'password':pwd}
        )
        req += '\0'
        print('Sending: ', req)
        self.sock.sendall(req.encode())

    def send_dwl_req(self,path, priority):
        """Send AUTH request to sock"""
        req = json.dumps(
            {'type':'REQUEST',
            'command':'DWL',
            'path':path,
            'priority':str(priority)}
        )
        req+= '\0'
        self.sock.sendall(req.encode())
    
    def recive_msg(self):
        """ Recive message from sock"""
        msg = self.sock.recv(1024)
        print('RECV msg = ',msg)
        msg = msg.decode()
        for c in msg:
            if c=='\n':
                self.response_q.append("".join(self.res))
                self.res = []
            else:
                self.res.append(c)
    
    def digest_response(self):
        """ Pop response from response_q and digest it"""
        self.recive_msg()
        if len(self.response_q) == 0:
            return 3
        res_str = self.response_q.pop(0) # pop oldest response from Q
        print('Digest: ', res_str);
        res = json.loads(res_str) 
        if res.get('code') is not None and res['code'] <200 and res['code'] >= 300:
            print("Server res: ", res_str)
            return -1
        if res.get('command') is not None:
            if res['command'] == 'DWL':
                encoded_chunk = res['data']
                decoded_chunk = base64.b64decode(encoded_chunk)
                f=open('dwl/'+self.dwl_fname, 'ab')
                f.write(bytearray(decoded_chunk))
                f.close()
                return 2
            elif res['command'] == 'DWLFIN':
                return 0
            elif res['command'] == 'AUTH':
                return 1
        return 0
            

# #test
# ADDR = 'localhost'
# PORT = 8888
# USER = 'root'
# PASS = 'root'
# DWL_FILENAME = 'obrazek.jpg'

# cli = Client(ADDR, PORT, 1)
# cli.login(USER, PASS)
# if cli.digest_response() != 1: #'AUTH OK'
#     print("Login failed for cli: ", cli.id)
#     exit(-1)
# else: #set user for cli
#     cli.username = USER;   

# cli.send_dwl_req(USER+'/public/'+DWL_FILENAME, 5)
# while cli.digest_response()>0: #DWLFIN
#     print("chunk recived")
# exit(0)

            



                    


