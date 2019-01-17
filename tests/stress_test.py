import client as cl

ADDR = 'localhost'
PORT = 8888
USER = 'root'
PASS = 'root'
DWL_FILENAME = 'obrazek.jpg'

cli = cl.Client(ADDR, PORT, 1)
cli.login(USER, PASS)
if cli.digest_response() != cl.AUTH_OK: #'AUTH OK'
    print("Login failed for cli: ", cli.id)
    exit(-1)
else: #set user for cli
    cli.username = USER;   

cli.send_dwl_req(USER+'/public/'+DWL_FILENAME, 5)
while cli.digest_response() > cl.DWLFIN: #DWLFIN
    print("chunk recived")
exit(0)