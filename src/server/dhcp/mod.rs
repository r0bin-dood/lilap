use crate::{lock, receiver, server::*, server_state};
use confee::conf::*;
use std::net::{IpAddr, UdpSocket, SocketAddr};
use std::io::{self, Cursor, Write, Read};
use byteorder::{BigEndian, ReadBytesExt};
use std::time::Duration;
use std::sync::mpsc;

pub struct Dhcp {
    addr: IpAddr,
    port: u16,
    dst_port: u16,
    pub state: ServerState,
}

impl Server for Dhcp {
    fn create(conf: &Conf) -> Self {
        let mut dhcp = Dhcp {
            addr: conf.get("link_addr").unwrap(),
            port: conf.get("dhcp_src_port").unwrap(),
            dst_port: conf.get("dhcp_dst_port").unwrap(),
            state: server_state!(),
        };
        dhcp.state.prefix = String::from("dhcp");
        dhcp
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
                    self.log(&format!("New request from {}", addr));
                    self.log(&format!("Received {} bytes of data:\n{}", n, self.format_bytes_as_hex(&buffer, *n)));
                    if let Some((xid, client_mac)) = self.parse_dhcp_request(&buffer[..*n]) {
                        let response = self.create_dhcp_offer(xid, &client_mac);
                        let response_addr = (addr.ip(), self.dst_port);
                        match socket.send_to(&response[..], response_addr) {
                            Ok(sent_bytes) => {
                                self.log(&format!("Sent {} bytes to {}:\n{}", sent_bytes, addr, self.format_bytes_as_hex(&response[..], sent_bytes)));
                            }
                            Err(e) => {
                                self.log(&format!("Failed to send response to {}: {}", addr, e));
                            }
                        }
                    } else {
                        self.log("Failed to parse DHCP request.");
                    }
                    self.log("Waiting for requests...");
                }
                Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                    // No connections available, continue looping
                }
                Err(e) => {
                    self.log(&format!("Error receiving request: {}", e));
                }
            }

            thread::sleep(Duration::from_millis(10));
        }

        self.log("Stopped");
    }
}

impl Dhcp {
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

    fn parse_dhcp_request(&self, buffer: &[u8]) -> Option<(u32, Vec<u8>)> {
        // Transaction ID is bytes 4 to 8
        let xid = (&buffer[4..8]).read_u32::<BigEndian>().ok()?;
    
        // MAC address is bytes 28 to 34 (assuming Ethernet hardware)
        let client_mac = buffer[28..34].to_vec();
    
        Some((xid, client_mac))
    }

    fn create_dhcp_offer(&self, transaction_id: u32, client_mac: &[u8]) -> Vec<u8> {
        let mut response = vec![0u8; 240]; // Basic DHCP response structure size
    
        // Fill in some of the basic fields (op, htype, xid, etc.)
        response[0] = 2; // op: 2 = BOOTREPLY
        response[1] = 1; // htype: Ethernet
        response[2] = 6; // hlen: MAC address length
        response[4..8].copy_from_slice(&transaction_id.to_be_bytes());
    
        // Copy the client MAC address into the response
        response[28..34].copy_from_slice(&client_mac);
    
        // Fill in DHCP options, including DHCP message type (DHCPOFFER)
        response.extend(&[53, 1, 2]); // Option 53 (Message Type) -> DHCPOFFER
    
        // Optionally, include more options like the offered IP address, lease time, etc.
    
        response.push(255); // End option
    
        response
    }
    
}