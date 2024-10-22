use crate::server::*;
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
    thread_handle: Option<JoinHandle<()>>,
}

impl Web {}

impl Server for Web {
    fn create(conf: &Conf) -> Self {
        let web = Web {
            dir: conf.get("web_dir").unwrap(),
            addr: conf.get("web_addr").unwrap(),
            port: conf.get("web_port").unwrap(),
            thread_handle: None,
        };
        web
    }

    fn set_join_handle(&mut self, handle: JoinHandle<()>) {
        self.thread_handle = Some(handle);
    }

    fn mainloop(&self) {
        println!("Web Server started!");
    }

    fn destroy(&mut self) {
        if let Some(handle) = self.thread_handle.take() {
            match handle.join() {
                Ok(_) => println!("Thread finished successfully"),
                Err(e) => println!("Thread panicked: {:?}", e),
            }
        } else {
            println!("No thread handle present");
        }
        println!("Web Server done!")
    }
}
