import client

ADDR = 'localhost'
PORT = 8888
USER = 'root'
PASS = 'root'
DWL_FILENAME = 'obrazek.jpg'

cli = Client(ADDR, PORT, 1)
cli.login(USER, PASS)
if cli.digest_response() != 1: #'AUTH OK'
    print("Login failed for cli: ", cli.id)
    exit(-1)
else: #set user for cli
    cli.username = USER;   

cli.send_dwl_req(USER+'/public/'+DWL_FILENAME, 5)
while cli.digest_response()>0: #DWLFIN
    print("chunk recived")
exit(0)