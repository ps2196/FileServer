import socket
import sys
import json # for request/response parsing
import base64 # for file decoding

REQERROR = -1
DWLFIN = 0
AUTH_OK = 1
DWL = 2
NORES = 3
print("ASDASDADASDA")
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
            return NORES
        res_str = self.response_q.pop(0) # pop oldest response from Q
        print('Digest: ', res_str);
        res = json.loads(res_str) 
        if res.get('code') is not None and res['code'] <200 and res['code'] >= 300:
            #print("Server res: ", res_str)
            return REQERROR
        if res.get('command') is not None:
            if res['command'] == 'DWL':
                encoded_chunk = res['data']
                decoded_chunk = base64.b64decode(encoded_chunk)
                f=open('dwl/'+self.dwl_fname, 'ab')
                f.write(bytearray(decoded_chunk))
                f.close()
                print("RES code: ", res['code'])
                if res['code'] == 200:
                    print(DWLFIN)
                    return DWLFIN
                return DWL
            elif res['command'] == 'AUTH':
                return AUTH_OK
        return 0
            
            



                    


