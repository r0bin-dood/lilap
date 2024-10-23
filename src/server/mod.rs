pub mod web;
pub mod common;
use confee::conf::*;
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};

#[macro_export]
macro_rules! try_lock_or_panic {
    ($lock:expr, $var:ident => $($body:tt)*) => {{
        let mut $var = $lock.try_lock().expect("Failed to acquire lock immediately");
        $($body)*
    }};
}

#[macro_export]
macro_rules! lock {
    ($lock:expr, $var:ident => $($body:tt)*) => {{
        let mut $var = $lock.lock().unwrap();  // This will panic if the lock can't be acquired
        $($body)*
    }};
}

#[macro_export]
macro_rules! server_state {
    () => {
        ServerState {
            join_handle: None,
            exit: {
                let (tx, rx) = mpsc::channel();
                let rx = Arc::new(Mutex::new(rx));
                (tx, rx)
            },
        }
    };
}

pub struct ServerState {
    pub join_handle: Option<JoinHandle<()>>,
    pub exit: (Sender<bool>, Arc<Mutex<Receiver<bool>>>),

}

pub trait HasServerState {
    fn get_state(&self) -> &ServerState;
    fn get_state_mut(&mut self) -> &mut ServerState;
}
pub trait HasStateField {
    fn state(&self) -> &ServerState;
    fn state_mut(&mut self) -> &mut ServerState;
}

impl<T> HasServerState for T
where
    T: HasStateField,
{
    fn get_state(&self) -> &ServerState {
        &self.state()
    }

    fn get_state_mut(&mut self) -> &mut ServerState {
        self.state_mut()
    }
}

pub trait Server: Send + Sync + HasServerState + 'static {
    fn create(conf: &Conf) -> Self;
    fn set_join_handle(&mut self, handle: JoinHandle<()>) {
        self.get_state_mut().join_handle = Some(handle);
    }
    fn mainloop(&self);
    fn destroy(&mut self) {
        let _ = self.get_state().exit.0.send(true);

        if let Some(handle) = self.get_state_mut().join_handle.take() {
            match handle.join() {
                Ok(_) => println!("Thread finished successfully"),
                Err(e) => println!("Thread panicked: {:?}", e),
            }
        } else {
            println!("No thread handle present");
        }
    }
}

pub struct ServerFactory;

impl ServerFactory {
    pub fn create<T: Server>(conf: &Conf) -> Arc<Mutex<T>> {
        let server_instance = Arc::new(Mutex::new(T::create(conf)));
        let server_clone = Arc::clone(&server_instance);

        let handle = thread::spawn(move || {
            lock!(server_clone, server => server.mainloop());
        });
        lock!(server_instance, server => server.set_join_handle(handle));

        server_instance
    }
}
