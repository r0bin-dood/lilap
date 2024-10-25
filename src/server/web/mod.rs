use crate::{lock, receiver, server::*, server_state};
use confee::conf::*;
use std::sync::mpsc;
use std::net::IpAddr;
use std::net::{TcpListener, TcpStream, SocketAddr};
use std::time::Duration;
use std::io::{self, Write, Read};
use std::fs;

pub struct Web {
    dir: String,
    addr: IpAddr,
    port: u16,
    pub state: ServerState,
}

impl Server for Web {
    fn create(conf: &Conf) -> Self {
        let mut web = Web {
            dir: conf.get("web_dir").unwrap(),
            addr: conf.get("web_addr").unwrap(),
            port: conf.get("web_port").unwrap(),
            state: server_state!(),
        };
        web.state.prefix = String::from("web");
        web
    }

    fn mainloop(&self) {
        self.log("Ready");

        let socket_addr = SocketAddr::new(self.addr, self.port);
        let listener = TcpListener::bind(socket_addr).expect(&format!("{}: Could not bind to address", self.state.prefix));
        listener.set_nonblocking(true).expect(&format!("{}: Failed to set non-blocking", self.state.prefix));

        loop {
            lock!(receiver!(self), rx => {
                if let Ok(_) = rx.try_recv() {
                    self.log("Stop signal received. Shutting down.");
                    break;
                }
            });

            match listener.accept() {
                Ok((ref mut stream, addr)) => {
                    self.log(&format!("New connection from {}", addr));
                    self.handle_connection(stream);
                    self.log("Waiting for connections...");
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                    // No connections available, continue looping
                }
                Err(e) => {
                    self.log(&format!("Error accepting connection: {}", e));
                }
            }

            thread::sleep(Duration::from_millis(10));
        }

        self.log("Stopped");
    }
}

impl Web {
    fn handle_connection(&self, stream: &mut TcpStream) {
        let mut buffer = [0u8; 4096];
        match self.read_from_stream(stream, &mut buffer) {
            Ok(_) => {
                match self.send_response(stream, &mut buffer) {
                    Ok(_) => {}
                    Err(_) => {}
                }
            }
            Err(_) => {}
        }
    }

    fn read_from_stream(&self, stream: &mut TcpStream, buffer: &mut [u8]) -> Result<(), io::Error> {
        match stream.read(&mut buffer[..]) {
            Ok(0) => {
                self.log("Client disconnected.");
                Err(io::Error::new(io::ErrorKind::UnexpectedEof, ""))
            }
            Ok(n) => {
                let received_data = String::from_utf8_lossy(&buffer[..n]);
                self.log(&format!("Received {} bytes of data:\n{}", n, received_data));
                Ok(())
            }
            Err(e) => {
                self.log(&format!("Error reading from stream: {}", e));
                Err(e)
            }
        }
    }

    fn send_response(&self, stream: &mut TcpStream, buffer: &[u8]) -> Result<(), io::Error> {
        let request = String::from_utf8_lossy(buffer);

        let request_line = request.lines().next().unwrap_or("");

        let mut parts = request_line.split_whitespace();
        let method = parts.next().unwrap_or("");
        let path = parts.next().unwrap_or("/");

        self.log(&format!("HTTP Request Method: {}", method));
        self.log(&format!("Requested Path: {}", path));

        let response = self.handle_path(path);

        stream.write_all(response.as_bytes())?;
        stream.flush()?;

        Ok(())
    }

    fn handle_path(&self, path: &str) -> String {
        if path.contains("..") {
            self.log("Detected directory traversal attempt.");
            return self.build_response(400, "Invalid path");
        }

        let file_path = if path == "/" {
            format!("{}index.html", self.dir)
        } else {
            format!("{}{}", self.dir, &path[1..])
        };

        match self.read_file(&file_path) {
            Ok(content) => self.build_response(200, &content),
            Err(_) => self.build_response(404, "Oops! That doesn't exist."),
        }
    }

    fn build_response(&self, status_code: u16, body: &str) -> String {
        let status_text = match status_code {
            200 => "OK",
            404 => "Not Found",
            400 => "Bad Request",
            _ => "Unknown",
        };
        self.log(&format!("Response: {} {}", status_code, status_text));
        format!(
            "HTTP/1.1 {} {}\r\nContent-Length: {}\r\n\r\n{}",
            status_code,
            status_text,
            body.len(),
            body
        )
    }

    fn read_file(&self, file_path: &str) -> Result<String, std::io::Error> {
        fs::read_to_string(file_path)
    }
}