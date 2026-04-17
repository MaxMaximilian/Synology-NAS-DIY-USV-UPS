from http.server import BaseHTTPRequestHandler, HTTPServer
import subprocess

# Tokens
TOKEN_SHUTDOWN = "shutdown"  # echter Shutdown
TOKEN_TEST = "test"  # Selbsttest


class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Pfad und Query auswerten
        path = self.path

        if path == f"/shutdown_token?token={TOKEN_SHUTDOWN}":
            # Shutdown-Token
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"Shutting down NAS")
            # NAS herunterfahren
            subprocess.call(["/sbin/poweroff"])  # Disable this Line for Windows Tests

        elif path == f"/shutdown_token?token={TOKEN_TEST}":
            # Selbsttest-Token
            self.send_response(200)
            self.end_headers()
            self.wfile.write(
                b"Token accepted"
            )  # Bestätigung zurückgeben, kein Shutdown

        else:
            # Ungültiger Token oder URL
            self.send_response(403)
            self.end_headers()
            self.wfile.write(b"Invalid token or URL")


if __name__ == "__main__":
    server_address = ("", 7000)  # Port 7000
    httpd = HTTPServer(server_address, RequestHandler)
    print("Shutdown server running on port 7000...")
    httpd.serve_forever()
