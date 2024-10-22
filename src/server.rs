use confee::conf::*;
use std::sync::mpsc::{self, Sender, Receiver};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};

pub struct ServerExit(Sender<bool>, Receiver<bool>);

pub trait Server: Send + Sync + 'static {
    fn create(conf: &Conf) -> Self;
    fn set_join_handle(&mut self, handle: JoinHandle<()>);
    fn mainloop(&self);
    fn destroy(&mut self);
}

pub struct ServerFactory;

impl ServerFactory {
    pub fn create<T: Server>(conf: &Conf) -> Arc<Mutex<T>> {
        let server_instance = Arc::new(Mutex::new(T::create(conf)));
        let server_clone = Arc::clone(&server_instance);

        let handle = thread::spawn(move || {
            let server = server_clone.lock().unwrap();
            server.mainloop();
        });
        {
            let mut server = server_instance.lock().unwrap();
            server.set_join_handle(handle);
        }

        server_instance
    }
}
