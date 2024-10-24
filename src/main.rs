mod conf_defaults;
mod server;

use confee::conf::*;
use server::*;
use signal_hook::{
    consts::signal::{SIGABRT, SIGINT, SIGTERM},
    iterator::Signals,
};
use std::env;
use server::web::Web;

fn main() {
    let mut conf = Conf::from(conf_defaults!());

    let args: Vec<String> = env::args().collect();
    match args.len() {
        2 => {
            let conf_file_name = &args[1];
            match conf.with_file(conf_file_name).update() {
                Ok(_) => println!(
                    "Successfully updated configuration with {}!",
                    conf_file_name
                ),
                Err(e) => panic!("Error updating configuration: {}", e),
            }
        }
        1 => {
            println!("Running with defaults:\n{}", conf);
        }
        _ => {
            println!("Invalid usage");
            println!("lilap <conf>");
            std::process::exit(exitcode::DATAERR);
        }
    }

    // Set up web server
    let web_server = ServerFactory::create::<Web>(&conf);

    let mut signals =
        Signals::new(&[SIGINT, SIGABRT, SIGTERM]).expect("Error setting up signal handler");
    signals.wait();

    web_server.destroy();

    ServerFactory::join();
}
