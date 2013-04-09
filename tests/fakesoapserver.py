#!/usr/bin/python

"""
Copyright (C) 2012 Kolibre

This file is part of kolibre-clientcore.

Kolibre-clientcore is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Kolibre-clientcore is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
"""

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import getopt, sys, os, re, ssl

input = None
orderfile = None

class FakeSoapServer(BaseHTTPRequestHandler):

    # override log request method
    def log_request(self, code=None, size=None):
        pass

    def do_GET(self):
        self.send_error(404,'File Not Found: %s' % self.path)

    def do_POST(self):
        response = self.getResponse()
        self.send_response(200)
        self.send_header('Content-Length', len(response))
        self.send_header('Content-Type', 'text/xml; charset=utf-8')
        self.end_headers()
        self.wfile.write(response)
        self.incrementOrder()
        return

    def getSoapAction(self):
        action = None
        for name, value in sorted(self.headers.items()):
            if name.lower() == "soapaction":
                action = value.strip()
        if action is None:
            self.sendInternalError('SOAPAction not found in header')
            return
        return action.lstrip('"/').rstrip('"')

    def getOrder(self):
        f = open(orderfile, 'r')
        order = int(f.read())
        self.order = order
        f.close()
        return order

    def incrementOrder(self):
        f = open(orderfile, 'w')
        order = self.order + 1
        f.write(str(order))
        f.close()

    def getResponse(self):
        SoapAction = self.getSoapAction()
        responsefile = input + '/' + str(self.getOrder()) + '_' + SoapAction
        if not os.path.exists(responsefile):
            self.sendInternalError('input file ' + responsefile + ' not found')
            return

        f = open(responsefile)
        content = f.read()
        f.close()

        # pattern for finding beginning of soap envelope "<SOAP-ENV:Envelope"
        pattern = re.compile('<[\w-]*:Envelope', re.IGNORECASE)

        # find last position of pattern and substring from there
        matches = pattern.findall(content)
        start = content.rfind(matches[len(matches)-1])
        body = content[start:]

        # manipulate response if SOAPAction is logOn
        if SoapAction == 'logOn':
            request_len = int(self.headers.getheader('content-length'))
            request = self.rfile.read(request_len)
            if 'incorrect' in request:
                body = body.replace('logOnResult>true', 'logOnResult>false')
        return body

    def sendInternalError(self, faultstring):
        soapfault = '<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://www.daisy.org/ns/daisy-online/"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>SOAP-ENV:Server</faultcode><faultstring>' + faultstring + '</faultstring><faultactor></faultactor><detail><ns1:internalServerErrorFault/></detail></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>'
        self.send_response(500)
        self.send_header('Content-Length', len(soapfault))
        self.send_header('Content-Type', 'text/xml; charset=utf-8')
        self.end_headers()
        self.wfile.write(soapfault)
        return

def usage():
    print ''
    print 'usage: python ' + sys.argv[0] + ' -i <dir> -o <file>'
    print
    print 'required arguments:'
    print ' -i, --input <dir>\t\tpath to folder containing soap responses'
    print ' -o, --order <file>\t\tpath to file controlling the order'
    print
    print 'optional arguments:'
    print ' -p, --port <port>\t\tport to listen on [default: 8080]'
    print ' -s, --ssl\t\t\tuse ssl'
    print ' -c, --cert <cert>\t\tpath to certificate'
    print ' -h, --help\t\t\tshow this help message and exit'

if __name__ == '__main__':
    host = 'localhost'
    port = 8080
    secure = False
    cert = None

	# parse command line options
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hp:i:o:sc:', ['help', 'port', 'input', 'order', 'ssl', 'cert'])
    except getopt.GetoptError, err:
        sys.stderr.write(str(err)) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    for opt, value in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit()
        elif opt in ('-p', '--port'):
            port = int(value)
        elif opt in ('-i', '--input'):
            input = value
        elif opt in ('-o', '--order'):
            orderfile = value
        elif opt in ('-s', '--ssl'):
            secure = True
        elif opt in ('-c', '--cert'):
            cert = value

    # check if input exists
    if input is None:
        usage()
        sys.exit()
    if not os.path.exists(input):
        sys.stderr.write("error: input '" + input + "' does not exists\n")
        sys.exit(2)
    if orderfile is None:
        usage()
        sys.exit()
    if not os.path.exists(orderfile):
        sys.stderr.write("error: orderfile '" + orderfile + "' does not exists\n")
        sys.exit(2)
    if secure and cert is None:
        sys.stderr.write("error: specify a certificate to use with ssl\n")
        sys.exit(2)
    if secure and not os.path.exists(cert):
        sys.stderr.write("error: certificate '" + cert + "' does not exists\n")
        sys.exit(2)

    # start server
    try:
        server = HTTPServer(('', port), FakeSoapServer)
        if secure:
            server.socket = ssl.wrap_socket(server.socket, certfile=cert, server_side=True)
        server.serve_forever()
    except KeyboardInterrupt:
        server.socket.close()
