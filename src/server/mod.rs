pub mod common;
pub mod web;
pub mod dns;
pub mod dhcp;
pub mod link;
use confee::conf::*;
use std::sync::mpsc::{Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use once_cell::sync::Lazy;

static JOIN_HANDLES: Lazy<Mutex<Vec<JoinHandle<()>>>> = Lazy::new(|| Mutex::new(Vec::new()));

#[macro_export]
macro_rules! sender {
    ($self:ident) => {
        $self.get_state().txrx.0
    };
}
#[macro_export]
macro_rules! receiver {
    ($self:ident) => {
        $self.get_state().txrx.1
    };
}

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
        let mut $var = $lock.lock().unwrap();
        $($body)*
    }};
}

#[macro_export]
macro_rules! server_state {
    () => {
        ServerState {
            prefix: String::from(""),
            txrx: {
                let (tx, rx) = mpsc::channel();
                let rx = Arc::new(Mutex::new(rx));
                (tx, rx)
            },
        }
    };
}

pub struct ServerState {
    pub prefix: String,
    pub txrx: (Sender<bool>, Arc<Mutex<Receiver<bool>>>),
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
    fn mainloop(&self);
    fn log(&self, fmt: &str) {
        let formatted_message = format!("{}: {}", self.get_state().prefix, fmt);
        let indented_message = if formatted_message.contains('\n') {
            let lines: Vec<&str> = formatted_message.lines().collect();
            let mut result = lines[0].to_string();
            for line in &lines[1..] {
                result.push_str(&format!("\n\t{}", line));
            }
            result
        } else {
            // If there's no newline, just format the message normally
            formatted_message // Keep the single line as it is
        };
        println!("{}", indented_message);
    }
    fn destroy(&self) {
        let _ = sender!(self).send(true);
    }
}

pub struct ServerFactory;

impl ServerFactory {
    pub fn create<T: Server>(conf: &Conf) -> Arc<T> {
        let server_instance = Arc::new(T::create(conf));
        let server_clone = Arc::clone(&server_instance);

        let handle = thread::spawn(move || {
            server_clone.mainloop();
        });
        JOIN_HANDLES.lock().unwrap().push(handle);

        server_instance
    }

    pub fn join() {
        let mut handles = JOIN_HANDLES.lock().unwrap();

        for handle in handles.drain(..) {
            handle.join().expect("Thread panicked!");
        }
    }
}
