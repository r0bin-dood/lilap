
mod conf;

use std::env;
use crate::conf::*;

fn main() {
    let args: Vec<String> = env::args().collect();
    // Program only accepts a configuration file as an argument
    if args.len() != 2 {
        std::process::exit(exitcode::DATAERR);
    }

    let mut conf = Conf::new();
    Conf::parse(&mut conf, &args[1]);

    if let Value::String(s) = conf.get("log_dir").unwrap() {
        println!("{}", s);
    }

    dbg!(conf);

}
