use crate::{lock, receiver, server::*, server_state};
use confee::conf::*;
use std::net::IpAddr;
use neli::{
    consts::{genl::*, nl::*, socket::*},
    genl::{Genlmsghdr, Nlattr, GenlmsghdrBuilder},
    nl::{Nlmsghdr, NlPayload, NlmsghdrBuilder},
    router::synchronous::NlRouter,
    types::{Buffer, GenlBuffer},
    utils::Groups,
};
use std::ffi::CString;
use std::io::*;
use libc::{if_nametoindex};
use std::time::Duration;
use std::sync::mpsc;


const NL80211_CMD_NEW_INTERFACE: u8 = 4;

pub struct Link {
    addr: IpAddr,
    parent_iface_name: String,
    iface_name: String,
    ssid: String,
    pub state: ServerState,
}

impl Server for Link {
    fn create(conf: &Conf) -> Self {
        let mut link = Link {
            addr: conf.get("link_addr").unwrap(),
            parent_iface_name: conf.get("link_parent_iface").unwrap(),
            iface_name: conf.get("link_iface").unwrap(),
            ssid: conf.get("link_ssid").unwrap(),
            state: server_state!(),
        };
        link.state.prefix = String::from("link");
        link
    }

    fn mainloop(&self) {
        let mut socket = NlRouter::connect(NlFamily::Generic, None, Groups::empty());

        let parent_iface = self.parent_iface_name.as_str();
        match self.get_interface_index(parent_iface) {
            Ok(index) => self.log(&format!("Interface index for {}: {}", parent_iface, index)),
            Err(e) => self.log(&format!("Failed to get interface index: {}", e)),
        }
    
        loop {
            lock!(receiver!(self), rx => {
                if let Ok(_) = rx.try_recv() {
                    self.log("Stop signal received. Shutting down.");
                    break;
                }
            });

            thread::sleep(Duration::from_millis(10));
        }

        self.log("Stopped");
    }
}

impl Link {
    fn get_interface_index(&self, interface_name: &str) -> Result<u32> {
        let cstr = CString::new(interface_name).map_err(|_| Error::new(ErrorKind::InvalidInput, "Invalid interface name"))?;
        
        let index = unsafe { if_nametoindex(cstr.as_ptr()) };
        if index == 0 {
            Err(Error::new(ErrorKind::NotFound, "Interface not found"))
        } else {
            Ok(index)
        }
    }
}