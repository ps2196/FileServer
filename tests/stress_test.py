import client as cl
import threading

ADDR = 'localhost'
PORT = 8888
USER = 'root'
PASS = 'root'
DWL_FILENAME = 'foto.jpg'
CLIENTS = 10 # number of clients to run



# clients = []
# for cli_id in range(0,CLIENTS):
#     cli = cl.Client(ADDR, PORT, cli_id)
#     cli.login(USER, PASS)
#     if cli.digest_response() != cl.AUTH_OK: #'AUTH OK'
#         print("Login failed for cli: ", cli.id)
#         exit(-1)
#     else: #set user for cli
#         clients.append(cli)
#         cli.username = USER;   
#     cli.send_dwl_req(USER+'/public/'+DWL_FILENAME, 5)
   

# for cli in clients: 
#     res_code = cli.digest_response() 
#     while res_code not in [cl.DWLFIN, cl.REQERROR]: #DWLFIN
#         res_code = cli.digest_response() 
#     print('Download finished! ( cli.id = ',cli.id,')')


def start_client(cli_id):
    cli = cl.Client(ADDR, PORT, cli_id)
    cli.login(USER, PASS)
    if cli.digest_response() != cl.AUTH_OK: #'AUTH OK'
        print("Login failed for cli: ", cli.id)
    else: #set user for cli
        cli.username = USER;   

    cli.send_dwl_req(USER+'/public/'+DWL_FILENAME, 5)
    res_code = cli.digest_response() 
    while res_code not in [cl.DWLFIN, cl.REQERROR]: #DWLFIN
        res_code = cli.digest_response() 
    print('Download finished! ( cli.id = ',cli.id,')')

# # Run client threads
threads = []
for i in range(0, CLIENTS):
    try:
        thr = threading.Thread( target=start_client, args=( i, ) )
        threads.append(thr)
        thr.start()
        print('Starting client ',i)
    except:
        print ("Error: unable to start thread: ",i)
for t in threads:
    if t.is_alive():
        t.join()