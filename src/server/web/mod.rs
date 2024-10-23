use crate::{lock, server::*, server_state};
use confee::conf::*;
use std::sync::mpsc;
use std::{
    net::IpAddr,
    thread::{self, JoinHandle},
};

pub struct Web {
    dir: String,
    addr: IpAddr,
    port: u16,
    pub state: ServerState,
}

impl Server for Web {
    fn create(conf: &Conf) -> Self {
        let web = Web {
            dir: conf.get("web_dir").unwrap(),
            addr: conf.get("web_addr").unwrap(),
            port: conf.get("web_port").unwrap(),
            state: server_state!(),
        };
        web
    }

    fn mainloop(&self) {
        println!("Web Server started!");

        lock!(self.state.exit.1, rx => {
            let _ = rx.recv();
        });

        println!("Web Server done!");
    }
}
