use crate::{lock, receiver, server::*, server_state};
use confee::conf::*;
use std::net::{IpAddr, UdpSocket, SocketAddr};
use std::io;
use std::time::Duration;
use std::sync::mpsc;

pub struct Dns {
    addr: IpAddr,
    port: u16,
    pub state: ServerState,
}

impl Server for Dns {
    fn create(conf: &Conf) -> Self {
        let mut dns = Dns {
            addr: conf.get("link_addr").unwrap(),
            port: conf.get("dns_port").unwrap(),
            state: server_state!(),
        };
        dns.state.prefix = String::from("dns");
        dns
    }

    fn mainloop(&self) {
        let socket_addr = SocketAddr::new(self.addr, self.port);
        let socket = UdpSocket::bind(socket_addr).expect(&format!("{}: Could not bind to address", self.state.prefix));
        socket.set_nonblocking(true).expect(&format!("{}: Failed to set non-blocking", self.state.prefix));

        loop {
            lock!(receiver!(self), rx => {
                if let Ok(_) = rx.try_recv() {
                    self.log("Stop signal received. Shutting down.");
                    break;
                }
            });

            let mut buffer = [0; 512];
            match socket.recv_from(&mut buffer) {
                Ok((ref mut n, addr)) => {
                    self.log(&format!("New query from {}", addr));
                    self.log(&format!("Received {} bytes of data:\n{}", n, self.format_bytes_as_hex(&buffer, *n)));
                    let response = self.create_response(&buffer, *n);
                    match socket.send_to(&response[..], addr) {
                        Ok(sent_bytes) => {
                            self.log(&format!("Sent {} bytes to {}:\n{}", sent_bytes, addr, self.format_bytes_as_hex(&response[..], sent_bytes)));
                        }
                        Err(e) => {
                            self.log(&format!("Failed to send response to {}: {}", addr, e));
                        }
                    }
                    self.log("Waiting for queries...");
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                    // No connections available, continue looping
                }
                Err(e) => {
                    self.log(&format!("Error receiving query: {}", e));
                }
            }

            thread::sleep(Duration::from_millis(10));
        }

        self.log("Stopped");
    }
}

impl Dns {
    fn create_response(&self, request: &[u8], size: usize) -> Vec<u8> {
        let mut response = Vec::new();
    
        // Copy the request header (transaction ID, flags, etc.)
        response.extend_from_slice(&request[0..12]);

        // Set the response flag (standard response, no error)
        response[2] = 0x80; // Response, recursion available
        if (request[2] & 0x01) != 0 {
            response[2] |= 0x01;
        }
        response[3] = 0x00; // No error
        response[7] = 0x01; // 1 Answer
        response[11] = 0x00; // No Additional section

        // Copy the question section
        if let Some(end) = request[12..].iter().position(|&b| b == 0x00) {
            let question_end = 12 + end + 5; // +1 for null byte, +4 for type and class
            response.extend_from_slice(&request[12..question_end]);
        }

        // Append the answer section
        // Name is a pointer to the domain in the question section (0xC00C)
        response.extend_from_slice(&[0xC0, 0x0C]);

        // Type A (IPv4), Class IN (Internet)
        response.extend_from_slice(&[0x00, 0x01, 0x00, 0x01]);

        // TTL (time-to-live) in seconds (300 here)
        response.extend_from_slice(&[0x00, 0x00, 0x01, 0x2C]);

        // Data length (4 bytes for IPv4 address)
        response.extend_from_slice(&[0x00, 0x04]);
    
        // Include the IPv4 address
        if let IpAddr::V4(ipv4_addr) = self.addr {
            response.extend_from_slice(&ipv4_addr.octets()); // Append the IP address
        }
    
        response // Return the constructed response
    }

    fn format_bytes_as_hex(&self, buffer: &[u8], n: usize) -> String {
        let bytes_to_read = n.min(buffer.len());
        let hex_bytes: Vec<String> = buffer[..bytes_to_read]
            .iter()
            .map(|byte| format!("{:02x}", byte)) 
            .collect();
        let mut hex_output = String::new();
        for chunk in hex_bytes.chunks(32) {
            hex_output.push_str(&chunk.join(" "));
            hex_output.push('\n');
        }
        hex_output
    }
}