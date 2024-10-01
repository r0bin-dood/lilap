use std::{collections::HashMap, fs};

#[derive(Debug)]
pub enum Value {
    String(String),
    UInt(u32),
}

pub struct Conf;
impl Conf {
    pub fn new() -> HashMap<String, Value> {
        let mut conf: HashMap<String, Value> = HashMap::new();
        conf.insert("log_dir".to_string(), Value::String("stdout".to_string()));
        conf.insert("ap_name".to_string(), Value::String("lilap AP".to_string()));
        conf.insert("tpool_threads".to_string(), Value::UInt(8));
        conf.insert("web_app_dir".to_string(), Value::String("/var/www/html/".to_string()));
        conf.insert("web_app_index".to_string(), Value::String("index.html".to_string()));
        conf.insert("web_addr".to_string(), Value::String("127.0.0.1".to_string()));
        conf.insert("web_port".to_string(), Value::UInt(8080));

        conf
    }

    pub fn parse(hm: &mut HashMap<String, Value>, file_name: &str) {
        let lines = Self::read_lines(file_name);
        for line in lines {
            let i = line
                .trim()
                .find(' ')
                .expect("Bad line in configuration file");
            let key = &line[..i];
            let value = line[i..].trim_start().to_string();
            hm.entry(key.to_string()).and_modify(|v| match *v {
                Value::String(_) => *v = Value::String(value),
                Value::UInt(_) => *v = Value::UInt(value.parse().expect("Value conversion failed")),
            });
        }
    }
    fn read_lines(file_name: &str) -> Vec<String> {
        fs::read_to_string(file_name)
            .unwrap()
            .lines()
            .map(String::from)
            .collect()
    }
}
