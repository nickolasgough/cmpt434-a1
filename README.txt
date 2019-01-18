Nickolas Gough, nvg081, 11181823


Commands recognized by x-client:

    get <local file name> <remote file name>
    put <local file name> <remote file name>
    quit


Running x-client:

    make x-client
    ./x-client <proxy/server name> <proxy/server port>

Example:

    make x-client
    ./x-client tux8 30001

Notes:

    The cient will print a prompt when it is ready to receive commands, and
    will print output indicating the status of its processing when handling a
    request.


Running tcp-proxy:

    make tcp-proxy
    ./tcp-proxy <host port> <server name> <server port>

Example:

    make tcp-proxy
    ./tcp-proxy 30001 tux8 30002

Notes:

    The proxy will not print a prompt when it is ready to receive commands, but
    will print output indicating the status of its processing when handling a
    request.


Running mixed-proxy:

    make mixed-proxy
    ./mixed-proxy <host port> <server name> <server port>

Example:

    make mixed-proxy
    ./mixed-proxy 30001 tux8 30002

Notes:

    The proxy will not print a prompt when it is ready to receive commands, but
    will print output indicating the status of its processing when handling a
    request.


Running tcp-server:

    make tcp-server
    ./tcp-server <host port>

Example:

    make tcp-server
    ./tcp-server 30002

Notes:

    The server will not print a prompt when it is ready to receive commands, but
    will print output indicating the status of its processing when handling a
    request.


Running udp-server:

    make udp-server
    ./udp-server <host port>

Example:

    make udp-server
    ./udp-server 30002

Notes:

    The server will not print a prompt when it is ready to receive commands, but
    will print output indicating the status of its processing when handling a
    request.
